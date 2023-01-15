import base64
import json
import os
import pickle
from copy import deepcopy

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
arr[0].set({"key": "value"})
assert arr.dumps() == '[{"key":"value"},3,[1,3,789,{"key":3.2}],{"key":3.2}]'

obj = rapidjson({"arr": arr})
assert obj.dumps() == '{"arr":[{"key":"value"},3,[1,3,789,{"key":3.2}],{"key":3.2}]}'

obj = rapidjson(geojson)
assert obj["type"]
assert obj["type"]() == "Feature"
try:
    assert obj["missing_key"]
except KeyError as e:
    assert "missing_key" in repr(e)
assert obj.get("missing_key") is None
assert obj.keys()
assert obj.values()

assert obj.dumps()
assert obj.dumps(indent=True)

mem = obj["type"].GetRawString()
assert bytes(mem).decode("utf-8") == "Feature"

obj2 = obj.clone()
obj3 = deepcopy(obj)
assert obj() == obj2() == obj3()

pickled = pickle.dumps(obj)
obj4 = pickle.loads(pickled)
assert obj() == obj4()

obj.loads("{}")
assert obj() == {}

assert not rapidjson(2**63 - 1).IsLosslessDouble()

obj.SetNull()
assert obj.GetType().name == "kNullType"
assert obj() is None

assert rapidjson(b"raw bytes")() == base64.b64encode(b"raw bytes").decode("utf-8")
assert rapidjson(b"raw bytes")() == "cmF3IGJ5dGVz"

__pwd = os.path.abspath(os.path.dirname(__file__))
basename = "rapidjson.png"
path = f"{__pwd}/../data/{basename}"
with open(path, "rb") as f:
    raw_bytes = f.read()
assert len(raw_bytes) == 5259
data = {basename: raw_bytes}

path = f"{__pwd}/{basename}.json"
if os.path.isfile(path):
    os.remove(path)
obj = rapidjson(data)
obj.dump(path, indent=True)
assert os.path.isfile(path)

loaded = rapidjson().load(path)
png = loaded["rapidjson.png"].GetRawString()
assert len(base64.b64decode(png)) == 5259
