import awkward_xaod_bridge as xaodbridge


def test_pybind11():
    assert xaodbridge.calibrate([1,2,3], [4,5,6]).tolist() == [5,7,9]


def test_version():
    assert xaodbridge.__version__
