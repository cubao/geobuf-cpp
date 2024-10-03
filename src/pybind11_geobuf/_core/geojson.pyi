from __future__ import annotations
import numpy
import pybind11_geobuf._core
import typing

__all__ = [
    "Feature",
    "FeatureCollection",
    "FeatureList",
    "GeoJSON",
    "Geometry",
    "GeometryBase",
    "GeometryCollection",
    "GeometryList",
    "LineString",
    "LineStringList",
    "LinearRing",
    "LinearRingList",
    "MultiLineString",
    "MultiPoint",
    "MultiPolygon",
    "Point",
    "Polygon",
    "PolygonList",
    "coordinates",
    "value",
]

class Feature:
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the feature to a Python dict
        """
    def __copy__(self, arg0: dict) -> Feature:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> Feature:
        """
        Create a deep copy of the object
        """
    def __delitem__(self, arg0: str) -> int:
        """
        Delete a custom property by key
        """
    def __eq__(self, arg0: Feature) -> bool:
        """
        Check if two features are equal
        """
    def __getitem__(self, arg0: str) -> value:
        """
        Get a custom property value by key
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for GeoJSON Feature
        """
    @typing.overload
    def __init__(self, arg0: Feature) -> None:
        """
        Copy constructor for GeoJSON Feature
        """
    @typing.overload
    def __init__(self, arg0: pybind11_geobuf._core.rapidjson) -> None:
        """
        Construct a GeoJSON Feature from a RapidJSON value
        """
    @typing.overload
    def __init__(self, arg0: dict) -> None:
        """
        Construct a GeoJSON Feature from a Python dict
        """
    def __ne__(self, arg0: Feature) -> bool:
        """
        Check if two features are not equal
        """
    def __setitem__(self, arg0: str, arg1: typing.Any) -> typing.Any:
        """
        Set a custom property value by key
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Get a numpy view of the feature geometry
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Compute the bounding box of the feature
        """
    def clear(self) -> Feature:
        """
        Clear all properties and geometry of the feature
        """
    def clone(self) -> Feature:
        """
        Create a clone of the object
        """
    @typing.overload
    def custom_properties(self) -> value.object_type:
        """
        Get the 'custom_properties' attribute
        """
    @typing.overload
    def custom_properties(self, new_value: value.object_type) -> Feature:
        """
        Set the 'custom_properties' attribute
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def dump(
        self,
        path: str,
        *,
        indent: bool = False,
        sort_keys: bool = False,
        precision: int = 8,
        only_xy: bool = False,
    ) -> bool:
        """
        Dump the feature to a file (GeoJSON or Geobuf)
        """
    def from_geobuf(self, arg0: str) -> Feature:
        """
        Initialize the feature from Geobuf bytes
        """
    @typing.overload
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> Feature:
        """
        Initialize the feature from a RapidJSON value
        """
    @typing.overload
    def from_rapidjson(self, arg0: typing.Any) -> Feature:
        """
        Initialize the feature from a Python object
        """
    @typing.overload
    def geometry(self) -> Geometry:
        """
        Get the 'geometry' attribute
        """
    @typing.overload
    def geometry(self, new_value: Geometry) -> Feature:
        """
        Set the 'geometry' attribute
        """
    @typing.overload
    def geometry(self, point: Point) -> Feature:
        """
        Set the geometry of the feature to the given geometry object
        """
    @typing.overload
    def geometry(self, multi_point: MultiPoint) -> Feature:
        """
        Set the geometry of the feature to the given geometry object
        """
    @typing.overload
    def geometry(self, line_string: LineString) -> Feature:
        """
        Set the geometry of the feature to the given geometry object
        """
    @typing.overload
    def geometry(self, multi_line_string: MultiLineString) -> Feature:
        """
        Set the geometry of the feature to the given geometry object
        """
    @typing.overload
    def geometry(self, polygon: Polygon) -> Feature:
        """
        Set the geometry of the feature to the given geometry object
        """
    @typing.overload
    def geometry(self, multi_polygon: MultiPolygon) -> Feature:
        """
        Set the geometry of the feature to the given geometry object
        """
    @typing.overload
    def geometry(self, arg0: typing.Any) -> Feature:
        """
        Set the geometry of the feature from a Python object
        """
    @typing.overload
    def id(self) -> typing.Any:
        """
        Get the feature ID
        """
    @typing.overload
    def id(self, arg0: typing.Any) -> Feature:
        """
        Set the feature ID
        """
    def items(self) -> typing.Iterator[tuple[str, value]]:
        """
        Get an iterator over custom property items
        """
    def keys(self) -> typing.Iterator[str]:
        """
        Get an iterator over custom property keys
        """
    def load(self, arg0: str) -> Feature:
        """
        Load a feature from a file (GeoJSON or Geobuf)
        """
    @typing.overload
    def properties(self) -> value.object_type:
        """
        Get the 'properties' attribute
        """
    @typing.overload
    def properties(self, new_value: value.object_type) -> Feature:
        """
        Set the 'properties' attribute
        """
    @typing.overload
    def properties(self, arg0: typing.Any) -> Feature:
        """
        Set the properties of the feature from a Python object
        """
    @typing.overload
    def properties(self, arg0: str) -> value:
        """
        Get a property value by key
        """
    @typing.overload
    def properties(self, arg0: str, arg1: typing.Any) -> Feature:
        """
        Set a property value by key
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> Feature:
        """
        Round the coordinates of the feature geometry
        """
    def to_geobuf(
        self, *, precision: int = 8, only_xy: bool = False, round_z: int | None = None
    ) -> bytes:
        """
        Convert the feature to Geobuf bytes
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert the feature geometry to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the feature to a RapidJSON value
        """

class FeatureCollection(FeatureList):
    def __call__(self) -> typing.Any:
        """
        Convert the FeatureCollection to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> FeatureCollection:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> FeatureCollection:
        """
        Create a deep copy of the object
        """
    @typing.overload
    def __delitem__(self, arg0: str) -> int:
        """
        Delete a custom property
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: str) -> value:
        """
        Get a custom property by key
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> Feature:
        """
        Get a feature from the collection by index
        """
    @typing.overload
    def __getitem__(self, s: slice) -> FeatureCollection:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Initialize an empty FeatureCollection
        """
    @typing.overload
    def __init__(self, arg0: FeatureCollection) -> None:
        """
        Initialize a FeatureCollection from another FeatureCollection
        """
    @typing.overload
    def __init__(self, N: int) -> None:
        """
        Initialize a FeatureCollection with N empty features
        """
    @typing.overload
    def __setitem__(self, arg0: str, arg1: typing.Any) -> typing.Any:
        """
        Set a custom property
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Feature) -> None:
        """
        Set a feature in the collection at the specified index
        """
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: FeatureCollection) -> None:
        """
        Assign list elements using a slice object
        """
    def clone(self) -> FeatureCollection:
        """
        Create a clone of the object
        """
    @typing.overload
    def custom_properties(self) -> value.object_type:
        """
        Get the 'custom_properties' attribute
        """
    @typing.overload
    def custom_properties(self, new_value: value.object_type) -> FeatureCollection:
        """
        Set the 'custom_properties' attribute
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def dump(
        self,
        path: str,
        *,
        indent: bool = False,
        sort_keys: bool = False,
        precision: int = 8,
        only_xy: bool = False,
    ) -> bool:
        """
        Dump the FeatureCollection to a file (GeoJSON or Geobuf)
        """
    def from_geobuf(self, arg0: str) -> FeatureCollection:
        """
        Load the FeatureCollection from Geobuf bytes
        """
    def from_rapidjson(
        self, arg0: pybind11_geobuf._core.rapidjson
    ) -> FeatureCollection:
        """
        Load the FeatureCollection from a RapidJSON value
        """
    def items(self) -> typing.Iterator[tuple[str, value]]:
        """
        Return an iterator over the items of custom properties
        """
    def keys(self) -> typing.Iterator[str]:
        """
        Return an iterator over the keys of custom properties
        """
    def load(self, arg0: str) -> FeatureCollection:
        """
        Load the FeatureCollection from a file (GeoJSON or Geobuf)
        """
    def resize(self, arg0: int) -> FeatureCollection:
        """
        Resize the FeatureCollection to contain N features
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> FeatureCollection:
        """
        Round the coordinates of all features in the collection
        """
    def to_geobuf(
        self, *, precision: int = 8, only_xy: bool = False, round_z: int | None = None
    ) -> bytes:
        """
        Convert the FeatureCollection to Geobuf bytes
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the FeatureCollection to a RapidJSON value
        """
    def values(self) -> typing.Iterator[value]:
        """
        Return an iterator over the values of custom properties
        """

class FeatureList:
    __hash__: typing.ClassVar[None] = None
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    def __call__(self) -> typing.Any: ...
    def __contains__(self, x: Feature) -> bool:
        """
        Return true the container contains ``x``
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    def __eq__(self, arg0: FeatureList) -> bool: ...
    @typing.overload
    def __getitem__(self, s: slice) -> FeatureList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> Feature: ...
    @typing.overload
    def __init__(self) -> None: ...
    @typing.overload
    def __init__(self, arg0: FeatureList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: typing.Iterable) -> None: ...
    def __iter__(self) -> typing.Iterator[Feature]: ...
    def __len__(self) -> int: ...
    def __ne__(self, arg0: FeatureList) -> bool: ...
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Feature) -> None: ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: FeatureList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: Feature) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    def count(self, x: Feature) -> int:
        """
        Return the number of times ``x`` appears in the list
        """
    @typing.overload
    def extend(self, L: FeatureList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: typing.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: int, x: Feature) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> Feature:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: int) -> Feature:
        """
        Remove and return the item at index ``i``
        """
    def remove(self, x: Feature) -> None:
        """
        Remove the first item from the list whose value is x. It is an error if there is no such item.
        """

class GeoJSON:
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the GeoJSON object to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> GeoJSON:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> GeoJSON:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: GeoJSON) -> bool:
        """
        Check if two GeoJSON objects are equal
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Create an empty GeoJSON object
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Create a GeoJSON object from a geometry
        """
    @typing.overload
    def __init__(self, arg0: ...) -> None:
        """
        Create a GeoJSON object from a feature
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Create a GeoJSON object from a feature collection
        """
    def __ne__(self, arg0: GeoJSON) -> bool:
        """
        Check if two GeoJSON objects are not equal
        """
    def as_feature(self) -> ...:
        """
        Get this GeoJSON object as a feature (if it is one)
        """
    def as_feature_collection(self) -> ...:
        """
        Get this GeoJSON object as a feature_collection (if it is one)
        """
    def as_geometry(self) -> ...:
        """
        Get this GeoJSON object as a geometry (if it is one)
        """
    def clone(self) -> GeoJSON:
        """
        Create a clone of the object
        """
    def crop(
        self,
        polygon: numpy.ndarray[numpy.float64[m, 3]],
        *,
        clipping_mode: str = "longest",
        max_z_offset: float | None = None,
    ) -> ...:
        """
        Crop the GeoJSON object using a polygon
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def dump(
        self,
        path: str,
        *,
        indent: bool = False,
        sort_keys: bool = False,
        precision: int = 8,
        only_xy: bool = False,
    ) -> bool:
        """
        Dump the GeoJSON object to a file
        """
    def from_geobuf(self, arg0: str) -> GeoJSON:
        """
        Decode a Geobuf byte string into a GeoJSON object
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> GeoJSON:
        """
        Convert a RapidJSON value to a GeoJSON object
        """
    def is_feature(self) -> bool:
        """
        Check if this GeoJSON object is of type feature
        """
    def is_feature_collection(self) -> bool:
        """
        Check if this GeoJSON object is of type feature_collection
        """
    def is_geometry(self) -> bool:
        """
        Check if this GeoJSON object is of type geometry
        """
    def load(self, arg0: str) -> GeoJSON:
        """
        Load a GeoJSON object from a file
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> GeoJSON:
        """
        Round coordinates to specified decimal places
        """
    def to_geobuf(
        self, *, precision: int = 8, only_xy: bool = False, round_z: int | None = None
    ) -> bytes:
        """
        Encode the GeoJSON object to a Geobuf byte string
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the GeoJSON object to a RapidJSON value
        """

class Geometry(GeometryBase):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the geometry to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> Geometry:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> Geometry:
        """
        Create a deep copy of the object
        """
    def __delitem__(self, arg0: str) -> int:
        """
        Delete a custom property
        """
    def __eq__(self, arg0: Geometry) -> bool:
        """
        Check if two geometries are equal
        """
    def __getitem__(self, key: str) -> ...:
        """
        Get a custom property value
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Initialize an empty geometry
        """
    @typing.overload
    def __init__(self, arg0: ...) -> None:
        """
        Initialize from a Point
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Initialize from a MultiPoint
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Initialize from a LineString
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Initialize from a MultiLineString
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Initialize from a Polygon
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Initialize from a MultiPolygon
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Initialize from a GeometryCollection
        """
    @typing.overload
    def __init__(self, arg0: Geometry) -> None:
        """
        Initialize from another Geometry
        """
    @typing.overload
    def __init__(self, arg0: ..., std: ...) -> None:
        """
        Initialize from a GeometryCollection
        """
    @typing.overload
    def __init__(self, arg0: pybind11_geobuf._core.rapidjson) -> None:
        """
        Initialize from a RapidJSON value
        """
    @typing.overload
    def __init__(self, arg0: dict) -> None:
        """
        Initialize from a Python dictionary
        """
    def __iter__(self) -> typing.Iterator[str]:
        """
        Get an iterator over the custom property keys
        """
    def __len__(self) -> int:
        """
        Get the number of coordinates or sub-geometries
        """
    def __ne__(self, arg0: Geometry) -> bool:
        """
        Check if two geometries are not equal
        """
    def __setitem__(self, arg0: str, arg1: typing.Any) -> typing.Any:
        """
        Set a custom property value
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Pickle support for Geometry objects
        """
    def as_geometry_collection(self) -> ...:
        """
        Get this geometry as a geometry_collection (if it is one)
        """
    def as_line_string(self) -> ...:
        """
        Get this geometry as a line_string (if it is one)
        """
    def as_multi_line_string(self) -> ...:
        """
        Get this geometry as a multi_line_string (if it is one)
        """
    def as_multi_point(self) -> ...:
        """
        Get this geometry as a multi_point (if it is one)
        """
    def as_multi_polygon(self) -> ...:
        """
        Get this geometry as a multi_polygon (if it is one)
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Get a numpy view of the geometry coordinates
        """
    def as_point(self) -> ...:
        """
        Get this geometry as a point (if it is one)
        """
    def as_polygon(self) -> ...:
        """
        Get this geometry as a polygon (if it is one)
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Get the bounding box of the geometry
        """
    def clear(self) -> Geometry:
        """
        Clear the geometry and custom properties
        """
    def clone(self) -> Geometry:
        """
        Create a clone of the object
        """
    @typing.overload
    def custom_properties(self) -> ...:
        """
        Get the 'custom_properties' attribute
        """
    @typing.overload
    def custom_properties(self, new_value: ...) -> Geometry:
        """
        Set the 'custom_properties' attribute
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def dump(
        self,
        path: str,
        *,
        indent: bool = False,
        sort_keys: bool = False,
        precision: int = 8,
        only_xy: bool = False,
    ) -> bool:
        """
        Dump the geometry to a file
        """
    def from_geobuf(self, arg0: str) -> Geometry:
        """
        Decode a Geobuf byte string into a geometry
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> Geometry:
        """
        Set geometry coordinates from a numpy array
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> Geometry:
        """
        Convert a RapidJSON value to a geometry
        """
    def get(self, key: str) -> ...:
        """
        Get a custom property value, returns None if not found
        """
    def is_empty(self) -> bool:
        """
        Check if this geometry is of type empty
        """
    def is_geometry_collection(self) -> bool:
        """
        Check if this geometry is of type geometry_collection
        """
    def is_line_string(self) -> bool:
        """
        Check if this geometry is of type line_string
        """
    def is_multi_line_string(self) -> bool:
        """
        Check if this geometry is of type multi_line_string
        """
    def is_multi_point(self) -> bool:
        """
        Check if this geometry is of type multi_point
        """
    def is_multi_polygon(self) -> bool:
        """
        Check if this geometry is of type multi_polygon
        """
    def is_point(self) -> bool:
        """
        Check if this geometry is of type point
        """
    def is_polygon(self) -> bool:
        """
        Check if this geometry is of type polygon
        """
    def items(self) -> typing.Iterator[tuple[str, ...]]:
        """
        Get an iterator over the custom property items
        """
    def keys(self) -> typing.Iterator[str]:
        """
        Get an iterator over the custom property keys
        """
    def load(self, arg0: str) -> Geometry:
        """
        Load a geometry from a file
        """
    def pop_back(self) -> Geometry:
        """
        Remove the last point or sub-geometry
        """
    @typing.overload
    def push_back(self, arg0: ...) -> Geometry:
        """
        Add a point to the geometry
        """
    @typing.overload
    def push_back(self, arg0: numpy.ndarray[numpy.float64[m, 1]]) -> Geometry:
        """
        Add a point to the geometry
        """
    @typing.overload
    def push_back(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> Geometry:
        """
        Add multiple points to the geometry
        """
    @typing.overload
    def push_back(self, arg0: Geometry) -> Geometry:
        """
        Add a sub-geometry to the geometry
        """
    @typing.overload
    def push_back(self, arg0: ..., std: ...) -> Geometry:
        """
        Add a polygon to a multi-polygon geometry
        """
    @typing.overload
    def push_back(self, arg0: ..., std: ...) -> Geometry:
        """
        Add a line string to a multi-line string geometry
        """
    def resize(self, arg0: int) -> Geometry:
        """
        Resize the geometry
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> Geometry:
        """
        Round coordinates to specified decimal places
        """
    def to_geobuf(
        self, *, precision: int = 8, only_xy: bool = False, round_z: int | None = None
    ) -> bytes:
        """
        Encode the geometry to a Geobuf byte string
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert geometry coordinates to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the geometry to a RapidJSON value
        """
    def type(self) -> str:
        """
        Get the type of the geometry
        """
    def values(self) -> typing.Iterator[...]:
        """
        Get an iterator over the custom property values
        """
    @property
    def __geo_interface__(self) -> typing.Any: ...

class GeometryBase:
    pass

class GeometryCollection(GeometryList):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the GeometryCollection to a Python object
        """
    def __eq__(self, arg0: GeometryCollection) -> bool:
        """
        Check if two GeometryCollections are equal
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for GeometryCollection
        """
    @typing.overload
    def __init__(self, arg0: GeometryCollection) -> None:
        """
        Copy constructor for GeometryCollection
        """
    @typing.overload
    def __init__(self, N: int) -> None:
        """
        Construct a GeometryCollection with N empty geometries
        """
    def __ne__(self, arg0: GeometryCollection) -> bool:
        """
        Check if two GeometryCollections are not equal
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Geometry) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Point) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: MultiPoint) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: LineString) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: MultiLineString) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Polygon) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: MultiPolygon) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: GeometryCollection) -> GeometryCollection:
        """
        Set a geometry in the GeometryCollection by index
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Pickle support for GeometryCollection
        """
    def clear(self) -> GeometryCollection:
        """
        Clear all geometries from the GeometryCollection
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def from_rapidjson(
        self, arg0: pybind11_geobuf._core.rapidjson
    ) -> GeometryCollection:
        """
        Set the GeometryCollection from a RapidJSON value
        """
    def pop_back(self) -> GeometryCollection:
        """
        Remove the last geometry from the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: Geometry) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: Point) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: MultiPoint) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: LineString) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: MultiLineString) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: Polygon) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: MultiPolygon) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    @typing.overload
    def push_back(self, arg0: GeometryCollection) -> GeometryCollection:
        """
        Add a new geometry to the GeometryCollection
        """
    def resize(self, arg0: int) -> GeometryCollection:
        """
        Resize the GeometryCollection to contain N geometries
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> GeometryCollection:
        """
        Round the coordinates of all geometries in the GeometryCollection
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the GeometryCollection to a RapidJSON value
        """
    @property
    def __geo_interface__(self) -> typing.Any:
        """
        Return the __geo_interface__ representation of the GeometryCollection
        """

class GeometryList:
    __hash__: typing.ClassVar[None] = None
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    def __contains__(self, x: Geometry) -> bool:
        """
        Return true the container contains ``x``
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    def __eq__(self, arg0: GeometryList) -> bool: ...
    @typing.overload
    def __getitem__(self, s: slice) -> GeometryList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> Geometry: ...
    @typing.overload
    def __init__(self) -> None: ...
    @typing.overload
    def __init__(self, arg0: GeometryList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: typing.Iterable) -> None: ...
    def __iter__(self) -> typing.Iterator[Geometry]: ...
    def __len__(self) -> int: ...
    def __ne__(self, arg0: GeometryList) -> bool: ...
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Geometry) -> None: ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: GeometryList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: Geometry) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    def count(self, x: Geometry) -> int:
        """
        Return the number of times ``x`` appears in the list
        """
    @typing.overload
    def extend(self, L: GeometryList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: typing.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: int, x: Geometry) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> Geometry:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: int) -> Geometry:
        """
        Remove and return the item at index ``i``
        """
    def remove(self, x: Geometry) -> None:
        """
        Remove the first item from the list whose value is x. It is an error if there is no such item.
        """

class LineString(coordinates):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the geometry to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> LineString:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> LineString:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: LineString) -> bool:
        """
        Check if two LineStrings are equal
        """
    def __getitem__(self, arg0: int) -> Point:
        """
        Get a point from the geometry by index
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for LineString
        """
    @typing.overload
    def __init__(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> None:
        """
        Initialize from a numpy array of points
        """
    def __iter__(self) -> typing.Iterator[Point]:
        """
        Iterate over the points in the geometry
        """
    def __len__(self) -> int:
        """
        Get the number of points in the geometry
        """
    def __ne__(self, arg0: LineString) -> bool:
        """
        Check if two LineStrings are not equal
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Point) -> Point:
        """
        Set a point in the geometry by index
        """
    @typing.overload
    def __setitem__(
        self, arg0: int, arg1: numpy.ndarray[numpy.float64[m, 1]]
    ) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Set a point in the geometry by index using a vector
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Pickle support for serialization
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Get a numpy view of the geometry points
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Compute the bounding box of the geometry
        """
    def clear(self) -> LineString:
        """
        Clear all points from the geometry
        """
    def clone(self) -> LineString:
        """
        Create a clone of the object
        """
    @typing.overload
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    @typing.overload
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> LineString:
        """
        Set the geometry points from a numpy array
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> LineString:
        """
        Initialize from a RapidJSON value
        """
    def pop_back(self) -> LineString:
        """
        Remove the last point from the geometry
        """
    @typing.overload
    def push_back(self, arg0: Point) -> LineString:
        """
        Add a point to the end of the geometry
        """
    @typing.overload
    def push_back(self, arg0: numpy.ndarray[numpy.float64[m, 1]]) -> LineString:
        """
        Add a point to the end of the geometry using a vector
        """
    def resize(self, arg0: int) -> LineString:
        """
        Resize the geometry to the specified size
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> LineString:
        """
        Round coordinates to specified decimal places
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert the geometry points to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert to a RapidJSON value
        """
    @property
    def __geo_interface__(self) -> typing.Any:
        """
        Return the __geo_interface__ representation
        """

class LineStringList:
    """
    A list of LineStrings
    """

    __hash__: typing.ClassVar[None] = None
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    def __contains__(self, x: LineString) -> bool:
        """
        Return true the container contains ``x``
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    def __eq__(self, arg0: LineStringList) -> bool: ...
    @typing.overload
    def __getitem__(self, s: slice) -> LineStringList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> LineString: ...
    @typing.overload
    def __init__(self) -> None: ...
    @typing.overload
    def __init__(self, arg0: LineStringList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: typing.Iterable) -> None: ...
    def __iter__(self) -> typing.Iterator[LineString]: ...
    def __len__(self) -> int: ...
    def __ne__(self, arg0: LineStringList) -> bool: ...
    @typing.overload
    def __setitem__(self, arg0: int, arg1: LineString) -> None: ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: LineStringList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: LineString) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    def count(self, x: LineString) -> int:
        """
        Return the number of times ``x`` appears in the list
        """
    @typing.overload
    def extend(self, L: LineStringList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: typing.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: int, x: LineString) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> LineString:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: int) -> LineString:
        """
        Remove and return the item at index ``i``
        """
    def remove(self, x: LineString) -> None:
        """
        Remove the first item from the list whose value is x. It is an error if there is no such item.
        """

class LinearRing(coordinates):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the geometry to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> LinearRing:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> LinearRing:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: LinearRing) -> bool:
        """
        Check if two LinearRings are equal
        """
    def __getitem__(self, arg0: int) -> Point:
        """
        Get a point from the geometry by index
        """
    def __init__(self) -> None:
        """
        Default constructor for LinearRing
        """
    def __iter__(self) -> typing.Iterator[Point]:
        """
        Iterate over the points in the geometry
        """
    def __len__(self) -> int:
        """
        Get the number of points in the geometry
        """
    def __ne__(self, arg0: LinearRing) -> bool:
        """
        Check if two LinearRings are not equal
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Point) -> Point:
        """
        Set a point in the geometry by index
        """
    @typing.overload
    def __setitem__(
        self, arg0: int, arg1: numpy.ndarray[numpy.float64[m, 1]]
    ) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Set a point in the geometry by index using a vector
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Get a numpy view of the geometry points
        """
    def clear(self) -> LinearRing:
        """
        Clear all points from the geometry
        """
    def clone(self) -> LinearRing:
        """
        Create a clone of the object
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> LinearRing:
        """
        Set the geometry points from a numpy array
        """
    def pop_back(self) -> LinearRing:
        """
        Remove the last point from the geometry
        """
    @typing.overload
    def push_back(self, arg0: Point) -> LinearRing:
        """
        Add a point to the end of the geometry
        """
    @typing.overload
    def push_back(self, arg0: numpy.ndarray[numpy.float64[m, 1]]) -> LinearRing:
        """
        Add a point to the end of the geometry using a vector
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert the geometry points to a numpy array
        """

class LinearRingList:
    """
    A list of LinearRings
    """

    __hash__: typing.ClassVar[None] = None
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    def __contains__(self, x: LinearRing) -> bool:
        """
        Return true the container contains ``x``
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    def __eq__(self, arg0: LinearRingList) -> bool: ...
    @typing.overload
    def __getitem__(self, s: slice) -> LinearRingList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> LinearRing: ...
    @typing.overload
    def __init__(self) -> None: ...
    @typing.overload
    def __init__(self, arg0: LinearRingList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: typing.Iterable) -> None: ...
    def __iter__(self) -> typing.Iterator[LinearRing]: ...
    def __len__(self) -> int: ...
    def __ne__(self, arg0: LinearRingList) -> bool: ...
    @typing.overload
    def __setitem__(self, arg0: int, arg1: LinearRing) -> None: ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: LinearRingList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: LinearRing) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    def count(self, x: LinearRing) -> int:
        """
        Return the number of times ``x`` appears in the list
        """
    @typing.overload
    def extend(self, L: LinearRingList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: typing.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: int, x: LinearRing) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> LinearRing:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: int) -> LinearRing:
        """
        Remove and return the item at index ``i``
        """
    def remove(self, x: LinearRing) -> None:
        """
        Remove the first item from the list whose value is x. It is an error if there is no such item.
        """

class MultiLineString(LineStringList):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the geometry to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> MultiLineString:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> MultiLineString:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: MultiLineString) -> bool:
        """
        Check if two MultiLineStrings are equal
        """
    def __getitem__(self, arg0: int) -> LineString:
        """
        Get a linear ring by index
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for MultiLineString
        """
    @typing.overload
    def __init__(self, arg0: LineStringList) -> None:
        """
        Construct MultiLineString from a container of LineStrings
        """
    @typing.overload
    def __init__(self, arg0: coordinates) -> None:
        """
        Construct MultiLineString from a single LineString
        """
    @typing.overload
    def __init__(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> None:
        """
        Initialize from a numpy array of points
        """
    def __iter__(self) -> typing.Iterator[LineString]:
        """
        Return an iterator over the linear rings in the geometry
        """
    def __len__(self) -> int:
        """
        Return the number of linear rings in the geometry
        """
    def __ne__(self, arg0: MultiLineString) -> bool:
        """
        Check if two MultiLineStrings are not equal
        """
    def __setitem__(
        self,
        arg0: int,
        arg1: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous],
    ) -> numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]:
        """
        Set a linear ring by index using a numpy array of points
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Pickle support for the geometry
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Return a numpy view of the geometry's points
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Compute the bounding box of the geometry
        """
    def clear(self) -> MultiLineString:
        """
        Clear all linear rings from the geometry
        """
    def clone(self) -> MultiLineString:
        """
        Create a clone of the object
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> MultiLineString:
        """
        Set the geometry from a numpy array of points
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> MultiLineString:
        """
        Initialize the geometry from a RapidJSON value
        """
    def pop_back(self) -> MultiLineString:
        """
        Remove the last point from the last linear ring
        """
    @typing.overload
    def push_back(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> MultiLineString:
        """
        Add a new linear ring from a numpy array of points
        """
    @typing.overload
    def push_back(self, arg0: LineString) -> MultiLineString:
        """
        Add a new linear ring
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> MultiLineString:
        """
        Round the coordinates of the geometry
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert the geometry to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the geometry to a RapidJSON value
        """
    @property
    def __geo_interface__(self) -> typing.Any:
        """
        Return the __geo_interface__ representation of the geometry
        """

class MultiPoint(coordinates):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the geometry to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> MultiPoint:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> MultiPoint:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: MultiPoint) -> bool:
        """
        Check if two MultiPoints are equal
        """
    def __getitem__(self, arg0: int) -> Point:
        """
        Get a point from the geometry by index
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for MultiPoint
        """
    @typing.overload
    def __init__(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> None:
        """
        Initialize from a numpy array of points
        """
    def __iter__(self) -> typing.Iterator[Point]:
        """
        Iterate over the points in the geometry
        """
    def __len__(self) -> int:
        """
        Get the number of points in the geometry
        """
    def __ne__(self, arg0: MultiPoint) -> bool:
        """
        Check if two MultiPoints are not equal
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Point) -> Point:
        """
        Set a point in the geometry by index
        """
    @typing.overload
    def __setitem__(
        self, arg0: int, arg1: numpy.ndarray[numpy.float64[m, 1]]
    ) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Set a point in the geometry by index using a vector
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Pickle support for serialization
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Get a numpy view of the geometry points
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Compute the bounding box of the geometry
        """
    def clear(self) -> MultiPoint:
        """
        Clear all points from the geometry
        """
    def clone(self) -> MultiPoint:
        """
        Create a clone of the object
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> MultiPoint:
        """
        Set the geometry points from a numpy array
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> MultiPoint:
        """
        Initialize from a RapidJSON value
        """
    def pop_back(self) -> MultiPoint:
        """
        Remove the last point from the geometry
        """
    @typing.overload
    def push_back(self, arg0: Point) -> MultiPoint:
        """
        Add a point to the end of the geometry
        """
    @typing.overload
    def push_back(self, arg0: numpy.ndarray[numpy.float64[m, 1]]) -> MultiPoint:
        """
        Add a point to the end of the geometry using a vector
        """
    def resize(self, arg0: int) -> MultiPoint:
        """
        Resize the geometry to the specified size
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> MultiPoint:
        """
        Round coordinates to specified decimal places
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert the geometry points to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert to a RapidJSON value
        """
    @property
    def __geo_interface__(self) -> typing.Any:
        """
        Return the __geo_interface__ representation
        """

class MultiPolygon(PolygonList):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert MultiPolygon to a Python object
        """
    def __copy__(self, arg0: dict) -> MultiPolygon:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> MultiPolygon:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: MultiPolygon) -> bool:
        """
        Check if two MultiPolygons are equal
        """
    def __getitem__(self, arg0: int) -> Polygon:
        """
        Get a Polygon from the MultiPolygon by index
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for MultiPolygon
        """
    @typing.overload
    def __init__(self, arg0: MultiPolygon) -> None:
        """
        Copy constructor for MultiPolygon
        """
    @typing.overload
    def __init__(self, arg0: PolygonList) -> None:
        """
        Construct MultiPolygon from a container of Polygons
        """
    @typing.overload
    def __init__(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> None:
        """
        Construct MultiPolygon from a numpy array of points
        """
    def __iter__(self) -> typing.Iterator[Polygon]:
        """
        Return an iterator over the Polygons in the MultiPolygon
        """
    def __len__(self) -> int:
        """
        Return the number of Polygons in the MultiPolygon
        """
    def __ne__(self, arg0: MultiPolygon) -> bool:
        """
        Check if two MultiPolygons are not equal
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Polygon) -> Polygon:
        """
        Set a Polygon in the MultiPolygon by index
        """
    @typing.overload
    def __setitem__(
        self,
        arg0: int,
        arg1: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous],
    ) -> Polygon:
        """
        Set a Polygon in the MultiPolygon by index using a numpy array
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Pickle support for MultiPolygon
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Return a numpy view of the MultiPolygon coordinates
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Compute the bounding box of the MultiPolygon
        """
    def clear(self) -> MultiPolygon:
        """
        Clear all Polygons from the MultiPolygon
        """
    def clone(self) -> MultiPolygon:
        """
        Create a clone of the object
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> MultiPolygon:
        """
        Set MultiPolygon coordinates from a numpy array
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> MultiPolygon:
        """
        Set the MultiPolygon from a RapidJSON value
        """
    def pop_back(self) -> MultiPolygon:
        """
        Remove the last Polygon from the MultiPolygon
        """
    @typing.overload
    def push_back(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> MultiPolygon:
        """
        Add a new Polygon to the MultiPolygon from a numpy array
        """
    @typing.overload
    def push_back(self, arg0: Polygon) -> MultiPolygon:
        """
        Add a new Polygon to the MultiPolygon
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> MultiPolygon:
        """
        Round the coordinates of the MultiPolygon
        """
    def to_numpy(self: Polygon) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert MultiPolygon to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the MultiPolygon to a RapidJSON value
        """
    @property
    def __geo_interface__(self) -> typing.Any:
        """
        Return the __geo_interface__ representation of the MultiPolygon
        """

class Point:
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the Point to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> Point:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> Point:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: Point) -> bool:
        """
        Check if two Points are equal
        """
    def __getitem__(self, index: int) -> float:
        """
        Get the coordinate value at the specified index (0: x, 1: y, 2: z)
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Initialize an empty Point
        """
    @typing.overload
    def __init__(self, x: float, y: float, z: float = 0.0) -> None:
        """
        Initialize a Point with coordinates (x, y, z)
        """
    @typing.overload
    def __init__(self, arg0: numpy.ndarray[numpy.float64[m, 1]]) -> None:
        """
        Initialize a Point from a numpy array or vector
        """
    def __iter__(self) -> typing.Iterator[float]:
        """
        Return an iterator over the point's coordinates
        """
    def __len__(self) -> int:
        """
        Return the number of coordinates (always 3)
        """
    def __ne__(self, arg0: Point) -> bool:
        """
        Check if two Points are not equal
        """
    def __setitem__(self, index: int, value: float) -> float:
        """
        Set the coordinate value at the specified index (0: x, 1: y, 2: z)
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Enable pickling support for Point objects
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[numpy.float64[3, 1], numpy.ndarray.flags.writeable]:
        """
        Get a numpy view of the point coordinates
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Get the bounding box of the point
        """
    def clear(self) -> Point:
        """
        Reset all coordinates of the point to 0.0
        """
    def clone(self) -> Point:
        """
        Create a clone of the object
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def from_numpy(self, arg0: numpy.ndarray[numpy.float64[m, 1]]) -> Point:
        """
        Set point coordinates from a numpy array
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> Point:
        """
        Create a Point from a RapidJSON value
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> Point:
        """
        Round coordinates to specified decimal places
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[3, 1]]:
        """
        Convert point coordinates to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the Point to a RapidJSON value
        """
    @property
    def __geo_interface__(self) -> typing.Any:
        """
        Return the __geo_interface__ representation of the point
        """
    @property
    def x(self) -> float:
        """
        Get or set the x-coordinate of the point
        """
    @x.setter
    def x(self, arg1: float) -> None: ...
    @property
    def y(self) -> float:
        """
        Get or set the y-coordinate of the point
        """
    @y.setter
    def y(self, arg1: float) -> None: ...
    @property
    def z(self) -> float:
        """
        Get or set the z-coordinate of the point
        """
    @z.setter
    def z(self, arg1: float) -> None: ...

class Polygon(LinearRingList):
    __hash__: typing.ClassVar[None] = None
    def __call__(self) -> typing.Any:
        """
        Convert the geometry to a Python dictionary
        """
    def __copy__(self, arg0: dict) -> Polygon:
        """
        Create a shallow copy of the object
        """
    def __deepcopy__(self, memo: dict) -> Polygon:
        """
        Create a deep copy of the object
        """
    def __eq__(self, arg0: Polygon) -> bool:
        """
        Check if two Polygons are equal
        """
    def __getitem__(self, arg0: int) -> LinearRing:
        """
        Get a linear ring by index
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for Polygon
        """
    @typing.overload
    def __init__(self, arg0: LinearRingList) -> None:
        """
        Construct Polygon from a container of LinearRings
        """
    @typing.overload
    def __init__(self, arg0: coordinates) -> None:
        """
        Construct Polygon from a single LinearRing (shell)
        """
    @typing.overload
    def __init__(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> None:
        """
        Initialize from a numpy array of points
        """
    def __iter__(self) -> typing.Iterator[LinearRing]:
        """
        Return an iterator over the linear rings in the geometry
        """
    def __len__(self) -> int:
        """
        Return the number of linear rings in the geometry
        """
    def __ne__(self, arg0: Polygon) -> bool:
        """
        Check if two Polygons are not equal
        """
    def __setitem__(
        self,
        arg0: int,
        arg1: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous],
    ) -> numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]:
        """
        Set a linear ring by index using a numpy array of points
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Pickle support for the geometry
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Return a numpy view of the geometry's points
        """
    def bbox(self, *, with_z: bool = False) -> numpy.ndarray[numpy.float64[m, 1]]:
        """
        Compute the bounding box of the geometry
        """
    def clear(self) -> Polygon:
        """
        Clear all linear rings from the geometry
        """
    def clone(self) -> Polygon:
        """
        Create a clone of the object
        """
    def deduplicate_xyz(self) -> bool:
        """
        Remove duplicate consecutive points based on their XYZ coordinates
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> Polygon:
        """
        Set the geometry from a numpy array of points
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> Polygon:
        """
        Initialize the geometry from a RapidJSON value
        """
    def pop_back(self) -> Polygon:
        """
        Remove the last point from the last linear ring
        """
    @typing.overload
    def push_back(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> Polygon:
        """
        Add a new linear ring from a numpy array of points
        """
    @typing.overload
    def push_back(self, arg0: LinearRing) -> Polygon:
        """
        Add a new linear ring
        """
    def round(self, *, lon: int = 8, lat: int = 8, alt: int = 3) -> Polygon:
        """
        Round the coordinates of the geometry
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert the geometry to a numpy array
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the geometry to a RapidJSON value
        """
    @property
    def __geo_interface__(self) -> typing.Any:
        """
        Return the __geo_interface__ representation of the geometry
        """

class PolygonList:
    __hash__: typing.ClassVar[None] = None
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    def __contains__(self, x: Polygon) -> bool:
        """
        Return true the container contains ``x``
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    def __eq__(self, arg0: PolygonList) -> bool: ...
    @typing.overload
    def __getitem__(self, s: slice) -> PolygonList:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> Polygon: ...
    @typing.overload
    def __init__(self) -> None: ...
    @typing.overload
    def __init__(self, arg0: PolygonList) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: typing.Iterable) -> None: ...
    def __iter__(self) -> typing.Iterator[Polygon]: ...
    def __len__(self) -> int: ...
    def __ne__(self, arg0: PolygonList) -> bool: ...
    @typing.overload
    def __setitem__(self, arg0: int, arg1: Polygon) -> None: ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: PolygonList) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: Polygon) -> None:
        """
        Add an item to the end of the list
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    def count(self, x: Polygon) -> int:
        """
        Return the number of times ``x`` appears in the list
        """
    @typing.overload
    def extend(self, L: PolygonList) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: typing.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def insert(self, i: int, x: Polygon) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> Polygon:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: int) -> Polygon:
        """
        Remove and return the item at index ``i``
        """
    def remove(self, x: Polygon) -> None:
        """
        Remove the first item from the list whose value is x. It is an error if there is no such item.
        """

class coordinates:
    __hash__: typing.ClassVar[None] = None
    def __bool__(self) -> bool:
        """
        Check whether the list is nonempty
        """
    def __contains__(self, x: ...) -> bool:
        """
        Return true the container contains ``x``
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete the list elements at index ``i``
        """
    @typing.overload
    def __delitem__(self, arg0: slice) -> None:
        """
        Delete list elements using a slice object
        """
    def __eq__(self, arg0: coordinates) -> bool: ...
    @typing.overload
    def __getitem__(self, s: slice) -> coordinates:
        """
        Retrieve list elements using a slice object
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> ...: ...
    @typing.overload
    def __init__(self) -> None: ...
    @typing.overload
    def __init__(self, arg0: coordinates) -> None:
        """
        Copy constructor
        """
    @typing.overload
    def __init__(self, arg0: typing.Iterable) -> None: ...
    def __iter__(self) -> typing.Iterator[...]: ...
    def __len__(self) -> int: ...
    def __ne__(self, arg0: coordinates) -> bool: ...
    @typing.overload
    def __setitem__(self, arg0: int, arg1: ...) -> None: ...
    @typing.overload
    def __setitem__(self, arg0: slice, arg1: coordinates) -> None:
        """
        Assign list elements using a slice object
        """
    def append(self, x: ...) -> None:
        """
        Add an item to the end of the list
        """
    def as_numpy(
        self,
    ) -> numpy.ndarray[
        numpy.float64[m, 3],
        numpy.ndarray.flags.writeable,
        numpy.ndarray.flags.c_contiguous,
    ]:
        """
        Get a numpy view of the coordinates
        """
    def clear(self) -> None:
        """
        Clear the contents
        """
    def count(self, x: ...) -> int:
        """
        Return the number of times ``x`` appears in the list
        """
    @typing.overload
    def extend(self, L: coordinates) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    @typing.overload
    def extend(self, L: typing.Iterable) -> None:
        """
        Extend the list by appending all the items in the given list
        """
    def from_numpy(
        self, arg0: numpy.ndarray[numpy.float64[m, n], numpy.ndarray.flags.c_contiguous]
    ) -> coordinates:
        """
        Set coordinates from a numpy array
        """
    def insert(self, i: int, x: ...) -> None:
        """
        Insert an item at a given position.
        """
    @typing.overload
    def pop(self) -> ...:
        """
        Remove and return the last item
        """
    @typing.overload
    def pop(self, i: int) -> ...:
        """
        Remove and return the item at index ``i``
        """
    def remove(self, x: ...) -> None:
        """
        Remove the first item from the list whose value is x. It is an error if there is no such item.
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[m, 3]]:
        """
        Convert coordinates to a numpy array
        """

class value:
    class ItemsView:
        def __iter__(self) -> typing.Iterator: ...
        def __len__(self) -> int: ...

    class KeysView:
        def __contains__(self, arg0: typing.Any) -> bool: ...
        def __iter__(self) -> typing.Iterator: ...
        def __len__(self) -> int: ...

    class ValuesView:
        def __iter__(self) -> typing.Iterator: ...
        def __len__(self) -> int: ...

    class array_type:
        __hash__: typing.ClassVar[None] = None
        def __bool__(self) -> bool:
            """
            Check whether the list is nonempty
            """
        def __call__(self) -> typing.Any:
            """
            Convert the GeoJSON array to a Python list
            """
        def __contains__(self, x: value) -> bool:
            """
            Return true the container contains ``x``
            """
        @typing.overload
        def __delitem__(self, arg0: int) -> None:
            """
            Delete the list elements at index ``i``
            """
        @typing.overload
        def __delitem__(self, arg0: slice) -> None:
            """
            Delete list elements using a slice object
            """
        def __eq__(self, arg0: value.array_type) -> bool: ...
        @typing.overload
        def __getitem__(self, s: slice) -> value.array_type:
            """
            Retrieve list elements using a slice object
            """
        @typing.overload
        def __getitem__(self, arg0: int) -> value: ...
        @typing.overload
        def __getitem__(self, arg0: int) -> value:
            """
            Get an item from the GeoJSON array by index
            """
        @typing.overload
        def __init__(self) -> None: ...
        @typing.overload
        def __init__(self, arg0: value.array_type) -> None:
            """
            Copy constructor
            """
        @typing.overload
        def __init__(self, arg0: typing.Iterable) -> None: ...
        @typing.overload
        def __init__(self) -> None:
            """
            Default constructor for GeoJSON array
            """
        @typing.overload
        def __init__(self, arg0: typing.Any) -> None:
            """
            Construct a GeoJSON array from a Python iterable
            """
        def __iter__(self) -> typing.Iterator[value]: ...
        def __len__(self) -> int: ...
        def __ne__(self, arg0: value.array_type) -> bool: ...
        @typing.overload
        def __setitem__(self, arg0: int, arg1: value) -> None: ...
        @typing.overload
        def __setitem__(self, arg0: slice, arg1: value.array_type) -> None:
            """
            Assign list elements using a slice object
            """
        @typing.overload
        def __setitem__(self, arg0: int, arg1: typing.Any) -> value:
            """
            Set an item in the GeoJSON array by index
            """
        def append(self, x: value) -> None:
            """
            Add an item to the end of the list
            """
        @typing.overload
        def clear(self) -> None:
            """
            Clear the contents
            """
        @typing.overload
        def clear(self) -> value.array_type:
            """
            Clear the GeoJSON array
            """
        def count(self, x: value) -> int:
            """
            Return the number of times ``x`` appears in the list
            """
        @typing.overload
        def extend(self, L: value.array_type) -> None:
            """
            Extend the list by appending all the items in the given list
            """
        @typing.overload
        def extend(self, L: typing.Iterable) -> None:
            """
            Extend the list by appending all the items in the given list
            """
        def from_rapidjson(
            self, arg0: pybind11_geobuf._core.rapidjson
        ) -> value.array_type:
            """
            Set the GeoJSON array from a RapidJSON value
            """
        def insert(self, i: int, x: value) -> None:
            """
            Insert an item at a given position.
            """
        @typing.overload
        def pop(self) -> value:
            """
            Remove and return the last item
            """
        @typing.overload
        def pop(self, i: int) -> value:
            """
            Remove and return the item at index ``i``
            """
        def remove(self, x: value) -> None:
            """
            Remove the first item from the list whose value is x. It is an error if there is no such item.
            """
        def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
            """
            Convert the GeoJSON array to a RapidJSON value
            """

    class object_type:
        def __bool__(self) -> bool:
            """
            Check whether the map is nonempty
            """
        def __call__(self) -> typing.Any:
            """
            Convert the GeoJSON object to a Python dict
            """
        @typing.overload
        def __contains__(self, arg0: str) -> bool: ...
        @typing.overload
        def __contains__(self, arg0: typing.Any) -> bool: ...
        def __delitem__(self, arg0: str) -> None: ...
        def __getitem__(self, arg0: str) -> value: ...
        @typing.overload
        def __init__(self) -> None: ...
        @typing.overload
        def __init__(self) -> None:
            """
            Default constructor for GeoJSON object
            """
        @typing.overload
        def __init__(self, arg0: typing.Any) -> None:
            """
            Construct a GeoJSON object from a Python dict
            """
        def __iter__(self) -> typing.Iterator[str]: ...
        def __len__(self) -> int: ...
        @typing.overload
        def __setitem__(self, arg0: str, arg1: value) -> None: ...
        @typing.overload
        def __setitem__(self, arg0: str, arg1: typing.Any) -> value:
            """
            Set an item in the GeoJSON object by key
            """
        def clear(self) -> value.object_type:
            """
            Clear the GeoJSON object
            """
        def from_rapidjson(
            self, arg0: pybind11_geobuf._core.rapidjson
        ) -> value.object_type:
            """
            Convert a RapidJSON value to a GeoJSON object
            """
        @typing.overload
        def items(self) -> value.ItemsView: ...
        @typing.overload
        def items(self) -> typing.Iterator[tuple[str, value]]:
            """
            Get an iterator over the items (key-value pairs) of the GeoJSON object
            """
        @typing.overload
        def keys(self) -> value.KeysView: ...
        @typing.overload
        def keys(self) -> typing.Iterator[str]:
            """
            Get an iterator over the keys of the GeoJSON object
            """
        def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
            """
            Convert the GeoJSON object to a RapidJSON value
            """
        @typing.overload
        def values(self) -> value.ValuesView: ...
        @typing.overload
        def values(self) -> typing.Iterator[value]:
            """
            Get an iterator over the values of the GeoJSON object
            """

    def Get(self) -> typing.Any:
        """
        Get the GeoJSON value as a Python object
        """
    def GetBool(self) -> bool:
        """
        Get the GeoJSON value as a boolean
        """
    def GetDouble(self) -> float:
        """
        Get the GeoJSON value as a double
        """
    def GetInt64(self) -> int:
        """
        Get the GeoJSON value as a signed 64-bit integer
        """
    def GetString(self) -> str:
        """
        Get the GeoJSON value as a string
        """
    def GetType(self) -> str:
        """
        Get the type of the GeoJSON value
        """
    def GetUint64(self) -> int:
        """
        Get the GeoJSON value as an unsigned 64-bit integer
        """
    def __bool__(self) -> bool:
        """
        Check if the GeoJSON value is truthy
        """
    def __call__(self) -> typing.Any:
        """
        Convert the GeoJSON value to a Python object
        """
    @typing.overload
    def __delitem__(self, arg0: str) -> int:
        """
        Delete an item from the GeoJSON object by key
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete an item from the GeoJSON array by index
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> value:
        """
        Get an item from the GeoJSON array by index
        """
    @typing.overload
    def __getitem__(self, arg0: str) -> value:
        """
        Get an item from the GeoJSON object by key
        """
    @typing.overload
    def __init__(self) -> None:
        """
        Default constructor for GeoJSON value
        """
    @typing.overload
    def __init__(self, arg0: typing.Any) -> None:
        """
        Construct a GeoJSON value from a Python object
        """
    def __len__(self) -> int:
        """
        Get the length of the GeoJSON value
        """
    @typing.overload
    def __setitem__(self, arg0: str, arg1: typing.Any) -> typing.Any:
        """
        Set an item in the GeoJSON object by key
        """
    @typing.overload
    def __setitem__(self, arg0: int, arg1: typing.Any) -> typing.Any:
        """
        Set an item in the GeoJSON array by index
        """
    def as_array(self) -> ...:
        """
        Get the GeoJSON value as an array
        """
    def as_object(self) -> ...:
        """
        Get the GeoJSON value as an object
        """
    def clear(self) -> value:
        """
        Clear the GeoJSON value
        """
    def from_rapidjson(self, arg0: pybind11_geobuf._core.rapidjson) -> value:
        """
        Set the GeoJSON value from a RapidJSON value
        """
    def get(self, key: str) -> value:
        """
        Get an item from the GeoJSON object by key, returning None if not found
        """
    def is_array(self) -> bool:
        """
        Check if the GeoJSON value is an array
        """
    def is_object(self) -> bool:
        """
        Check if the GeoJSON value is an object
        """
    def items(self) -> typing.Iterator[tuple[str, value]]:
        """
        Get an iterator over the items of the GeoJSON object
        """
    def keys(self) -> typing.Iterator[str]:
        """
        Get an iterator over the keys of the GeoJSON object
        """
    def pop_back(self) -> value:
        """
        Remove the last value from the GeoJSON array
        """
    def push_back(self, arg0: typing.Any) -> value:
        """
        Add a value to the end of the GeoJSON array
        """
    def set(self, arg0: typing.Any) -> value:
        """
        Set the GeoJSON value from a Python object
        """
    def to_rapidjson(self) -> pybind11_geobuf._core.rapidjson:
        """
        Convert the GeoJSON value to a RapidJSON value
        """
    def values(self) -> typing.Iterator[value]:
        """
        Get an iterator over the values of the GeoJSON object
        """
