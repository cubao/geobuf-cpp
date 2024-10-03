from __future__ import annotations

import argparse
import glob
import hashlib
import json
import os
import shutil
import sys
import time

from loguru import logger

PWD = os.path.abspath(os.path.dirname(__file__))
CACHE_DIR = os.path.join(PWD, "build/roundtrip_test")
os.umask(0)
os.makedirs(CACHE_DIR, exist_ok=True)


def system(
    cmd: str,
    *,
    verbose: bool = False,
    dry_run: bool = False,
    assert_return: bool = True,
):
    if dry_run:
        print(f"$ {cmd}")
        return
    ret = os.system(cmd)
    if verbose:
        print(f"$ {cmd} # -> {ret}")
    if assert_return:
        assert ret == 0, f"failed at {cmd} -> {ret}"


def hash(data: list | dict | str | bytes | bytearray | memoryview) -> str:
    if isinstance(data, list):
        digests = [hash(d) for d in data]
        return hash(";".join(digests))
    if isinstance(data, dict):
        data = json.dumps(data)
    if isinstance(data, str):
        data = str.encode(data)
    return hashlib.md5(data).hexdigest()


def hash_of(filename: str) -> str:
    hash_md5 = hashlib.md5()
    with open(filename, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()


def normalize_json(input_path: str, output_path: str):
    with open(input_path) as f:
        obj = json.load(f)
    output_path = os.path.abspath(output_path)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w") as f:
        json.dump(obj, f, sort_keys=True, indent=4)
    logger.info(f"normalize json: {input_path} -> {output_path}")
    return output_path


def roundtrip_cpp(input_json: str, output_pbf: str, output_json: str):
    system(
        f"{PWD}/build/json2geobuf {input_json} > {output_pbf}",
        dry_run=False,
    )
    system(
        f"{PWD}/build/geobuf2json {output_pbf} > {output_json}",
        dry_run=False,
    )


def roundtrip_js(input_json: str, output_pbf: str, output_json: str):
    system(f"json2geobuf {input_json} > {output_pbf}")
    system(f"geobuf2json {output_pbf} > {output_json}")


def roundtrip_py(input_json: str, output_pbf: str, output_json: str):
    system(f"geobuf encode < {input_json} > {output_pbf}")
    system(f"geobuf decode < {output_pbf} > {output_json}")


def roundtrip(
    input_json: str,
    *,
    force_rerun: bool = False,
):
    input_json = os.path.abspath(input_json)
    md5hex = hash_of(input_json)
    output_dir = f"{CACHE_DIR}/{md5hex}"
    if force_rerun and os.path.isdir(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir, exist_ok=True)

    input_path = f"{output_dir}/input.json"
    if not os.path.isfile(input_path):
        # shutil.copy(input_json, input_path)
        link = os.path.relpath(input_json, os.path.dirname(input_path))
        os.symlink(link, input_path)
    input_normalized = normalize_json(
        input_path,
        f"{output_dir}/input_normalized.json",
    )

    pbf_decoder = f"{PWD}/build/pbf_decoder"

    t0j = time.time()
    roundtrip_js(input_path, f"{output_dir}/js.pbf", f"{output_dir}/js.json")
    t1j = time.time()
    logger.info(f"roundtrip ( js): {t1j - t0j:.3f} sec ({input_path})")
    system(
        f"{pbf_decoder} {output_dir}/js.pbf > {output_dir}/js.pbf.txt",
    )
    output_js = normalize_json(
        f"{output_dir}/js.json",
        f"{output_dir}/js_normalized.json",
    )

    t0p = time.time()
    roundtrip_py(input_path, f"{output_dir}/py.pbf", f"{output_dir}/py.json")
    t1p = time.time()
    logger.info(f"roundtrip ( py): {t1p - t0p:.3f} sec ({input_path})")
    system(
        f"{pbf_decoder} {output_dir}/py.pbf > {output_dir}/py.pbf.txt",
    )
    output_py = normalize_json(
        f"{output_dir}/py.json", f"{output_dir}/py_normalized.json"
    )

    t0c = time.time()
    roundtrip_cpp(
        input_path,
        f"{output_dir}/cpp.pbf",
        f"{output_dir}/cpp.json",
    )
    t1c = time.time()
    logger.info(f"roundtrip (cpp): {t1c - t0c:.3f} sec ({input_path})")
    system(f"{pbf_decoder} {output_dir}/cpp.pbf > {output_dir}/cpp.pbf.txt")
    output_cpp = normalize_json(
        f"{output_dir}/cpp.json", f"{output_dir}/cpp_normalized.json"
    )

    system(
        f"diff {input_normalized} {output_js} > {output_dir}/diff_input_output_js.diff",
        assert_return=False,
    )
    system(
        f"diff {output_js} {output_py} > {output_dir}/diff_js_py.diff",
        assert_return=False,
    )
    system(
        f"diff {output_js} {output_cpp} > {output_dir}/diff_js_cpp.diff",
        assert_return=False,
    )
    system(
        f"diff {output_py} {output_cpp} > {output_dir}/diff_py_cpp.diff",
        assert_return=False,
    )
    system(f"tree --du -h {output_dir}")

    filenames = [input_normalized, output_js, output_py, output_cpp]
    md5sums = [hash_of(p) for p in filenames]
    if len(set(md5sums)) == 1:
        for p, m in zip(filenames, md5sums):
            logger.info(f"md5sum = {m} ({p})")
    else:
        for p, m in zip(filenames, md5sums):
            logger.error(f"md5sum = {m} ({p})")

    logger.info(f"roundtrip for {input_path} done")


if __name__ == "__main__":
    prog = f"python3 {sys.argv[0]}"
    description = "geobuf-roundtrip-test (js/py/c++)"
    parser = argparse.ArgumentParser(prog=prog, description=description)
    parser.add_argument(
        "input",
        type=str,
        help="input json (file or directory)",
    )
    args = parser.parse_args()
    input_path = os.path.abspath(args.input)
    args = None

    if os.path.isfile(input_path):
        roundtrip(input_path)
    else:
        for path in sorted(glob.glob(f"{input_path}/*.json")):
            if path.split("/")[-1] == "precision.json":
                continue
            roundtrip(path)
