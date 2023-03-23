import base64
import json
import os
import pickle
import sys
from copy import deepcopy

import numpy as np
import pytest

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
    assert id(obj["type"]) == id(obj["type"])
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

    # https://github.com/pybind/pybind11_json/blob/b02a2ad597d224c3faee1f05a56d81d4c4453092/include/pybind11_json/pybind11_json.hpp#L110
    assert rapidjson(b"raw bytes")() == base64.b64encode(b"raw bytes").decode(
        "utf-8"
    )  # noqa
    assert rapidjson(b"raw bytes")() == "cmF3IGJ5dGVz"  # base64 encoded

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


def test_rapidjson_sort_dump():
    obj1 = rapidjson(
        {
            "key1": 42,
            "key2": 3.14,
        }
    )
    assert list(obj1.keys()) == ["key1", "key2"]
    assert obj1.dumps() == '{"key1":42,"key2":3.14}'
    assert list(obj1.sort_keys().keys()) == ["key1", "key2"]
    obj2 = rapidjson(
        {
            "key2": 3.14,
            "key1": 42,
        }
    )
    assert list(obj2.keys()) == ["key2", "key1"]
    assert obj2.dumps() == '{"key2":3.14,"key1":42}'
    assert obj2.dumps(sort_keys=True) == '{"key1":42,"key2":3.14}'
    assert (
        obj1.dumps(sort_keys=True, indent=True)
        == '{\n    "key1": 42,\n    "key2": 3.14\n}'
    )
    assert list(obj2.keys()) == ["key2", "key1"]  # won't modify obj
    assert list(obj2.sort_keys().keys()) == ["key1", "key2"]
    obj = rapidjson([obj1, obj2, {"obj2": obj2, "obj1": obj1}])
    obj[0]["another"] = 5
    assert obj1.dumps() == '{"key1":42,"key2":3.14}'
    assert (
        obj.dumps()
        == '[{"key1":42,"key2":3.14,"another":5},{"key1":42,"key2":3.14},{"obj2":{"key1":42,"key2":3.14},"obj1":{"key1":42,"key2":3.14}}]'  # noqa
    )
    obj.sort_keys()
    assert (
        obj.dumps()
        == '[{"another":5,"key1":42,"key2":3.14},{"key1":42,"key2":3.14},{"obj1":{"key1":42,"key2":3.14},"obj2":{"key1":42,"key2":3.14}}]'  # noqa
    )

    obj3 = obj
    assert id(obj3) == id(obj)  # python assign
    obj4 = obj.clone()
    obj5 = rapidjson()
    obj5.set(obj)
    assert id(obj4) != id(obj)
    assert id(obj5) != id(obj)
    assert obj4 == obj5
    assert obj4.dumps() == obj5.dumps()
    obj4.push_back(42)
    assert obj4 != obj5
    obj5.push_back(42)
    assert obj4 == obj5

    obj6 = rapidjson().copy_from(obj5)
    assert id(obj6) != id(obj5)
    assert obj6 == obj5


def test_geojson_point():
    # as_numpy
    g1 = geojson.Point()
    assert np.all(g1.as_numpy() == [0, 0, 0])
    g2 = geojson.Point(1, 2)
    assert np.all(g2.as_numpy() == [1, 2, 0])
    g3 = geojson.Point(1, 2, 3)
    assert np.all(g3.as_numpy() == [1, 2, 3])
    assert list(g3) == [1, 2, 3]
    for x in g3:
        x += 10  # more like value semantic (python can't provide you double&)
    assert list(g3) == [1, 2, 3]
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

    assert g3() == [3, 7, 2]

    # from/to_rapidjson
    j = g3.to_rapidjson()()
    assert j == {"type": "Point", "coordinates": [3.0, 7.0, 2.0]}
    # update
    g3[0] = 0.0
    assert g3.to_rapidjson()() != {
        "type": "Point",
        "coordinates": [3.0, 7.0, 2.0],
    }
    # reset
    g3.from_rapidjson(rapidjson(j))
    assert g3.to_rapidjson()() == {
        "type": "Point",
        "coordinates": [3.0, 7.0, 2.0],
    }


def test_geojson_point2():
    pt = geojson.Point()
    assert pt() == [0.0, 0.0, 0.0]
    pt = geojson.Point([1, 2])
    assert pt() == [1.0, 2.0, 0.0]
    pt = geojson.Point([1, 2, 3])
    assert pt() == [1.0, 2.0, 3.0]
    pt.from_numpy([4, 5, 6])
    assert pt() == [4.0, 5.0, 6.0]
    pt.from_numpy([7, 8])
    assert pt() == [7.0, 8.0, 0.0]
    assert pt.x == 7.0
    pt.x = 6.0
    assert pt.x == 6.0
    assert pt.y == 8.0 and pt.z == 0.0
    assert pt.x == pt[0] == pt[-3]
    assert pt.y == pt[1] == pt[-2]
    assert pt.z == pt[2] == pt[-1]
    pt[2] += 1.0
    assert pt.z == 1.0
    assert pt.to_rapidjson()() == {
        "type": "Point",
        "coordinates": [6.0, 8.0, 1.0],
    }
    pt.from_rapidjson(
        rapidjson(
            {
                "type": "Point",
                "coordinates": [2.0, 4.0, 1.0],
            }
        )
    )
    assert pt.as_numpy().tolist() == [2, 4, 1]
    pt.from_rapidjson(
        rapidjson({"type": "Point", "coordinates": [3.0, 5.0, 2.0]})
    ).x = 33
    assert pt.as_numpy().tolist() == [33, 5, 2]

    pt.clear()
    assert pt() == [0, 0, 0]
    assert pt.clear() == pt


def test_geojson_multi_point():
    g1 = geojson.MultiPoint()
    assert g1.as_numpy().shape == (0, 3)
    g1 = geojson.MultiPoint([[1, 2, 3], [4, 5, 6]])
    assert g1.as_numpy().shape == (2, 3)
    assert len(g1) == 2
    assert np.all(g1.as_numpy() == [[1, 2, 3], [4, 5, 6]])
    assert g1() == [[1, 2, 3], [4, 5, 6]]

    # g2 = geojson.MultiPoint([g1[0], g1[1]])
    g3 = geojson.MultiPoint([[1, 2], [3, 4]])
    assert np.all(g3.as_numpy() == [[1, 2, 0], [3, 4, 0]])

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
    g1.push_back(geojson.Point())
    assert len(g1) == 3  # push_back works now
    g1.pop_back()
    assert len(g1) == 2

    j = g1.to_rapidjson()
    gg = geojson.MultiPoint().from_rapidjson(j)
    assert g1 == gg
    assert gg() == [[1, 2, 0], [7, 8, 9]]
    assert j() == gg.to_rapidjson()()
    # rapidjson is comparable
    assert j == gg.to_rapidjson()
    j["another_key"] = "value"
    assert j != gg.to_rapidjson()

    xyz = np.zeros(3)
    for x in g1:  # iterable
        xyz += x.as_numpy()
    assert np.all(xyz == np.sum(g1.as_numpy(), axis=0))
    assert np.all(xyz == g1[0].as_numpy() + g1[-1].as_numpy())

    assert len(g1) == 2
    g1.clear()
    assert len(g1) == 0
    assert g1.clear() == g1


def test_geojson_line_string():
    g1 = geojson.LineString()
    assert g1.as_numpy().shape == (0, 3)
    g1 = geojson.LineString([[1, 2, 3], [4, 5, 6]])
    assert g1.as_numpy().shape == (2, 3)
    assert len(g1) == 2
    assert np.all(g1.as_numpy() == [[1, 2, 3], [4, 5, 6]])
    assert g1() == [[1, 2, 3], [4, 5, 6]]

    j = g1.to_rapidjson()
    j["coordinates"] = [[1, 1, 1], [2, 2, 2]]
    g1.from_rapidjson(j)
    assert g1() == [[1, 1, 1], [2, 2, 2]]
    G = geojson.Geometry(g1)
    assert G.to_rapidjson() == g1.to_rapidjson()
    assert G.type() == "LineString"

    assert isinstance(g1, geojson.LineString)

    xyz = np.zeros(3)
    for x in g1:  # iterable
        xyz += x.as_numpy()
    assert np.all(xyz == np.sum(g1.as_numpy(), axis=0))
    assert np.all(xyz == g1[0].as_numpy() + g1[-1].as_numpy())

    assert len(g1) == 2
    g1.push_back([1, 2, 3])
    assert len(g1) == 3
    g1.push_back([4, 5])
    assert len(g1) == 4
    g1.clear()
    assert len(g1) == 0
    assert g1 == g1.clear()

    # TODO, fix append
    g1.append(geojson.Point(1, 2))  # don't use append for now
    assert len(g1) == 0


def test_geojson_multi_line_string():
    g1 = geojson.MultiLineString()
    assert isinstance(g1, geojson.MultiLineString)
    assert isinstance(g1, geojson.LineStringList)
    assert len(g1) == 0

    xyzs = [[1, 2, 3], [4, 5, 6]]
    g1.from_numpy(xyzs)
    assert len(g1) == 1
    assert np.all(g1.to_numpy() == xyzs)

    j = g1.to_rapidjson()
    g1.as_numpy()[:] = 1
    assert g1.as_numpy().sum() == 6
    g1.from_rapidjson(j)
    assert np.all(g1.to_numpy() == xyzs)

    coords = np.array(j["coordinates"]())
    assert coords.ndim == 3
    assert coords.shape == (1, 2, 3)
    assert np.array(g1()).shape == (1, 2, 3)

    assert len(g1) == 1
    g10 = g1[0]
    assert isinstance(g10, geojson.LineString)
    for ls in g1:
        assert isinstance(ls, geojson.LineString)
        assert g10 == ls
        assert len(ls) == 2
        for pt in ls:
            assert isinstance(pt, geojson.Point)
            assert len(pt) == 3

    g1.push_back([[1, 2], [3, 4]])
    assert len(g1) == 2
    assert g1() == [
        [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]],
        [[1.0, 2.0, 0.0], [3.0, 4.0, 0.0]],
    ]
    with pytest.raises(ValueError) as excinfo:
        g1.push_back([5, 6])
    assert "shape expected to be Nx2 or Nx3" in repr(excinfo)
    g1[-1].push_back([5, 6])
    assert g1() == [
        [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]],
        [[1.0, 2.0, 0.0], [3.0, 4.0, 0.0], [5.0, 6.0, 0.0]],
    ]
    assert g1() == [g1[0](), g1[1]()]
    g1.from_numpy([[1, 2, 3], [4, 5, 6]])
    assert g1() == [[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]]

    g1[0] = [[7, 8], [2, 3]]
    assert g1() == [[[7, 8, 0], [2, 3, 0]]]

    g1.clear()
    assert len(g1) == 0


def test_geojson_polygon():
    g1 = geojson.Polygon()
    assert isinstance(g1, geojson.Polygon)
    assert isinstance(g1, geojson.LinearRingList)
    assert len(g1) == 0
    assert g1.to_rapidjson()() == {"type": "Polygon", "coordinates": []}

    g1.from_numpy([[1, 0], [1, 1], [0, 1], [1, 0]])
    assert len(g1) == 1
    assert np.all(
        g1.to_numpy()
        == [
            [1, 0, 0],
            [1, 1, 0],
            [0, 1, 0],
            [1, 0, 0],
        ]
    )
    assert isinstance(g1[0], geojson.LinearRing)
    assert g1[0]() == [
        [1.0, 0.0, 0.0],
        [1.0, 1.0, 0.0],
        [0.0, 1.0, 0.0],
        [1.0, 0.0, 0.0],
    ]
    assert g1[0].as_numpy().shape == (4, 3)
    assert len(g1[0]) == 4
    assert len(g1[0][0]) == 3
    for pt in g1[0]:
        assert isinstance(pt, geojson.Point)
    assert isinstance(g1[0][0], geojson.Point)
    for gg in g1:
        assert isinstance(gg, geojson.LinearRing)
        for pt in gg:
            assert isinstance(pt, geojson.Point)

    g1[0].push_back([8, 9]).push_back(geojson.Point(10, 11))
    assert np.all(g1[0].as_numpy()[-2:, :] == [[8, 9, 0], [10, 11, 0]])

    g1[0].from_numpy([[1, 2], [3, 4]])
    assert g1[0]() == [[1.0, 2.0, 0.0], [3.0, 4.0, 0.0]]

    g1.clear()
    assert len(g1) == 0
    assert g1.clear() == g1


def test_geojson_multi_polygon():
    g1 = geojson.MultiPolygon()
    assert isinstance(g1, geojson.MultiPolygon)
    assert len(g1) == 0
    assert np.array(g1()).shape == (0,)
    assert g1() == []
    assert g1.to_rapidjson()() == {"type": "MultiPolygon", "coordinates": []}
    g1.from_numpy([[1, 0], [1, 1], [0, 1], [1, 0]])
    assert np.array(g1()).shape == (1, 1, 4, 3)

    assert g1.to_rapidjson()() == {
        "type": "MultiPolygon",
        "coordinates": [
            [
                [
                    [1.0, 0.0, 0.0],
                    [1.0, 1.0, 0.0],
                    [0.0, 1.0, 0.0],
                    [1.0, 0.0, 0.0],
                ]
            ]
        ],
    }
    coords = np.array(g1.to_rapidjson()["coordinates"]())
    assert coords.shape == (1, 1, 4, 3)

    g2 = geojson.MultiPolygon().from_rapidjson(g1.to_rapidjson())
    assert g1 == g2

    value = 0
    for polygon in g1:
        assert isinstance(polygon, geojson.Polygon)
        for ring in polygon:
            assert isinstance(ring, geojson.LinearRing)
            for pt in ring:
                assert isinstance(pt, geojson.Point)
                for x in pt:
                    assert isinstance(x, float)
                    value += x
    assert value == 5.0
    assert len(g1) == 1

    assert g1() == [g1[0]()]
    g1[0] = geojson.Polygon([[7, 6, 5]])
    assert g1() == [[[[7.0, 6.0, 5.0]]]]
    g1[0] = [[1, 2, 3]]
    assert g1() == [[[[1.0, 2.0, 3.0]]]]
    g1[0] = [[1, 2]]
    assert g1() == [[[[1.0, 2.0, 0.0]]]]

    with pytest.raises(ValueError) as excinfo:
        g1[0] = [3, 4]  # should be Nx2 or Nx3 (dim==2)
    assert "matrix shape expected to be Nx2 or Nx3, actual=2x1" in repr(excinfo)  # noqa
    assert g1() == [[[[1.0, 2.0, 0.0]]]]
    with pytest.raises(ValueError) as excinfo:
        g1.push_back([3, 4])
    assert "matrix shape expected to be Nx2 or Nx3, actual=2x1" in repr(excinfo)  # noqa
    assert g1() == [[[[1.0, 2.0, 0.0]]]]

    g1.push_back([[3, 4]])
    assert g1() == [
        [[[1.0, 2.0, 0.0]]],
        [[[3.0, 4.0, 0.0]]],
    ]

    g1.push_back(geojson.Polygon([[5, 6, 7]]))
    assert g1() == [
        [[[1.0, 2.0, 0.0]]],
        [[[3.0, 4.0, 0.0]]],
        [[[5.0, 6.0, 7.0]]],
    ]
    g1[-1].push_back([[1, 2, 3]]).push_back([[4, 5, 6]])
    assert g1() == [
        [[[1.0, 2.0, 0.0]]],
        [[[3.0, 4.0, 0.0]]],
        [
            [[5.0, 6.0, 7.0]],
            [[1.0, 2.0, 3.0]],
            [[4.0, 5.0, 6.0]],
        ],
    ]
    g1[-1][-1].push_back([7, 8]).push_back([9, 10, 11, 12])
    assert g1() == [
        [[[1.0, 2.0, 0.0]]],
        [[[3.0, 4.0, 0.0]]],
        [
            [[5.0, 6.0, 7.0]],
            [[1.0, 2.0, 3.0]],
            [
                [4.0, 5.0, 6.0],
                [7.0, 8.0, 0.0],
                [9.0, 10.0, 11.0],
            ],
        ],
    ]

    g1.clear()
    assert len(g1) == 0


def test_geojson_geometry():
    g1 = geojson.Geometry()
    assert g1() is None
    assert g1.type() == "None"
    g2 = geojson.Geometry(geojson.Point())
    assert g2.type() == "Point"
    assert g2() == {"type": "Point", "coordinates": [0.0, 0.0, 0.0]}
    g2["my_key"] = "my_value"
    assert g2()["my_key"] == "my_value"
    assert g2() == {
        "type": "Point",
        "coordinates": [0.0, 0.0, 0.0],
        "my_key": "my_value",
    }

    # g2['type'] = 'my_value' # TODO, should raise

    g3 = geojson.Geometry(geojson.MultiPoint([[1, 2, 3]]))
    g4 = geojson.Geometry(geojson.LineString([[1, 2, 3], [4, 5, 6]]))
    gc = geojson.Geometry(geojson.GeometryCollection())
    assert gc.type() == "GeometryCollection"
    gc.push_back(g3)
    gc.push_back(g4)
    assert gc() == {"type": gc.type(), "geometries": [g3(), g4()]}
    assert len(gc) == 2

    # update value
    g31 = g3.clone()
    g32 = g3.clone()
    assert g32 == g31
    g31.as_multi_point()[0][0] = 5
    assert g31.as_multi_point()[0][0] == 5
    assert g32 != g31

    gc2 = gc.clone()
    assert gc2() == gc()
    assert id(gc2) != id(gc)

    pickled = pickle.dumps(gc2)
    gc3 = pickle.loads(pickled)
    assert gc3() == gc()
    assert id(gc3) != id(gc)

    gc4 = deepcopy(gc3)
    assert gc4() == gc()
    assert id(gc4) != id(gc)

    assert gc4.__geo_interface__ == gc()


def test_geobuf_from_geojson():
    encoder = Encoder(max_precision=int(10**8))
    feature = sample_geojson()
    encoded_0 = encoder.encode(json.dumps(feature))
    encoded = encoder.encode(feature)
    assert encoded == encoded_0
    decoded = Decoder().decode(encoded)

    decoded_again = Decoder().decode(
        Encoder(max_precision=int(10**8)).encode(decoded)
    )
    assert decoded_again == decoded
    assert decoded_again == Decoder().decode(encoded)

    j = Decoder().decode_to_rapidjson(encoded)
    g = Decoder().decode_to_geojson(encoded)
    assert g.is_feature()
    f = g.as_feature()
    assert isinstance(f, geojson.Feature)
    with pytest.raises(RuntimeError) as excinfo:
        g.as_geometry()
    assert "in get<T>()" in str(excinfo)
    assert str2json2str(json.dumps(f()), sort_keys=True) == str2json2str(
        decoded, sort_keys=True
    )
    assert str2geojson2str(json.dumps(f()), sort_keys=True) == str2geojson2str(
        decoded, sort_keys=True
    )

    coords = np.array(
        [
            [120.40317479950272, 31.416966084052177, 1.111111],
            [120.28451900911591, 31.30578266928819, 2.22],
            [120.35592249359615, 31.21781895672254, 3.3333333333333],
            [120.67093786630113, 31.299502266522722, 4.4],
        ]
    )
    np.testing.assert_allclose(coords, f.to_numpy(), atol=1e-9)
    np.testing.assert_allclose(coords, f.as_numpy(), atol=1e-9)
    f.to_numpy()[0, 2] = 0.0
    assert 0.0 != f.as_numpy()[0, 2]
    f.as_numpy()[0, 2] = 0.0
    assert 0.0 == f.as_numpy()[0, 2]

    print(j(), j.dumps())

    expected = str2json2str(json.dumps(feature), indent=True, sort_keys=True)
    actually = str2json2str(decoded, indent=True, sort_keys=True)
    assert len(expected) > 0
    assert len(actually) > 0
    # assert expected == actually # TODO

    encoded1 = encoder.encode(rapidjson(feature))
    assert len(encoded1) == len(encoded)


def test_geojson_feature():
    return
    feature = geojson.Feature()
    props = feature.properties()
    assert not isinstance(props, dict)
    assert isinstance(props, int)


def pytest_main(dir: str, *, test_file: str = None):

    os.chdir(dir)
    sys.exit(
        pytest.main(
            [
                dir,
                *(["-k", test_file] if test_file else []),
                "--capture",
                "tee-sys",
                "-vv",
                "-x",
            ]
        )
    )


if __name__ == "__main__":
    np.set_printoptions(suppress=True)
    pwd = os.path.abspath(os.path.dirname(__file__))
    pytest_main(pwd, test_file=os.path.basename(__file__))
