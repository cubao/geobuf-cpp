import os
from typing import List, Optional, Tuple, Union  # noqa

import numpy as np
from _pybind11_geobuf import geojson, tf
from loguru import logger


def bbox2polygon(bbox: np.ndarray):
    lon0, lat0, lon1, lat1 = bbox
    return np.array(
        [
            [lon0, lat0, 0.0],
            [lon1, lat0, 0.0],
            [lon1, lat1, 0.0],
            [lon0, lat1, 0.0],
            [lon0, lat0, 0.0],
        ]
    )


def crop_by_feature_id(
    input_path: str,
    output_path: str,
    *,
    feature_id: str,
    buffer: Union[float, Tuple[float, float]] = 100.0,
    clipping_mode: str = "longest",
    max_z_offset: float = None,
) -> bool:
    if not feature_id:
        logger.info(
            f"invalid feature id: {feature_id} (type: {type(feature_id)})"
        )  # noqa
        return False
    g = geojson.GeoJSON().load(input_path)
    if not g.is_feature_collection():
        logger.warning(f"{input_path} is not valid GeoJSON FeatureCollection")
        return False

    fc = g.as_feature_collection()
    bbox = None
    height = None
    for f in fc:
        props = f.properties()
        if "id" not in props:
            continue
        fid = props["id"]()
        if fid == feature_id:
            bbox = f.bbox()
            if max_z_offset is not None:
                height = f.bbox(with_z=True)[2::3].mean()
    if bbox is None:
        logger.error(f"not any feature matched by id: {feature_id}")
        return False

    dlon, dlat = 1.0 / tf.cheap_ruler_k(bbox[1::2].mean())[:2]
    if isinstance(buffer, (int, float, np.generic)):
        dlon *= buffer
        dlat *= buffer
    else:
        dlon *= buffer[0]
        dlat *= buffer[1]
    bbox += [-dlon, -dlat, dlon, dlat]
    logger.info(f"bbox: {bbox}")

    polygon = bbox2polygon(bbox)
    if height is not None:
        polygon[:, 2] = height
    logger.info(f"polygon:\n{polygon}")

    logger.info(f"writing to {output_path} ...")
    os.makedirs(os.path.dirname(os.path.abspath(output_path)), exist_ok=True)
    cropped = g.crop(
        polygon,
        clipping_mode=clipping_mode,
        max_z_offset=max_z_offset,
    )
    return cropped.to_rapidjson().sort_keys().dump(output_path, indent=True)


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
