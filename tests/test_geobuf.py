import base64
import json
import os
import pickle
from copy import deepcopy

import numpy as np
import pybind11_geobuf
from pybind11_geobuf import (  # noqa
    Decoder,
    Encoder,
    geojson,
    pbf_decode,
    rapidjson,
    str2geojson2str,
    str2json2str,
)


def test_version():
    print(pybind11_geobuf.__version__)


def sample_geojson():
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
    return geojson


def test_geobuf():
    geojson = sample_geojson()
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


def test_rapidjson_arr():
    arr = rapidjson([1, 3, "text", {"key": 3.2}])
    assert arr[2]() == "text"
    arr[2] = 789
    assert arr[2]() == 789
    arr[2] = arr()
    arr[0].set({"key": "value"})
    assert (
        arr.dumps() == '[{"key":"value"},3,[1,3,789,{"key":3.2}],{"key":3.2}]'
    )  # noqa

    obj = rapidjson({"arr": arr})
    assert (
        obj.dumps()
        == '{"arr":[{"key":"value"},3,[1,3,789,{"key":3.2}],{"key":3.2}]}'  # noqa
    )


def test_rapidjson_obj():
    geojson = sample_geojson()

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

    assert rapidjson(b"raw bytes")() == base64.b64encode(b"raw bytes").decode(
        "utf-8"
    )  # noqa
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


def test_geojson_point():
    # as_numpy
    g1 = geojson.Point()
    assert np.all(g1.as_numpy() == [0, 0, 0])
    g2 = geojson.Point(1, 2)
    assert np.all(g2.as_numpy() == [1, 2, 0])
    g3 = geojson.Point(1, 2, 3)
    assert np.all(g3.as_numpy() == [1, 2, 3])

    g1.as_numpy()[:] = 5
    assert np.all(g1.as_numpy() == [5, 5, 5])
    g2.as_numpy()[::2] = 5
    assert np.all(g2.as_numpy() == [5, 2, 5])
    g3.as_numpy()[1:] *= 2
    assert np.all(g3.as_numpy() == [1, 4, 6])

    # to_numpy
    g3.to_numpy()[1:] *= 2
    assert np.all(g3.as_numpy() == [1, 4, 6])

    # from_numpy
    g3.from_numpy([3, 7, 2])
    assert np.all(g3.as_numpy() == [3, 7, 2])


def test_geojson_multi_point():
    g1 = geojson.MultiPoint()
    assert g1.as_numpy().shape == (0, 3)
    g1 = geojson.MultiPoint([[1, 2, 3], [4, 5, 6]])
    assert g1.as_numpy().shape == (2, 3)
    assert len(g1) == 2
    assert np.all(g1.as_numpy() == [[1, 2, 3], [4, 5, 6]])
    assert g1() == [[1, 2, 3], [4, 5, 6]]

    assert g1[0]() == [1, 2, 3]
    assert g1[1]() == [4, 5, 6]
    g1[0] = [7, 8, 9]
    g1[1] = [1, 2]
    assert g1() == [[7, 8, 9], [1, 2, 0]]
    g1[1] = geojson.Point([7, 8, 9])
    g1[0] = geojson.Point([1, 2])
    assert g1() == [[1, 2, 0], [7, 8, 9]]
    for idx, pt in enumerate(g1):
        print(idx, pt)
        assert isinstance(pt, geojson.Point)
        if idx == 0:
            assert pt() == [1, 2, 0]
        if idx == 1:
            assert pt() == [7, 8, 9]
    g1.append(geojson.Point())
    assert len(g1) == 2  # append not working, for now


def test_geojson_line_string():
    g1 = geojson.LineString()
    assert g1.as_numpy().shape == (0, 3)
    g1 = geojson.LineString([[1, 2, 3], [4, 5, 6]])
    assert g1.as_numpy().shape == (2, 3)
    assert len(g1) == 2
    assert np.all(g1.as_numpy() == [[1, 2, 3], [4, 5, 6]])
    assert g1() == [[1, 2, 3], [4, 5, 6]]


def test_geojson_multi_line_string():
    g1 = geojson.MultiLineString()
    assert isinstance(g1, geojson.MultiLineString)
    # TODO, fix
    # assert len(g1) == 0


def test_geojson_polygon():
    g1 = geojson.Polygon()
    assert isinstance(g1, geojson.Polygon)
    # assert len(g1) == 0


def test_geojson_multi_polygon():
    g1 = geojson.MultiPolygon()
    assert isinstance(g1, geojson.MultiPolygon)
    # assert len(g1) == 0


def test_geojson_geometry():
    g1 = geojson.Geometry()
    assert g1() is None
    assert g1.type() == "None"
    g2 = geojson.Geometry(geojson.Point())
    assert g2.type() == "Point"
    assert g2() == {"type": "Point", "coordinates": [0.0, 0.0, 0.0]}
    g2["my_key"] = "my_value"
    assert g2()["my_key"] == "my_value"

    # g2['type'] = 'my_value' # TODO, should raise

    g3 = geojson.Geometry(geojson.MultiPoint([[1, 2, 3]]))
    g4 = geojson.Geometry(geojson.LineString([[1, 2, 3], [4, 5, 6]]))
    gc = geojson.Geometry(geojson.GeometryCollection())
    assert gc.type() == "GeometryCollection"
    gc.push_back(g3)
    gc.push_back(g4)
    assert gc() == {"type": gc.type(), "geometries": [g3(), g4()]}
