from __future__ import annotations
import numpy
import pybind11_stubgen.typing_ext
import typing
from . import geojson
from . import tf

__all__ = [
    "Decoder",
    "Encoder",
    "GeobufIndex",
    "NodeItem",
    "PackedRTree",
    "Planet",
    "geojson",
    "is_subset_of",
    "normalize_json",
    "pbf_decode",
    "rapidjson",
    "str2geojson2str",
    "str2json2str",
    "tf",
]

class Decoder:
    def __init__(self) -> None:
        """
        Initialize a Decoder object.
        """
    @typing.overload
    def decode(
        self, geobuf: str, *, indent: bool = False, sort_keys: bool = False
    ) -> str:
        """
        Decode Protocol Buffer (PBF) bytes to GeoJSON string.

        Args:
            geobuf (str): Input PBF bytes.
            indent (bool, optional): Whether to indent the output JSON. Defaults to False.
            sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

        Returns:
            str: Decoded GeoJSON as a string.
        """
    @typing.overload
    def decode(
        self,
        *,
        geobuf: str,
        geojson: str,
        indent: bool = False,
        sort_keys: bool = False,
    ) -> bool:
        """
        Decode Protocol Buffer (PBF) file to GeoJSON file.

        Args:
            geobuf (str): Path to input PBF file.
            geojson (str): Path to output GeoJSON file.
            indent (bool, optional): Whether to indent the output JSON. Defaults to False.
            sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

        Returns:
            None
        """
    def decode_feature(
        self, bytes: str, only_geometry: bool = False, only_properties: bool = False
    ) -> geojson.Feature | None:
        """
        Decode Protocol Buffer (PBF) feature.

        Args:
            bytes (str): Input PBF bytes.
            only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
            only_properties (bool, optional): Whether to decode only properties. Defaults to False.

        Returns:
            mapbox::geojson::feature: Decoded GeoJSON feature.
        """
    def decode_header(self, bytes: str) -> None:
        """
        Decode Protocol Buffer (PBF) header.

        Args:
            bytes (str): Input PBF bytes.

        Returns:
            dict: Decoded header information.
        """
    def decode_non_features(self, arg0: str) -> geojson.value:
        """
        Decode non-feature elements from Protocol Buffer (PBF) bytes.

        Args:
            bytes (str): Input PBF bytes.

        Returns:
            dict: Decoded non-feature elements.
        """
    def decode_to_geojson(self, geobuf: str) -> geojson.GeoJSON:
        """
        Decode Protocol Buffer (PBF) bytes to mapbox::geojson::geojson object.

        Args:
            geobuf (str): Input PBF bytes.

        Returns:
            mapbox::geojson::geojson: Decoded GeoJSON object.
        """
    def decode_to_rapidjson(self, geobuf: str, *, sort_keys: bool = False) -> rapidjson:
        """
        Decode Protocol Buffer (PBF) bytes to RapidjsonValue GeoJSON.

        Args:
            geobuf (str): Input PBF bytes.
            sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

        Returns:
            RapidjsonValue: Decoded GeoJSON as a RapidjsonValue object.
        """
    def dim(self) -> int:
        """
        Get the dimension of the coordinates in the decoded data.

        Returns:
            int: The dimension value (2 for 2D, 3 for 3D).
        """
    def keys(self) -> list[str]:
        """
        Get the keys of the decoded Protocol Buffer (PBF) data.

        Returns:
            list: A list of strings representing the keys in the decoded PBF data.
        """
    def offsets(self) -> list[int]:
        """
        Get the offsets of features in the Protocol Buffer (PBF) file.

        Returns:
            list: A list of integer offsets representing the starting positions of features in the PBF file.
        """
    def precision(self) -> int:
        """
        Get the precision used in the decoding process.

        Returns:
            int: The precision value.
        """

class Encoder:
    def __init__(
        self,
        *,
        max_precision: int = 1000000,
        only_xy: bool = False,
        round_z: int | None = None,
    ) -> None:
        """
        Initialize an Encoder object.

        Args:
            max_precision (int): Maximum precision for coordinate encoding. Default is 10^8.
            only_xy (bool): If True, only encode X and Y coordinates. Default is False.
            round_z (Optional[int]): Number of decimal places to round Z coordinates. Default is None.
        """
    def dim(self) -> int:
        """
        Get the dimension of the encoded coordinates (2 or 3).
        """
    def e(self) -> int:
        """
        Get the encoding factor used for coordinate precision.
        """
    @typing.overload
    def encode(self, geojson: geojson.GeoJSON) -> bytes:
        """
        Encode GeoJSON to Protocol Buffer (PBF) bytes.

        Args:
            geojson (mapbox::geojson::geojson): Input GeoJSON object.

        Returns:
            bytes: Encoded PBF bytes.
        """
    @typing.overload
    def encode(self, features: geojson.FeatureCollection) -> bytes:
        """
        Encode GeoJSON FeatureCollection to Protocol Buffer (PBF) bytes.

        Args:
            features (mapbox::geojson::feature_collection): Input GeoJSON FeatureCollection.

        Returns:
            bytes: Encoded PBF bytes.
        """
    @typing.overload
    def encode(self, feature: geojson.Feature) -> bytes:
        """
        Encode GeoJSON Feature to Protocol Buffer (PBF) bytes.

        Args:
            feature (mapbox::geojson::feature): Input GeoJSON Feature.

        Returns:
            bytes: Encoded PBF bytes.
        """
    @typing.overload
    def encode(self, geometry: geojson.Geometry) -> bytes:
        """
        Encode GeoJSON Geometry to Protocol Buffer (PBF) bytes.

        Args:
            geometry (mapbox::geojson::geometry): Input GeoJSON Geometry.

        Returns:
            bytes: Encoded PBF bytes.
        """
    @typing.overload
    def encode(self, geojson: rapidjson) -> bytes:
        """
        Encode RapidjsonValue GeoJSON to Protocol Buffer (PBF) bytes.

        Args:
            geojson (RapidjsonValue): Input RapidjsonValue GeoJSON object.

        Returns:
            bytes: Encoded PBF bytes.
        """
    @typing.overload
    def encode(self, geojson: typing.Any) -> bytes:
        """
        Encode Python object GeoJSON to Protocol Buffer (PBF) bytes.

        Args:
            geojson (object): Input Python object representing GeoJSON.

        Returns:
            bytes: Encoded PBF bytes.
        """
    @typing.overload
    def encode(self, *, geojson: str, geobuf: str) -> bool:
        """
        Encode GeoJSON file to Protocol Buffer (PBF) file.

        Args:
            geojson (str): Path to input GeoJSON file.
            geobuf (str): Path to output PBF file.

        Returns:
            Bool: succ or not.
        """
    def keys(self) -> dict[str, int]:
        """
        Get keys used in the encoded data.
        """
    def max_precision(self) -> int:
        """
        Get the maximum precision used for coordinate encoding.
        """
    def only_xy(self) -> bool:
        """
        Check if only X and Y coordinates are being encoded.
        """
    def round_z(self) -> float | None:
        """
        Get the number of decimal places used for rounding Z coordinates.
        """

class GeobufIndex:
    @staticmethod
    def indexing(
        input_geobuf_path: str,
        output_index_path: str,
        *,
        feature_id: str | None = "@",
        packed_rtree: str | None = "@",
    ) -> bool:
        """
        Create an index for a Geobuf file.

        Args:
            input_geobuf_path (str): Path to the input Geobuf file.
            output_index_path (str): Path to save the output index file.
            feature_id (str, optional): Feature ID field. Defaults to "@".
            packed_rtree (str, optional): Packed R-tree option. Defaults to "@".

        Returns:
            None
        """
    def __init__(self) -> None:
        """
        Default constructor for GeobufIndex.
        """
    @typing.overload
    def decode_feature(
        self, index: int, *, only_geometry: bool = False, only_properties: bool = False
    ) -> geojson.Feature | None:
        """
        Decode a feature from the Geobuf file.

        Args:
            index (int): Index of the feature to decode.
            only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
            only_properties (bool, optional): Whether to decode only properties. Defaults to False.

        Returns:
            mapbox::geojson::feature: Decoded feature.
        """
    @typing.overload
    def decode_feature(
        self, bytes: str, *, only_geometry: bool = False, only_properties: bool = False
    ) -> geojson.Feature | None:
        """
        Decode a feature from bytes.

        Args:
            bytes (str): Bytes containing the feature data.
            only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
            only_properties (bool, optional): Whether to decode only properties. Defaults to False.

        Returns:
            mapbox::geojson::feature: Decoded feature.
        """
    def decode_feature_of_id(
        self, id: str, *, only_geometry: bool = False, only_properties: bool = False
    ) -> geojson.Feature | None:
        """
        Decode a feature by its ID.

        Args:
            id (str): ID of the feature to decode.
            only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
            only_properties (bool, optional): Whether to decode only properties. Defaults to False.

        Returns:
            mapbox::geojson::feature: Decoded feature.
        """
    def decode_features(
        self,
        index: list[int],
        *,
        only_geometry: bool = False,
        only_properties: bool = False,
    ) -> geojson.FeatureCollection:
        """
        Decode multiple features from the Geobuf file.

        Args:
            index (List[int]): List of indices of the features to decode.
            only_geometry (bool, optional): Whether to decode only geometry. Defaults to False.
            only_properties (bool, optional): Whether to decode only properties. Defaults to False.

        Returns:
            List[mapbox::geojson::feature]: List of decoded features.
        """
    @typing.overload
    def decode_non_features(self, bytes: str) -> geojson.value:
        """
        Decode non-feature data from bytes.

        Args:
            bytes (str): Bytes containing the non-feature data.

        Returns:
            Dict: Decoded non-feature data.
        """
    @typing.overload
    def decode_non_features(self) -> geojson.value:
        """
        Decode non-feature data from the Geobuf file.

        Returns:
            Dict: Decoded non-feature data.
        """
    def init(self, index_bytes: str) -> bool:
        """
        Initialize the GeobufIndex from index bytes.

        Args:
            index_bytes (str): Bytes containing the index information.

        Returns:
            None
        """
    def mmap_bytes(self, offset: int, length: int) -> bytes | None:
        """
        Read bytes from the memory-mapped file.

        Args:
            offset (int): Offset in the file.
            length (int): Number of bytes to read.

        Returns:
            Optional[bytes]: Read bytes, or None if reading failed.
        """
    @typing.overload
    def mmap_init(self, index_path: str, geobuf_path: str) -> bool:
        """
        Initialize the GeobufIndex using memory-mapped files.

        Args:
            index_path (str): Path to the index file.
            geobuf_path (str): Path to the Geobuf file.

        Returns:
            None
        """
    @typing.overload
    def mmap_init(self, geobuf_path: str) -> bool:
        """
        Initialize the GeobufIndex using a memory-mapped Geobuf file.

        Args:
            geobuf_path (str): Path to the Geobuf file.

        Returns:
            None
        """
    def query(
        self,
        arg0: numpy.ndarray[numpy.float64[2, 1]],
        arg1: numpy.ndarray[numpy.float64[2, 1]],
    ) -> set[int]:
        """
        Query features within a bounding box.

        Args:
            min_corner (Eigen::Vector2d): Minimum corner of the bounding box.
            max_corner (Eigen::Vector2d): Maximum corner of the bounding box.

        Returns:
            List[int]: List of indices of features within the bounding box.
        """
    @property
    def header_size(self) -> int:
        """
        Get the size of the header in bytes.

        Returns:
            int: The size of the header in bytes.
        """
    @property
    def ids(self) -> dict[str, int] | None:
        """
        Get the IDs of features in the index.

        Returns:
            list: A list of feature IDs.
        """
    @property
    def num_features(self) -> int:
        """
        Get the number of features in the index.

        Returns:
            int: The number of features.
        """
    @property
    def offsets(self) -> list[int]:
        """
        Get the offsets of features in the Geobuf file.

        Returns:
            list: A list of offsets for each feature.
        """
    @property
    def packed_rtree(self) -> PackedRTree:
        """
        Get the packed R-tree of the index.

        Returns:
            FlatGeobuf.PackedRTree: The packed R-tree of the index, or None if not available.
        """

class NodeItem:
    __hash__: typing.ClassVar[None] = None
    def __eq__(self, arg0: NodeItem) -> bool:
        """
        Check if two nodes are equal
        """
    def __ne__(self, arg0: NodeItem) -> bool:
        """
        Check if two nodes are not equal
        """
    def expand(self, other: NodeItem) -> NodeItem:
        """
        Expand the node's bounding box to include another node
        """
    def intersects(self, other: NodeItem) -> bool:
        """
        Check if this node's bounding box intersects with another node's bounding box
        """
    def to_numpy(self) -> numpy.ndarray[numpy.float64[4, 1]]:
        """
        Convert the node's bounding box to a numpy array [minX, minY, maxX, maxY]
        """
    @property
    def height(self) -> float:
        """
        Get the height of the node's bounding box
        """
    @property
    def max_x(self) -> float:
        """
        Get the maximum X coordinate of the node
        """
    @property
    def max_y(self) -> float:
        """
        Get the maximum Y coordinate of the node
        """
    @property
    def min_x(self) -> float:
        """
        Get the minimum X coordinate of the node
        """
    @property
    def min_y(self) -> float:
        """
        Get the minimum Y coordinate of the node
        """
    @property
    def offset(self) -> int:
        """
        Get the offset of the node
        """
    @property
    def width(self) -> float:
        """
        Get the width of the node's bounding box
        """

class PackedRTree:
    def search(
        self, min_x: float, min_y: float, max_x: float, max_y: float
    ) -> list[int]:
        """
        Search for items within the given bounding box.

        Args:
            min_x (float): Minimum X coordinate of the bounding box.
            min_y (float): Minimum Y coordinate of the bounding box.
            max_x (float): Maximum X coordinate of the bounding box.
            max_y (float): Maximum Y coordinate of the bounding box.

        Returns:
            list: List of offsets of items within the bounding box.
        """
    @property
    def extent(self) -> numpy.ndarray[numpy.float64[4, 1]]: ...
    @property
    def node_size(self) -> int: ...
    @property
    def num_items(self) -> int: ...
    @property
    def num_nodes(self) -> int: ...
    @property
    def size(self) -> int: ...

class Planet:
    @typing.overload
    def __init__(self) -> None:
        """
        Initialize an empty Planet object.
        """
    @typing.overload
    def __init__(self, arg0: geojson.FeatureCollection) -> None:
        """
        Initialize a Planet object with a feature collection.

        Args:
            feature_collection (mapbox::geojson::feature_collection): The feature collection to initialize with.
        """
    def build(self, *, per_line_segment: bool = False, force: bool = False) -> None:
        """
        Build the spatial index for the features.

        Args:
            per_line_segment (bool, optional): Whether to index each line segment separately. Defaults to False.
            force (bool, optional): Whether to force rebuilding the index. Defaults to False.

        Returns:
            None
        """
    def copy(self, arg0: numpy.ndarray[numpy.int32[m, 1]]) -> geojson.FeatureCollection:
        """
        Create a deep copy of the Planet object.

        Returns:
            Planet: A new Planet object that is a deep copy of the current one.
        """
    def crop(
        self,
        polygon: numpy.ndarray[numpy.float64[m, 2], numpy.ndarray.flags.c_contiguous],
        *,
        clipping_mode: str = "longest",
        strip_properties: bool = False,
        is_wgs84: bool = True,
    ) -> geojson.FeatureCollection:
        """
        Crop features using a polygon.

        Args:
            polygon (mapbox::geojson::polygon): Polygon to crop with.
            clipping_mode (str, optional): Clipping mode. Defaults to "longest".
            strip_properties (bool, optional): Whether to strip properties from cropped features. Defaults to False.
            is_wgs84 (bool, optional): Whether the coordinates are in WGS84. Defaults to True.

        Returns:
            Planet: New Planet object with cropped features.
        """
    @typing.overload
    def features(self) -> geojson.FeatureCollection:
        """
        Get the features of the Planet object.

        Returns:
            mapbox::geojson::feature_collection: The features of the Planet object.
        """
    @typing.overload
    def features(self, arg0: geojson.FeatureCollection) -> Planet:
        """
        Set the features of the Planet object.

        Args:
            feature_collection (mapbox::geojson::feature_collection): The new feature collection to set.
        """
    def packed_rtree(self) -> PackedRTree:
        """
        Get the packed R-tree of the Planet object.

        Returns:
            FlatGeobuf::PackedRTree: The packed R-tree of the Planet object.
        """
    def query(
        self,
        min: numpy.ndarray[numpy.float64[2, 1]],
        max: numpy.ndarray[numpy.float64[2, 1]],
    ) -> numpy.ndarray[numpy.int32[m, 1]]:
        """
        Query features within the given bounding box.

        Args:
            min (array-like): Minimum coordinates of the bounding box.
            max (array-like): Maximum coordinates of the bounding box.

        Returns:
            list: List of features within the bounding box.
        """

class rapidjson:
    class type:
        """
        Members:

          kNullType : Null type

          kFalseType : False type

          kTrueType : True type

          kObjectType : Object type

          kArrayType : Array type

          kStringType : String type

          kNumberType : Number type
        """

        __members__: typing.ClassVar[
            dict[str, rapidjson.type]
        ]  # value = {'kNullType': <type.kNullType: 0>, 'kFalseType': <type.kFalseType: 1>, 'kTrueType': <type.kTrueType: 2>, 'kObjectType': <type.kObjectType: 3>, 'kArrayType': <type.kArrayType: 4>, 'kStringType': <type.kStringType: 5>, 'kNumberType': <type.kNumberType: 6>}
        kArrayType: typing.ClassVar[rapidjson.type]  # value = <type.kArrayType: 4>
        kFalseType: typing.ClassVar[rapidjson.type]  # value = <type.kFalseType: 1>
        kNullType: typing.ClassVar[rapidjson.type]  # value = <type.kNullType: 0>
        kNumberType: typing.ClassVar[rapidjson.type]  # value = <type.kNumberType: 6>
        kObjectType: typing.ClassVar[rapidjson.type]  # value = <type.kObjectType: 3>
        kStringType: typing.ClassVar[rapidjson.type]  # value = <type.kStringType: 5>
        kTrueType: typing.ClassVar[rapidjson.type]  # value = <type.kTrueType: 2>
        def __eq__(self, other: typing.Any) -> bool: ...
        def __getstate__(self) -> int: ...
        def __hash__(self) -> int: ...
        def __index__(self) -> int: ...
        def __init__(self, value: int) -> None: ...
        def __int__(self) -> int: ...
        def __ne__(self, other: typing.Any) -> bool: ...
        def __repr__(self) -> str: ...
        def __setstate__(self, state: int) -> None: ...
        def __str__(self) -> str: ...
        @property
        def name(self) -> str: ...
        @property
        def value(self) -> int: ...

    __hash__: typing.ClassVar[None] = None
    kArrayType: typing.ClassVar[rapidjson.type]  # value = <type.kArrayType: 4>
    kFalseType: typing.ClassVar[rapidjson.type]  # value = <type.kFalseType: 1>
    kNullType: typing.ClassVar[rapidjson.type]  # value = <type.kNullType: 0>
    kNumberType: typing.ClassVar[rapidjson.type]  # value = <type.kNumberType: 6>
    kObjectType: typing.ClassVar[rapidjson.type]  # value = <type.kObjectType: 3>
    kStringType: typing.ClassVar[rapidjson.type]  # value = <type.kStringType: 5>
    kTrueType: typing.ClassVar[rapidjson.type]  # value = <type.kTrueType: 2>
    def Empty(self) -> bool:
        """
        Check if the value is empty
        """
    def Get(self) -> typing.Any:
        """
        Convert RapidJSON value to Python object
        """
    def GetBool(self) -> bool:
        """
        Get boolean value
        """
    def GetDouble(self) -> float:
        """
        Get double value
        """
    def GetFloat(self) -> float:
        """
        Get float value
        """
    def GetInt(self) -> int:
        """
        Get integer value
        """
    def GetInt64(self) -> int:
        """
        Get 64-bit integer value
        """
    def GetRawString(self) -> memoryview:
        """
        Get raw string as memory view
        """
    def GetString(self) -> str:
        """
        Get string value
        """
    def GetStringLength(self) -> int:
        """
        Get length of string value
        """
    def GetType(self) -> ...:
        """
        Get the type of the value
        """
    def GetUInt64(self) -> int:
        """
        Get 64-bit unsigned integer value
        """
    def GetUint(self) -> int:
        """
        Get unsigned integer value
        """
    def HasMember(self, arg0: str) -> bool:
        """
        Check if the object has a member with the given key
        """
    def IsArray(self) -> bool:
        """
        Check if the value is an array
        """
    def IsBool(self) -> bool:
        """
        Check if the value is a boolean
        """
    def IsDouble(self) -> bool:
        """
        Check if the value is a double
        """
    def IsFalse(self) -> bool:
        """
        Check if the value is false
        """
    def IsFloat(self) -> bool:
        """
        Check if the value is a float
        """
    def IsInt(self) -> bool:
        """
        Check if the value is an integer
        """
    def IsInt64(self) -> bool:
        """
        Check if the value is a 64-bit integer
        """
    def IsLosslessDouble(self) -> bool:
        """
        Check if the value can be losslessly converted to double
        """
    def IsLosslessFloat(self) -> bool:
        """
        Check if the value can be losslessly converted to float
        """
    def IsNull(self) -> bool:
        """
        Check if the value is null
        """
    def IsNumber(self) -> bool:
        """
        Check if the value is a number
        """
    def IsObject(self) -> bool:
        """
        Check if the value is an object
        """
    def IsString(self) -> bool:
        """
        Check if the value is a string
        """
    def IsTrue(self) -> bool:
        """
        Check if the value is true
        """
    def IsUint(self) -> bool:
        """
        Check if the value is an unsigned integer
        """
    def IsUint64(self) -> bool:
        """
        Check if the value is a 64-bit unsigned integer
        """
    def SetArray(self) -> rapidjson:
        """
        Set the value to an empty array
        """
    def SetDouble(self, arg0: float) -> rapidjson:
        """
        Set the value to a double
        """
    def SetFloat(self, arg0: float) -> rapidjson:
        """
        Set the value to a float
        """
    def SetInt(self, arg0: int) -> rapidjson:
        """
        Set the value to an integer
        """
    def SetInt64(self, arg0: int) -> rapidjson:
        """
        Set the value to a 64-bit integer
        """
    def SetNull(self) -> rapidjson:
        """
        Set the value to null
        """
    def SetObject(self) -> rapidjson:
        """
        Set the value to an empty object
        """
    def SetUint(self, arg0: int) -> rapidjson:
        """
        Set the value to an unsigned integer
        """
    def SetUint64(self, arg0: int) -> rapidjson:
        """
        Set the value to a 64-bit unsigned integer
        """
    def Size(self) -> int:
        """
        Get the size of the value (for arrays and objects)
        """
    def __bool__(self) -> bool:
        """
        Check if the value is truthy
        """
    def __call__(self) -> typing.Any:
        """
        Convert RapidJSON value to Python object
        """
    def __contains__(self, arg0: str) -> bool:
        """
        Check if the object has a member with the given key
        """
    def __copy__(self, arg0: dict) -> rapidjson:
        """
        Create a shallow copy of the value
        """
    def __deepcopy__(self, memo: dict) -> rapidjson:
        """
        Create a deep copy of the value
        """
    @typing.overload
    def __delitem__(self, arg0: str) -> bool:
        """
        Delete a member by key
        """
    @typing.overload
    def __delitem__(self, arg0: int) -> None:
        """
        Delete an array element by index
        """
    def __eq__(self, arg0: rapidjson) -> bool:
        """
        Compare two RapidJSON values for equality
        """
    @typing.overload
    def __getitem__(self, arg0: str) -> rapidjson:
        """
        Get a member value by key
        """
    @typing.overload
    def __getitem__(self, arg0: int) -> rapidjson:
        """
        Get an array element by index
        """
    def __getstate__(self) -> typing.Any: ...
    @typing.overload
    def __init__(self) -> None:
        """
        Initialize an empty RapidJSON value
        """
    @typing.overload
    def __init__(self, arg0: typing.Any) -> None:
        """
        Initialize a RapidJSON value from a Python object
        """
    def __len__(self) -> int:
        """
        Get the size of the value (for arrays and objects)
        """
    def __ne__(self, arg0: rapidjson) -> bool:
        """
        Compare two RapidJSON values for inequality
        """
    @typing.overload
    def __setitem__(self, index: int, value: typing.Any) -> typing.Any:
        """
        Set array element by index
        """
    @typing.overload
    def __setitem__(self, arg0: str, arg1: typing.Any) -> typing.Any:
        """
        Set object member by key
        """
    def __setstate__(self, arg0: typing.Any) -> None:
        """
        Enable pickling support
        """
    def clear(self) -> rapidjson:
        """
        Clear all members of an object or array
        """
    def clone(self) -> rapidjson:
        """
        Create a deep copy of the value
        """
    def copy_from(self, arg0: rapidjson) -> rapidjson:
        """
        Copy value from another RapidJSON value
        """
    def denoise_double_0(self) -> rapidjson:
        """
        Denoise double values that are close to zero
        """
    def dump(self, path: str, *, indent: bool = False, sort_keys: bool = False) -> bool:
        """
        Dump JSON to a file
        """
    def dumps(self, *, indent: bool = False, sort_keys: bool = False) -> str:
        """
        Dump JSON to a string
        """
    def get(self, key: str) -> rapidjson:
        """
        Get a member value by key, returns None if not found
        """
    def is_subset_of(self, other: rapidjson) -> bool:
        """
        Check if this value is a subset of another value
        """
    def keys(self) -> list[str]:
        """
        Get a list of keys for an object
        """
    def load(self, arg0: str) -> rapidjson:
        """
        Load JSON from a file
        """
    def loads(self, arg0: str) -> rapidjson:
        """
        Load JSON from a string
        """
    def locate_nan_inf(self) -> str | None:
        """
        Locate NaN or Inf values in the JSON
        """
    def normalize(
        self,
        *,
        sort_keys: bool = True,
        strip_geometry_z_0: bool = True,
        round_geojson_non_geometry: int | None = 3,
        round_geojson_geometry: typing.Annotated[
            list[int], pybind11_stubgen.typing_ext.FixedSize(3)
        ]
        | None = [8, 8, 3],
        denoise_double_0: bool = True,
    ) -> rapidjson:
        """
        Normalize JSON by applying multiple transformations
        """
    def pop_back(self) -> rapidjson:
        """
        Remove and return the last element of the array
        """
    def push_back(self, arg0: typing.Any) -> rapidjson:
        """
        Append value to array
        """
    def round(
        self, *, precision: float = 3, depth: int = 32, skip_keys: list[str] = []
    ) -> rapidjson:
        """
        Round numeric values in the JSON
        """
    def round_geojson_geometry(
        self,
        *,
        precision: typing.Annotated[
            list[int], pybind11_stubgen.typing_ext.FixedSize(3)
        ] = [8, 8, 3],
    ) -> rapidjson:
        """
        Round geometry coordinates in GeoJSON
        """
    def round_geojson_non_geometry(self, *, precision: int = 3) -> rapidjson:
        """
        Round non-geometry numeric values in GeoJSON
        """
    @typing.overload
    def set(self, arg0: typing.Any) -> rapidjson:
        """
        Set value from Python object
        """
    @typing.overload
    def set(self, arg0: rapidjson) -> rapidjson:
        """
        Set value from another RapidJSON value
        """
    def sort_keys(self) -> rapidjson:
        """
        Sort keys of objects recursively
        """
    def strip_geometry_z_0(self) -> rapidjson:
        """
        Strip zero Z values from GeoJSON geometries
        """
    def values(self) -> list[rapidjson]:
        """
        Get a list of values for an object
        """

def is_subset_of(path1: str, path2: str) -> bool:
    """
    Check if the JSON at path1 is a subset of the JSON at path2.

    Args:
        path1 (str): Path to the first JSON file.
        path2 (str): Path to the second JSON file.

    Returns:
        bool: True if the first JSON is a subset of the second, False otherwise.
    """

@typing.overload
def normalize_json(
    input_path: str,
    output_path: str,
    *,
    indent: bool = True,
    sort_keys: bool = True,
    denoise_double_0: bool = True,
    strip_geometry_z_0: bool = True,
    round_non_geojson: int | None = 3,
    round_geojson_non_geometry: int | None = 3,
    round_geojson_geometry: typing.Annotated[
        list[int], pybind11_stubgen.typing_ext.FixedSize(3)
    ]
    | None = [8, 8, 3],
) -> bool:
    """
    Normalize JSON file.

    Args:
        input_path (str): Path to input JSON file.
        output_path (str): Path to output normalized JSON file.
        indent (bool, optional): Whether to indent the output JSON. Defaults to True.
        sort_keys (bool, optional): Whether to sort object keys. Defaults to True.
        denoise_double_0 (bool, optional): Whether to remove trailing zeros from doubles. Defaults to True.
        strip_geometry_z_0 (bool, optional): Whether to strip Z coordinate if it's 0. Defaults to True.
        round_non_geojson (int, optional): Number of decimal places to round non-GeoJSON numbers. Defaults to 3.
        round_geojson_non_geometry (int, optional): Number of decimal places to round GeoJSON non-geometry numbers. Defaults to 3.
        round_geojson_geometry (array of 3 ints, optional): Number of decimal places to round GeoJSON geometry coordinates. Defaults to [8, 8, 3].

    Returns:
        None
    """

@typing.overload
def normalize_json(
    json: rapidjson,
    *,
    sort_keys: bool = True,
    denoise_double_0: bool = True,
    strip_geometry_z_0: bool = True,
    round_non_geojson: int | None = 3,
    round_geojson_non_geometry: int | None = 3,
    round_geojson_geometry: typing.Annotated[
        list[int], pybind11_stubgen.typing_ext.FixedSize(3)
    ]
    | None = [8, 8, 3],
) -> rapidjson:
    """
    Normalize JSON object in-place.

    Args:
        json (RapidjsonValue): JSON object to normalize.
        sort_keys (bool, optional): Whether to sort object keys. Defaults to True.
        denoise_double_0 (bool, optional): Whether to remove trailing zeros from doubles. Defaults to True.
        strip_geometry_z_0 (bool, optional): Whether to strip Z coordinate if it's 0. Defaults to True.
        round_non_geojson (int, optional): Number of decimal places to round non-GeoJSON numbers. Defaults to 3.
        round_geojson_non_geometry (int, optional): Number of decimal places to round GeoJSON non-geometry numbers. Defaults to 3.
        round_geojson_geometry (array of 3 ints, optional): Number of decimal places to round GeoJSON geometry coordinates. Defaults to [8, 8, 3].

    Returns:
        RapidjsonValue: Reference to the normalized JSON object.
    """

def pbf_decode(pbf_bytes: str, *, indent: str = "") -> str:
    """
    Decode Protocol Buffer (PBF) bytes to a printable string.

    Args:
        pbf_bytes (str): Input PBF bytes.
        indent (str, optional): Indentation string. Defaults to "".

    Returns:
        str: Decoded and formatted PBF content as a string.
    """

def str2geojson2str(
    json_string: str, *, indent: bool = False, sort_keys: bool = False
) -> str | None:
    """
    Convert JSON string to GeoJSON object and back to JSON string.

    Args:
        json_string (str): Input JSON string.
        indent (bool, optional): Whether to indent the output JSON. Defaults to False.
        sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

    Returns:
        Optional[str]: Converted GeoJSON string, or None if input is invalid.
    """

def str2json2str(
    json_string: str, *, indent: bool = False, sort_keys: bool = False
) -> str | None:
    """
    Convert JSON string to JSON object and back to string.

    Args:
        json_string (str): Input JSON string.
        indent (bool, optional): Whether to indent the output JSON. Defaults to False.
        sort_keys (bool, optional): Whether to sort object keys. Defaults to False.

    Returns:
        Optional[str]: Converted JSON string, or None if input is invalid.
    """

__version__: str = "0.2.2"
