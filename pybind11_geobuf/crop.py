import os
from typing import List, Optional, Tuple, Union  # noqa

import numpy as np
from _pybind11_geobuf import *  # noqa
from loguru import logger


def crop_by_feature_id(
    input_path: str,
    output_path: str,
    *,
    feature_id: str,
    size: Union[float, Tuple[float, float]] = 100.0,
):
    logger.info(f"wrote to {output_path}")


def crop_by_grid(
    input_path: str,
    output_dir: str,
    *,
    anchor_lla: Union[str, List[float]] = None,
    grid_size: Union[float, Tuple[float, float]] = 1000.0,
):
    os.makedirs(os.path.abspath(output_dir), exist_ok=True)


def crop_by_center(
    input_path: str,
    output_dir: str,
    *,
    anchor_lla: Union[str, List[float]] = None,
    size: Union[float, Tuple[float, float]] = 1000.0,
):
    os.makedirs(os.path.abspath(output_dir), exist_ok=True)


def crop_by_bbox(
    input_path: str,
    output_path: str,
    *,
    bbox: Union[str, List[float]],
    z_center: float = None,
    z_max_offset: float = None,
):
    logger.info(f"wrote to {output_path}")


def crop_by_polygon(
    input_path: str,
    output_path: str,
    *,
    polygon: Union[str, np.ndarray],
    z_max_offset: float = None,
):
    pass


if __name__ == "__main__":
    import fire

    fire.core.Display = lambda lines, out: print(*lines, file=out)
    fire.Fire(
        {
            "by_feature_id": crop_by_feature_id,
            "by_grid": crop_by_grid,
            "by_center": crop_by_center,
            "by_bbox": crop_by_bbox,
            "by_polygon": crop_by_polygon,
        }
    )
