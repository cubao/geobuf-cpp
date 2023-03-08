from loguru import logger


def geobuf2geojson(input_path: str, output_path: str):
    logger.info("TODO")
    pass


def geojson2geobuf(input_path: str, output_path: str):
    logger.info("TODO")


if __name__ == "__main__":
    import fire

    fire.core.Display = lambda lines, out: print(*lines, file=out)
    fire.Fire(
        {
            "geobuf2geojson",
            "geojson2geobuf",
        }
    )
