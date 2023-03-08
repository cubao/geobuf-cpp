import os

from loguru import logger

from pybind11_geobuf import rapidjson  # noqa
from pybind11_geobuf import Decoder, Encoder  # noqa
from pybind11_geobuf import normalize_json as normalize_json_impl  # noqa
from pybind11_geobuf import pbf_decode as pbf_decode_impl  # noqa


def __filesize(path: str) -> int:
    return os.stat(path).st_size


def geobuf2json(
    input_path: str,
    output_path: str,
    *,
    indent: bool = False,
    sort_keys: bool = False,
):
    logger.info(
        f"geobuf decoding {input_path} ({__filesize(input_path):,} bytes)..."
    )  # noqa
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    decoder = Decoder()
    assert decoder.decode(
        geobuf=input_path,
        geojson=output_path,
        indent=indent,
        sort_keys=sort_keys,
    ), f"failed at decoding geojson, input:{input_path}, output:{output_path}"
    logger.info(f"wrote to {output_path} ({__filesize(output_path):,} bytes)")


def json2geobuf(
    input_path: str,
    output_path: str,
    *,
    precision: int = 8,
):
    logger.info(
        f"geobuf encoding {input_path} ({__filesize(input_path):,} bytes)..."
    )  # noqa
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    encoder = Encoder(max_precision=int(10**precision))
    assert encoder.encode(
        geojson=input_path,
        geobuf=output_path,
    ), f"failed at encoding geojson, input:{input_path}, output:{output_path}"
    logger.info(f"wrote to {output_path} ({__filesize(output_path):,} bytes)")


def normalize_geobuf(
    input_path: str,
    output_path: str = None,
    *,
    sort_keys: bool = True,
    precision: int = -1,
):
    logger.info(
        f"normalize_geobuf {input_path} ({__filesize(input_path):,} bytes)"
    )  # noqa
    with open(input_path, "rb") as f:
        encoded = f.read()
    decoder = Decoder()
    json = decoder.decode_to_rapidjson(encoded, sort_keys=sort_keys)
    if precision < 0:
        precision = decoder.precision()
        logger.info(f"auto precision from geobuf: {precision}")
    else:
        logger.info(f"user precision: {precision}")
    encoder = Encoder(max_precision=int(10**precision))
    encoded = encoder.encode(json)
    logger.info(f"encoded #bytes: {len(encoded):,}")
    output_path = output_path or input_path
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    with open(output_path, "wb") as f:
        f.write(encoded)
    logger.info(f"wrote to {output_path} ({__filesize(output_path):,} bytes)")


def normalize_json(
    input_path: str,
    output_path: str = None,
    *,
    indent: bool = True,
    sort_keys: bool = True,
    precision: int = -1,
):
    logger.info(
        f"normalize_json {input_path} ({__filesize(input_path):,} bytes)"
    )  # noqa
    output_path = output_path or input_path
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    if precision > 0:
        logger.info(
            f"convert to geobuf (precision: {precision}), then back to geojson"
        )  # noqa
        encoder = Encoder(max_precision=int(10**precision))
        geojson = rapidjson().load(input_path)
        assert geojson.IsObject(), f"invalid geojson: {input_path}"
        encoded = encoder.encode(geojson)
        logger.info(f"geobuf #bytes: {len(encoded):,}")
        decoder = Decoder()
        text = decoder.decode(encoded, indent=indent, sort_keys=sort_keys)
        logger.info(f"geojson #bytes: {len(text):,}")
        with open(output_path, "w", encoding="utf8") as f:
            f.write(text)
    else:
        assert normalize_json_impl(
            input_path,
            output_path,
            indent=indent,
            sort_keys=sort_keys,
        ), f"failed to normalize json to {output_path}"
    logger.info(f"wrote to {output_path} ({__filesize(output_path):,} bytes)")


def pbf_decode(path: str, output_path: str = None, *, indent: str = ""):
    with open(path, "rb") as f:
        data = f.read()
    decoded = pbf_decode_impl(data, indent=indent)
    if output_path:
        os.makedirs(
            os.path.dirname(os.path.abspath(output_path)), exist_ok=True
        )  # noqa
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
            "geobuf2json": geobuf2json,
            "json2geobuf": json2geobuf,
            "normalize_geobuf": normalize_geobuf,
            "normalize_json": normalize_json,
            "pbf_decode": pbf_decode,
        }
    )
