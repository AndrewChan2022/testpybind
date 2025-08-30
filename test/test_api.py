import numpy as np
import pytest
import testpybind

def test_scalar():
    assert testpybind.add_scalar(1, 2) == 3
    assert testpybind.add_scalar(1.5, 2.5) == 4.0
    assert testpybind.add_scalar(1.2, 2.8) == pytest.approx(4.0)
    assert testpybind.fadd(1.2, 2.8) == pytest.approx(4.0)
    print(f"testpybind.fadd(1.2, 2.8) = {testpybind.fadd(1.2, 2.8)}")

def test_array():
    a = np.array([1, 2, 3], dtype=np.float64)
    b = np.array([4, 5, 6], dtype=np.float64)
    c = testpybind.add_arrays(a, b)
    np.testing.assert_array_equal(c, [5, 7, 9])
    print(f"{a} +  {b} = {c}")

    # test size mismatch
    a2 = np.array([1, 2], dtype=np.float64)
    with pytest.raises(RuntimeError):
        testpybind.add_arrays(a2, b)

if __name__ == "__main__":
    test_scalar()
    test_array()