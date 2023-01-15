import json

import pybind11_geobuf
from pybind11_geobuf import (  # noqa
    Decoder,
    Encoder,
    pbf_decode,
    rapidjson,
    str2geojson2str,
    str2json2str,
)

print(pybind11_geobuf.__version__)

geojson = {
    "type": "Feature",
    "properties": {
        "string": "string",
        "int": 42,
        "double": 3.141592653,
        "list": ["a", "list", "is", "a", "list"],
    },
    "geometry": {
        "coordinates": [
            [120.40317479950272, 31.416966084052177, 1.111111],
            [120.28451900911591, 31.30578266928819, 2.22],
            [120.35592249359615, 31.21781895672254, 3.3333333333333],
            [120.67093786630113, 31.299502266522722, 4.4],
        ],
        "type": "LineString",
        "extra_key": "extra_value",
    },
    "my_key": "my_value",
}

print("input:")
print(json.dumps(geojson, indent=4))
print("str2json2str:")
print(str2json2str(json.dumps(geojson), indent=True, sort_keys=True))
print("str2geojson2str:")
print(str2geojson2str(json.dumps(geojson), indent=True, sort_keys=True))

# precision: 6(default), 7, 8(recommand), 9
encoder = Encoder(max_precision=int(10**8))
encoded = encoder.encode(geojson=json.dumps(geojson))
print("encoded pbf bytes")
print(pbf_decode(encoded))

decoder = Decoder()
geojson_text = decoder.decode(encoded, indent=True)
print(geojson_text)

arr = rapidjson([1, 3, "text", {"key": 3.2}])
assert arr[2]() == "text"
arr[2] = 789
assert arr[2]() == 789
arr[2] = arr()

obj = rapidjson(geojson)
assert obj["type"]
assert obj["type"]() == "Feature"
try:
    assert obj["missing_key"]
except KeyError as e:
    assert "missing_key" in repr(e)
obj.get("missing_key") is None

print()
