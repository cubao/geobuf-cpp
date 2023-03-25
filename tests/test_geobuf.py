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
            # "int2": -101,
            "double": 3.141592653,
            "list": ["a", "list", "is", "a", "list"],
            # "dict": {"key": 42, "value": 3.14},
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


def test_rapidjson_empty():
    assert not bool(rapidjson())
    assert not bool(rapidjson([]))
    assert not bool(rapidjson({}))
    assert not bool(rapidjson(0))
    assert not bool(rapidjson(0.0))
    assert not bool(rapidjson(""))
    assert not bool(rapidjson(False))

    assert bool(rapidjson([4]))
    assert bool(rapidjson({"key": "value"}))
    assert bool(rapidjson(-1))
    assert bool(rapidjson(-1.0))
    assert bool(rapidjson("text"))
    assert bool(rapidjson(True))

    assert not rapidjson([4]).Empty()
    assert not rapidjson({"key": "value"}).Empty()
    assert not rapidjson(-1).Empty()
    assert not rapidjson(-1.0).Empty()
    assert not rapidjson("text").Empty()
    assert not rapidjson(True).Empty()

    assert rapidjson([4, 2]).clear() == rapidjson([])
    assert rapidjson([4, 2]).clear() != rapidjson({})
    assert rapidjson({"key": "value"}).clear() == rapidjson({})


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
    obj = rapidjson({"bytes": b"raw bytes"})
    obj["other_bytes"] = b"bytes"
    assert obj.dumps() == '{"bytes":"cmF3IGJ5dGVz","other_bytes":"Ynl0ZXM="}'

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

    g2 = geojson.MultiPoint([g1[0], g1[1], g1[0], g1[1]])
    assert g2() == [*g1(), *g1()]
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
    assert len(g1) == 3  # append works now
    g1.push_back(geojson.Point())
    assert len(g1) == 4  # push_back works now
    g1.pop_back()
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

    g1.append(geojson.Point(1, 2))
    assert len(g1) == 1


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


def test_geojson_geometry_collection():
    gc = geojson.GeometryCollection()
    assert gc() == {"type": "GeometryCollection", "geometries": []}
    assert gc.to_rapidjson()() == {
        "type": "GeometryCollection",
        "geometries": [],
    }
    assert len(gc) == 0

    with pytest.raises(TypeError) as excinfo:
        gc.append(geojson.Point(1, 2))  # won't work
    assert "geojson.Geometry" in repr(excinfo)
    # you can use push_back, it has more overrides
    gc2 = geojson.GeometryCollection()
    gc2.push_back(geojson.Point(1, 2))
    assert gc2() == {
        "type": "GeometryCollection",
        "geometries": [{"type": "Point", "coordinates": [1.0, 2.0, 0.0]}],
    }
    gc2.push_back(geojson.Point(1, 2)).push_back(
        geojson.MultiPoint()
    ).push_back(  # noqa
        geojson.LineString()
    )  # noqa
    gc2.push_back(geojson.MultiLineString()).push_back(
        geojson.Polygon()
    ).push_back(  # noqa
        geojson.MultiPolygon()
    )  # noqa
    gc2.push_back(geojson.GeometryCollection())  # nesting
    # value semantic, push back a current copy, weird but working
    gc2.push_back(geojson.GeometryCollection(gc2))
    # or append Geometry (vector::value_type)
    gc.append(geojson.Geometry(geojson.Point(1, 2)))  # okay
    gc.append(geojson.Geometry(geojson.MultiPoint([[3, 4], [5, 6]])))
    gc.append(geojson.Geometry(geojson.LineString([[7, 8], [9, 10]])))
    assert gc() == {
        "type": "GeometryCollection",
        "geometries": [
            {
                "type": "Point",
                "coordinates": [1.0, 2.0, 0.0],
            },
            {
                "type": "MultiPoint",
                "coordinates": [
                    [3.0, 4.0, 0.0],
                    [5.0, 6.0, 0.0],
                ],
            },
            {
                "type": "LineString",
                "coordinates": [
                    [7.0, 8.0, 0.0],
                    [9.0, 10.0, 0.0],
                ],
            },
        ],
    }
    assert len(gc) == 3
    assert isinstance(gc[0], geojson.Geometry)
    assert gc[2] == gc[-1]

    del gc[0]
    assert len(gc) == 2

    gc[0] = geojson.Geometry(geojson.Point())  # works
    gc[0] = geojson.Point()  # also works
    gc[0] = geojson.MultiPoint()
    gc[0] = geojson.LineString()
    gc[0] = geojson.MultiLineString()
    gc[0] = geojson.Polygon()
    gc[0] = geojson.MultiPolygon()

    gc_init = deepcopy(gc)
    assert gc_init == gc
    del gc[-1]
    assert gc_init != gc
    gc.append(gc_init[-1])
    assert gc_init == gc

    for g in gc:
        isinstance(g, geojson.Geometry)

    gc3 = geojson.GeometryCollection(3)
    assert len(gc3) == 3
    assert gc3() == {
        "type": "GeometryCollection",
        "geometries": [
            None,
            None,
            None,
        ],
    }
    gc3[0] = geojson.Point(1, 2)
    gc3[1] = geojson.Point(3, 4)
    gc3[2] = geojson.Point(5, 6)
    gc3.resize(2)
    assert gc3() == {
        "type": "GeometryCollection",
        "geometries": [
            {"type": "Point", "coordinates": [1.0, 2.0, 0.0]},
            {"type": "Point", "coordinates": [3.0, 4.0, 0.0]},
        ],
    }
    gc3.resize(4)
    assert gc3() == {
        "type": "GeometryCollection",
        "geometries": [
            {"type": "Point", "coordinates": [1.0, 2.0, 0.0]},
            {"type": "Point", "coordinates": [3.0, 4.0, 0.0]},
            None,
            None,
        ],
    }


def test_geojson_geometry():
    g1 = geojson.Geometry()
    assert g1() is None
    assert g1.is_empty()
    assert g1.type() == "None"
    assert g1.as_numpy().shape == (0, 3)

    g2 = geojson.Geometry(geojson.Point())
    assert g2.type() == "Point"
    assert not g2.is_empty()
    assert g2.is_point()
    assert g2() == {"type": "Point", "coordinates": [0.0, 0.0, 0.0]}
    assert len(g2.custom_properties()) == 0
    g2["my_key"] = "my_value"
    assert len(g2.custom_properties()) == 1
    assert g2()["my_key"] == "my_value"
    with pytest.raises(IndexError):  # why not KeyError?
        g2["missing_key"]
    g2.get("missing_key")  # okay to be none
    assert g2() == {
        "type": "Point",
        "coordinates": [0.0, 0.0, 0.0],
        "my_key": "my_value",
    }

    g2["key"] = "wrapped in custom_properties"
    with pytest.raises(KeyError):
        g2["type"] = "type,geometry,properties are reserved"
    assert len(g2) == 3  # size of x,y,z
    assert len(g2.custom_properties()) == 2
    for k in g2:
        assert k in ["key", "my_key"]
    for v in g2.values():
        assert isinstance(v, geojson.value)
    for k, v in g2.items():
        assert k in ["key", "my_key"]
        assert isinstance(v, geojson.value)
    for x in g2.as_point():
        assert x == 0.0

    assert (
        g2.to_rapidjson().sort_keys().dumps()
        == '{"coordinates":[0.0,0.0,0.0],"key":"wrapped in custom_properties","my_key":"my_value","type":"Point"}'  # noqa
    )
    g2.custom_properties().clear()
    assert len(g2.custom_properties()) == 0
    g2.custom_properties()["key"] = "value"
    assert len(g2.custom_properties()) == 1
    assert geojson.Geometry(g2()) == geojson.Geometry(g2.to_rapidjson()) == g2

    g3 = geojson.Geometry(geojson.MultiPoint([[1, 2, 3]]))
    assert len(g3) == 1  # size of point
    g3.push_back([4, 5])
    g3.push_back(geojson.Point(6, 7))
    assert np.all(g3.as_numpy() == [[1, 2, 3], [4, 5, 0], [6, 7, 0]])
    assert len(g3) == 3
    for pt in g3.as_multi_point():
        assert isinstance(pt, geojson.Point)
        for x in pt:
            assert isinstance(x, float) and 0 <= x <= 7
    assert geojson.Geometry(g3()) == geojson.Geometry(g3.to_rapidjson()) == g3

    g4 = geojson.Geometry(geojson.LineString([[1, 2, 3], [4, 5, 6]]))
    assert g4() == {
        "type": "LineString",
        "coordinates": [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]],
    }
    g4.push_back([7, 8])
    expected = {
        "type": "LineString",
        "coordinates": [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 0.0]],
    }
    assert g4() == expected
    g4.push_back(
        [[1, 2, 3], [4, 5, 6]]
    )  # not for LineString, you can only push Nx3 to MultiLineString, Polygon
    expected = {
        "type": "LineString",
        "coordinates": [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [7.0, 8.0, 0.0]],
    }
    assert g4() == expected
    assert geojson.Geometry(g4()) == geojson.Geometry(g4.to_rapidjson()) == g4

    g5 = geojson.Geometry(geojson.MultiLineString([[1, 2, 3], [4, 5, 6]]))
    g5.push_back([[10, 20, 30], [40, 50, 60]])
    assert g5() == {
        "type": "MultiLineString",
        "coordinates": [
            [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]],
            [[10.0, 20.0, 30.0], [40.0, 50.0, 60.0]],
        ],
    }
    g5.push_back([70, 80, 90, 100])  # only x, y, z, push to last LineString
    assert g5() == {
        "type": "MultiLineString",
        "coordinates": [
            [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]],
            [[10.0, 20.0, 30.0], [40.0, 50.0, 60.0], [70.0, 80.0, 90.0]],
        ],
    }
    assert geojson.Geometry(g5()) == geojson.Geometry(g5.to_rapidjson()) == g5

    g5 = geojson.Geometry(geojson.MultiLineString())
    # g5.push_back([70, 80, 90]), don't do this, will segment fault
    # TODO, raise Exception for push_back

    g6 = geojson.Geometry(geojson.Polygon([[1, 2, 3], [4, 5, 6]]))
    assert np.array(g6()["coordinates"]).shape == (1, 2, 3)
    g6.push_back(np.ones((4, 3)))
    assert g6()["coordinates"] == [
        [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]],
        [[1.0, 1.0, 1.0], [1.0, 1.0, 1.0], [1.0, 1.0, 1.0], [1.0, 1.0, 1.0]],
    ]
    g6.push_back([2, 2])
    assert g6()["coordinates"] == [
        [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]],
        [
            [1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0],
            [1.0, 1.0, 1.0],
            [2.0, 2.0, 0.0],
        ],
    ]
    assert geojson.Geometry(g6()) == geojson.Geometry(g6.to_rapidjson()) == g6
    g6.clear()
    assert len(g6) == 0
    assert g6.as_numpy().shape == (0, 3)

    g7 = geojson.Geometry(geojson.MultiPolygon([[1, 2, 3], [4, 5, 6]]))
    assert np.array(g7()["coordinates"]).shape == (1, 1, 2, 3)
    assert geojson.Geometry(g7()) == geojson.Geometry(g7.to_rapidjson()) == g7

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
    print(encoder.keys())
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


def test_geojson_value():
    assert geojson.value().to_rapidjson().dumps() == "null"
    assert geojson.value([]).to_rapidjson().dumps() == "[]"
    assert geojson.value({}).to_rapidjson().dumps() == "{}"
    assert geojson.value("text").to_rapidjson().dumps() == '"text"'
    assert geojson.value(3.14).to_rapidjson().dumps() == "3.14"
    assert geojson.value(42).to_rapidjson().dumps() == "42"


def test_geojson_feature_id():
    feature = geojson.Feature(sample_geojson())
    assert feature.id() is None
    assert feature.id(5).id() == 5
    assert feature.id(None).id() is None
    for fid in [42, -42, 3.14, "id can be string or number"]:
        feature = geojson.Feature({**sample_geojson(), "id": fid})
        assert feature.id() == fid

    # invalid id in json
    with pytest.raises(RuntimeError) as excinfo:
        geojson.Feature({**sample_geojson(), "id": None})
    assert "Feature id must be a string or number" in repr(excinfo)


def test_geojson_feature():
    feature = sample_geojson()
    feature = geojson.Feature(feature)
    assert feature.as_numpy().shape == (4, 3)
    geom = feature.geometry()
    assert geom.type()
    orig = geom.to_numpy()
    llas = geom.as_numpy()
    feature.as_numpy()[:] = 1.0
    assert np.all(llas == 1.0)
    geom.from_numpy(orig)

    props = feature.properties()
    assert not isinstance(props, dict)
    assert isinstance(props, geojson.value.object_type)
    # assert (
    #     props.to_rapidjson().sort_keys().dumps()
    #     == '{"dict":{"key":42,"value":3.14},"double":3.141592653,"int":42,"int2":-101,"list":["a","list","is","a","list"],"string":"string"}'  # noqa
    # )

    # assert set(props.keys()) == {
    #     # "dict",
    #     "double",
    #     "int",
    #     # "int2",
    #     "list",
    #     "string",
    # }
    keys = list(props.keys())
    values = list(props.values())
    for i, (k, v) in enumerate(props.items()):
        assert keys[i] == k
        assert values[i] == v
        assert isinstance(v, geojson.value)

    assert props["list"].is_array()
    for x in list(props["list"].as_array()):
        assert isinstance(x, geojson.value)
        assert type(x) == geojson.value
    with pytest.raises(RuntimeError) as excinfo:
        props["list"].as_object()
    assert "in get<T>()" in repr(excinfo)
    assert len(props["list"]) == 5
    assert props["list"]() == ["a", "list", "is", "a", "list"]
    assert props["list"].as_array()() == ["a", "list", "is", "a", "list"]

    # assert props["dict"].is_object()
    # for k, v in props["dict"].as_object().items():
    #     assert isinstance(k, str)
    #     assert isinstance(v, geojson.value)
    #     assert type(x) == geojson.value
    # with pytest.raises(RuntimeError) as excinfo:
    #     props["dict"].as_array()
    # assert "in get<T>()" in repr(excinfo)
    # assert props["dict"]() == {"key": 42, "value": 3.14}
    # assert props["dict"].as_object()() == {"key": 42, "value": 3.14}
    # assert list(props["dict"].keys()) in [
    #     # order no guarantee (rapidjson has order, value(unordered_map) not)
    #     ["key", "value"],
    #     ["value", "key"],
    # ]

    d = props["double"]
    assert d.GetType() == "double"
    assert d.Get() == d()
    assert isinstance(d(), float)
    with pytest.raises(RuntimeError) as excinfo:
        d.get("key")
    assert "in get<T>()" in repr(excinfo)
    assert d.set([1, 2, 3])() == [1, 2, 3]

    i = props["int"]
    assert i.GetType() == "uint64_t"
    assert i.GetUint64() == 42
    assert isinstance(i.GetUint64(), int)
    with pytest.raises(RuntimeError) as excinfo:
        i.GetInt64()
    assert "in get<T>()" in repr(excinfo)

    # i = props["int2"]
    # assert i.GetType() == "int64_t"
    # assert i.GetInt64() == -101
    # assert isinstance(i.GetInt64(), int)
    # with pytest.raises(RuntimeError) as excinfo:
    #     i.GetUint64()
    # assert "in get<T>()" in repr(excinfo)

    props["new"] = 6
    assert props["new"]() == 6
    props.from_rapidjson(rapidjson({"key": 6})).to_rapidjson()
    assert props() == {"key": 6}
    assert feature.properties()() == {"key": 6}
    assert feature.properties({"key": 7}).properties()() == {"key": 7}
    assert feature.properties(rapidjson({"key": 8})).properties()() == {
        "key": 8,
    }
    assert feature.geometry(geojson.Point(1, 2)).geometry()() == {
        "type": "Point",
        "coordinates": [1.0, 2.0, 0.0],
    }
    assert feature.geometry(geojson.Point(3, 4)).geometry().as_point()() == [
        3.0,
        4.0,
        0.0,
    ]
    assert (
        feature.to_rapidjson().dumps()
        == '{"type":"Feature","geometry":{"type":"Point","coordinates":[3.0,4.0,0.0]},"properties":{"key":8},"my_key":"my_value"}'  # noqa
    )
    f2 = geojson.Feature().from_rapidjson(feature.to_rapidjson())
    assert f2 == feature
    assert f2() == feature()


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
