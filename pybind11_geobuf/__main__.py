import json
import os
import sys

from loguru import logger

from pybind11_geobuf import Decoder, Encoder, geojson
from pybind11_geobuf import pbf_decode as pbf_decode_impl  # noqa
from pybind11_geobuf import rapidjson, str2geojson2str, str2json2str


def __filesize(path: str) -> int:
    return os.stat(path).st_size


def geobuf2geojson(
    input_path: str,
    output_path: str,
    *,
    indent: bool = False,
    sort_keys: bool = False,
):
    logger.info(f'geobuf decoding {input_path} ({__filesize(input_path):,} bytes)...')
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    decoder = Decoder()
    assert decoder.decode(
        geobuf=input_path,
        geojson=output_path,
        indent=indent,
        sort_keys=sort_keys,
    ), f'failed at decoding geojson, input:{input_path}, output:{output_path}'
    logger.info(f'wrote to {output_path} ({__filesize(output_path):,} bytes)')


def geojson2geobuf(input_path: str, output_path: str,
    *,
    precision: int = 8,
):
    logger.info(f'geobuf encoding {input_path} ({__filesize(input_path):,} bytes)...')
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    encoder = Encoder(max_precision=int(10**precision))
    assert encoder.encode(
        geojson=input_path,
        geobuf=output_path,
    ), f'failed at encoding geojson, input:{input_path}, output:{output_path}'
    logger.info(f'wrote to {output_path} ({__filesize(output_path):,} bytes)')


def normalize_json(path: str, *, output_path: str = None):
    pass

def pbf_decode(path: str, output_path: str = None, *, indent: str = ""):
    with open(path, "rb") as f:
        data = f.read()
    decoded = pbf_decode_impl(data, indent=indent)
    if output_path:
        os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
        with open(output_path, "w", encoding="utf8") as f:
            f.write(decoded)
        logger.info(f"wrote to {output_path}")
    else:
        print(decoded)


if __name__ == "__main__":
    import fire

    fire.core.Display = lambda lines, out: print(*lines, file=out)
    fire.Fire(
        {
            "geobuf2geojson": geobuf2geojson,
            "geojson2geobuf": geojson2geobuf,
            "pbf_decode": pbf_decode,
        }
    )
