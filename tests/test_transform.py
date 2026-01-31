from __future__ import annotations

import numpy as np
import pytest

from pybind11_geobuf import geojson


def sample_coords():
    """Sample WGS84 coordinates (lon, lat, alt)"""
    return np.array(
        [
            [120.40317479950272, 31.416966084052177, 1.0],
            [120.28451900911591, 31.30578266928819, 2.0],
            [120.35592249359615, 31.21781895672254, 3.0],
            [120.67093786630113, 31.299502266522722, 4.0],
        ]
    )


def sample_anchor():
    """Sample anchor point for ENU conversion"""
    return np.array([120.4, 31.3, 0.0])


class TestTransformPoint:
    def test_transform_custom_function(self):
        pt = geojson.Point(1.0, 2.0, 3.0)

        def double_coords(coords):
            coords[:] *= 2

        pt.transform(double_coords)
        assert pt() == [2.0, 4.0, 6.0]

    def test_transform_return_new_array(self):
        pt = geojson.Point(1.0, 2.0, 3.0)

        def add_offset(coords):
            return coords + 10

        pt.transform(add_offset)
        assert pt() == [11.0, 12.0, 13.0]

    def test_to_enu_to_wgs84_roundtrip(self):
        pt = geojson.Point(120.4, 31.3, 100.0)
        original = pt.to_numpy().copy()
        anchor = np.array([120.4, 31.3, 0.0])

        pt.to_enu(anchor)
        # After to_enu, coordinates should be near origin in ENU
        enu_coords = pt.to_numpy()
        assert np.abs(enu_coords[0]) < 1.0  # east ~0
        assert np.abs(enu_coords[1]) < 1.0  # north ~0
        assert np.abs(enu_coords[2] - 100.0) < 0.01  # up ~100

        pt.to_wgs84(anchor)
        np.testing.assert_allclose(pt.to_numpy(), original, rtol=1e-6)

    def test_translate(self):
        pt = geojson.Point(1.0, 2.0, 3.0)
        pt.translate(np.array([10.0, 20.0, 30.0]))
        assert pt() == [11.0, 22.0, 33.0]

    def test_scale(self):
        pt = geojson.Point(1.0, 2.0, 3.0)
        pt.scale(np.array([2.0, 3.0, 4.0]))
        assert pt() == [2.0, 6.0, 12.0]

    def test_rotate(self):
        pt = geojson.Point(1.0, 0.0, 0.0)
        # Rotate 90 degrees around Z axis
        R = np.array([[0, -1, 0], [1, 0, 0], [0, 0, 1]], dtype=float)
        pt.rotate(R)
        np.testing.assert_allclose(pt.to_numpy(), [0.0, 1.0, 0.0], atol=1e-10)

    def test_affine(self):
        pt = geojson.Point(1.0, 2.0, 3.0)
        T = np.eye(4)
        T[:3, 3] = [10, 20, 30]  # translation
        pt.affine(T)
        assert pt() == [11.0, 22.0, 33.0]
        pt = geojson.Point(1.0, 2.0, 3.0)
        T[:, 0] *= 5
        T[:, 1] *= 2
        pt.affine(T)
        assert pt() == [15.0, 24.0, 33.0]


class TestTransformLineString:
    def test_transform_custom_function(self):
        ls = geojson.LineString(sample_coords())
        original_shape = ls.to_numpy().shape

        def add_noise(coords):
            coords[:, 2] += 10.0

        ls.transform(add_noise)
        result = ls.to_numpy()
        assert result.shape == original_shape
        np.testing.assert_allclose(result[:, 2], sample_coords()[:, 2] + 10.0)

    def test_to_enu_to_wgs84_roundtrip(self):
        ls = geojson.LineString(sample_coords())
        original = ls.to_numpy().copy()
        anchor = sample_anchor()

        ls.to_enu(anchor)
        # After to_enu, coords should be in local ENU frame
        enu_coords = ls.to_numpy()
        assert not np.allclose(enu_coords, original)  # should be different

        ls.to_wgs84(anchor)
        np.testing.assert_allclose(ls.to_numpy(), original, rtol=1e-6)

    def test_translate(self):
        ls = geojson.LineString([[0, 0, 0], [1, 1, 1]])
        ls.translate(np.array([10.0, 20.0, 30.0]))
        expected = [[10, 20, 30], [11, 21, 31]]
        np.testing.assert_allclose(ls.to_numpy(), expected)

    def test_chain_operations(self):
        ls = geojson.LineString([[0, 0, 0], [1, 1, 1]])
        result = ls.translate(np.array([1, 1, 1])).scale(np.array([2, 2, 2]))
        assert result == ls  # chain should return self
        expected = [[2, 2, 2], [4, 4, 4]]
        np.testing.assert_allclose(ls.to_numpy(), expected)


class TestTransformMultiPoint:
    def test_transform(self):
        mp = geojson.MultiPoint(sample_coords())
        mp.translate(np.array([1.0, 2.0, 3.0]))
        result = mp.to_numpy()
        expected = sample_coords() + np.array([1.0, 2.0, 3.0])
        np.testing.assert_allclose(result, expected)


class TestTransformPolygon:
    def test_transform(self):
        coords = [[0, 0, 0], [1, 0, 0], [1, 1, 0], [0, 1, 0], [0, 0, 0]]
        poly = geojson.Polygon(coords)
        poly.translate(np.array([10.0, 20.0, 30.0]))
        result = poly.to_numpy()
        expected = np.array(coords) + np.array([10.0, 20.0, 30.0])
        np.testing.assert_allclose(result, expected)


class TestTransformMultiLineString:
    def test_transform(self):
        coords = sample_coords()
        mls = geojson.MultiLineString(coords)
        mls.push_back(coords * 2)

        mls.translate(np.array([1.0, 1.0, 1.0]))

        # Both line strings should be translated
        for ls in mls:
            assert ls.to_numpy()[0, 2] != 1.0  # z was 1 or 2, now +1


class TestTransformMultiPolygon:
    def test_transform(self):
        coords = [[0, 0, 0], [1, 0, 0], [1, 1, 0], [0, 0, 0]]
        mpoly = geojson.MultiPolygon(coords)
        mpoly.translate(np.array([5.0, 5.0, 5.0]))
        expected = np.array(coords) + np.array([5.0, 5.0, 5.0])
        # MultiPolygon.to_numpy returns the first polygon's first ring
        np.testing.assert_allclose(mpoly.as_numpy(), expected)


class TestTransformGeometryCollection:
    def test_transform(self):
        gc = geojson.GeometryCollection()
        gc.push_back(geojson.Point(1, 2, 3))
        gc.push_back(geojson.LineString([[0, 0, 0], [1, 1, 1]]))

        gc.translate(np.array([10.0, 10.0, 10.0]))

        pt = gc[0].as_point()
        assert pt() == [11.0, 12.0, 13.0]

        ls = gc[1].as_line_string()
        np.testing.assert_allclose(ls.to_numpy(), [[10, 10, 10], [11, 11, 11]])


class TestTransformFeature:
    def test_transform(self):
        f = geojson.Feature()
        f.geometry(geojson.LineString(sample_coords()))
        original = f.to_numpy().copy()

        f.translate(np.array([0.0, 0.0, 100.0]))
        result = f.to_numpy()

        np.testing.assert_allclose(result[:, :2], original[:, :2])
        np.testing.assert_allclose(result[:, 2], original[:, 2] + 100.0)

    def test_to_enu_to_wgs84_roundtrip(self):
        f = geojson.Feature()
        f.geometry(geojson.LineString(sample_coords()))
        original = f.to_numpy().copy()
        anchor = sample_anchor()

        f.to_enu(anchor).to_wgs84(anchor)
        np.testing.assert_allclose(f.to_numpy(), original, rtol=1e-6)


class TestTransformFeatureCollection:
    def test_transform(self):
        fc = geojson.FeatureCollection()
        f1 = geojson.Feature()
        f1.geometry(geojson.Point(1, 2, 3))
        f2 = geojson.Feature()
        f2.geometry(geojson.LineString([[0, 0, 0], [1, 1, 1]]))
        fc.append(f1)
        fc.append(f2)

        fc.translate(np.array([10.0, 10.0, 10.0]))

        assert fc[0].geometry().as_point()() == [11.0, 12.0, 13.0]
        np.testing.assert_allclose(
            fc[1].geometry().as_line_string().to_numpy(), [[10, 10, 10], [11, 11, 11]]
        )

    def test_to_enu_to_wgs84_roundtrip(self):
        fc = geojson.FeatureCollection()
        f = geojson.Feature()
        f.geometry(geojson.LineString(sample_coords()))
        fc.append(f)

        original = fc[0].to_numpy().copy()
        anchor = sample_anchor()

        fc.to_enu(anchor).to_wgs84(anchor)
        np.testing.assert_allclose(fc[0].to_numpy(), original, rtol=1e-6)


class TestTransformGeometry:
    def test_transform_point(self):
        g = geojson.Geometry(geojson.Point(1, 2, 3))
        g.translate(np.array([10.0, 10.0, 10.0]))
        assert g.as_point()() == [11.0, 12.0, 13.0]

    def test_transform_line_string(self):
        g = geojson.Geometry(geojson.LineString([[0, 0, 0], [1, 1, 1]]))
        g.scale(np.array([2.0, 2.0, 2.0]))
        np.testing.assert_allclose(
            g.as_line_string().to_numpy(), [[0, 0, 0], [2, 2, 2]]
        )


class TestTransformGeoJSON:
    def test_transform_geometry(self):
        gj = geojson.GeoJSON(geojson.Geometry(geojson.Point(1, 2, 3)))
        gj.translate(np.array([10.0, 10.0, 10.0]))
        assert gj.as_geometry().as_point()() == [11.0, 12.0, 13.0]

    def test_transform_feature(self):
        f = geojson.Feature()
        f.geometry(geojson.Point(1, 2, 3))
        gj = geojson.GeoJSON(f)
        gj.translate(np.array([10.0, 10.0, 10.0]))
        assert gj.as_feature().geometry().as_point()() == [11.0, 12.0, 13.0]


class TestCheapRulerOption:
    def test_cheap_ruler_vs_full(self):
        ls = geojson.LineString(sample_coords())
        anchor = sample_anchor()

        # With cheap_ruler=True (default)
        ls_cheap = ls.clone()
        ls_cheap.to_enu(anchor, cheap_ruler=True)

        # With cheap_ruler=False
        ls_full = ls.clone()
        ls_full.to_enu(anchor, cheap_ruler=False)

        # Results should be similar but not identical
        cheap_coords = ls_cheap.to_numpy()
        full_coords = ls_full.to_numpy()

        # X and Y should be close (within a few hundred meters for typical scenarios)
        # Z differs significantly because cheap_ruler preserves original Z while
        # full transform converts through ECEF which changes altitude reference
        # Use absolute tolerance of 100m for comparison since cheap_ruler is an approximation
        # and the test data spans ~30km, so 100m error is reasonable
        np.testing.assert_allclose(cheap_coords[:, :2], full_coords[:, :2], atol=100)


def test_chain_multiple_transforms():
    """Test chaining multiple transform operations"""
    fc = geojson.FeatureCollection()
    f = geojson.Feature()
    f.geometry(geojson.LineString(sample_coords()))
    fc.append(f)

    anchor = sample_anchor()

    # Chain: to_enu -> translate -> rotate -> to_wgs84
    R = np.eye(3)  # identity rotation

    # This should work without errors
    result = fc.to_enu(anchor).translate(np.array([100.0, 100.0, 0.0])).rotate(R)

    # Verify result is the same object (fluent interface)
    assert result is fc


def test_transform_preserves_properties():
    """Test that transform operations preserve feature properties"""
    f = geojson.Feature()
    f.geometry(geojson.LineString([[0, 0, 0], [1, 1, 1]]))
    f.properties({"name": "test", "value": 42})

    f.translate(np.array([10.0, 10.0, 10.0]))

    # Properties should be unchanged
    assert f.properties()["name"]() == "test"
    assert f.properties()["value"]() == 42

    # Geometry should be translated
    np.testing.assert_allclose(f.to_numpy(), [[10, 10, 10], [11, 11, 11]])


if __name__ == "__main__":
    import os
    import sys

    os.chdir(os.path.dirname(__file__))
    sys.exit(pytest.main([__file__, "-v", "-x"]))
