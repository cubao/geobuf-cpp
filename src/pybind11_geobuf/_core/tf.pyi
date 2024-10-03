from __future__ import annotations
import numpy
import typing

__all__ = [
    "R_ecef_enu",
    "T_ecef_enu",
    "apply_transform",
    "apply_transform_inplace",
    "cheap_ruler_k",
    "ecef2enu",
    "ecef2lla",
    "enu2ecef",
    "enu2lla",
    "lla2ecef",
    "lla2enu",
]

def R_ecef_enu(lon: float, lat: float) -> numpy.ndarray[numpy.float64[3, 3]]: ...
@typing.overload
def T_ecef_enu(
    lon: float, lat: float, alt: float
) -> numpy.ndarray[numpy.float64[4, 4]]: ...
@typing.overload
def T_ecef_enu(
    lla: numpy.ndarray[numpy.float64[3, 1]],
) -> numpy.ndarray[numpy.float64[4, 4]]: ...
def apply_transform(
    T: numpy.ndarray[numpy.float64[4, 4]],
    coords: numpy.ndarray[numpy.float64[m, 3], numpy.ndarray.flags.c_contiguous],
) -> numpy.ndarray[numpy.float64[m, 3]]: ...
def apply_transform_inplace(
    T: numpy.ndarray[numpy.float64[4, 4]],
    coords: numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ],
    *,
    batch_size: int = 1000,
) -> None: ...
def cheap_ruler_k(latitude: float) -> numpy.ndarray[numpy.float64[3, 1]]: ...
def ecef2enu(
    ecefs: numpy.ndarray[numpy.float64[m, 3], numpy.ndarray.flags.c_contiguous],
    *,
    anchor_lla: numpy.ndarray[numpy.float64[3, 1]] | None = None,
    cheap_ruler: bool = False,
) -> numpy.ndarray[numpy.float64[m, 3]]: ...
@typing.overload
def ecef2lla(x: float, y: float, z: float) -> numpy.ndarray[numpy.float64[3, 1]]: ...
@typing.overload
def ecef2lla(
    ecefs: numpy.ndarray[numpy.float64[m, 3], numpy.ndarray.flags.c_contiguous],
) -> numpy.ndarray[numpy.float64[m, 3]]: ...
def enu2ecef(
    enus: numpy.ndarray[numpy.float64[m, 3], numpy.ndarray.flags.c_contiguous],
    *,
    anchor_lla: numpy.ndarray[numpy.float64[3, 1]],
    cheap_ruler: bool = False,
) -> numpy.ndarray[numpy.float64[m, 3]]: ...
def enu2lla(
    enus: numpy.ndarray[numpy.float64[m, 3], numpy.ndarray.flags.c_contiguous],
    *,
    anchor_lla: numpy.ndarray[numpy.float64[3, 1]],
    cheap_ruler: bool = True,
) -> numpy.ndarray[numpy.float64[m, 3]]: ...
@typing.overload
def lla2ecef(
    lon: float, lat: float, alt: float
) -> numpy.ndarray[numpy.float64[3, 1]]: ...
@typing.overload
def lla2ecef(
    llas: numpy.ndarray[numpy.float64[m, 3], numpy.ndarray.flags.c_contiguous],
) -> numpy.ndarray[numpy.float64[m, 3]]: ...
def lla2enu(
    llas: numpy.ndarray[numpy.float64[m, 3], numpy.ndarray.flags.c_contiguous],
    *,
    anchor_lla: numpy.ndarray[numpy.float64[3, 1]] | None = None,
    cheap_ruler: bool = True,
) -> numpy.ndarray[numpy.float64[m, 3]]: ...
