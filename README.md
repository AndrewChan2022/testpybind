# qick start

```bash
git clone https://github.com/AndrewChan2022/testpybind
cd testpybind

# install
pip install -v .
pip install numpy pytest

# test
python test/test_api.py
pytest -v test/test_api.py
```


# testpybind


## overview

1. pyproject.toml + cmake for build and install
2. script to build wheels for conda py39~py312
3. bind c++ with pybind11
4. interface for both numpy 1 and numpy 2
5. same libstdc++ with conda default

## build

1. prerequesite: conda env
    ```bash
    conda create -n py310 python=3.10
    conda activate py310
    pip install numpy pytest
    ```

2. install and test
    ```bash
    pip install -v .
    python test/test_api.py
    ```

3. wheel build
    ```bash 
    # unix
    script/build_wheels.sh
    # windows
    script/build_wheels.bat
    ```
4. build c++ only, not necessary
   ```bash
    # manually install pybind11, pip install will auto install it
    conda install -c conda-forge pybind11 -y

    # build with msvc
    mkdir vsbuild
    cd vsbuild
    cmake ..
   ```


# implement 


## **1. Project Structure**

We want a clean project structure:

```
TestPyBind/
├── CMakeLists.txt
├── pyproject.toml
├── src/
│   ├── CMakeLists.txt
│   └── testpybind/
│       ├── main.cpp
│       └── array_ops.cpp
├── scripts/
│   ├── build_wheels.sh
│   └── build_wheels.bat
│── tests/
|    └── test_api.py
└── README.md
```

* `src/testpybind` → C++ source code
* `test` → python test code
* `pyproject.toml` → PEP 517 build system
* `scripts/build_wheels.sh` → build for multiple Python versions
* `scripts/build_wheels.bat` → build for multiple Python versions
* `CMakeLists.txt` → main build system

---

## **2. CMake + PyBind11**

### Root `CMakeLists.txt`

document: https://pybind11.readthedocs.io/en/stable/compiling.html

1. root cmake must have pybind11_add_module
2. must install: install(TARGETS testpybind LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})

```cmake
cmake_minimum_required(VERSION 3.20)
project(TestPyBind LANGUAGES CXX)

# Find pybind11
find_package(pybind11 REQUIRED)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pybind11_add_module(testpybind)

# Add src
add_subdirectory(src)

install(TARGETS testpybind
        LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
```

### `src/CMakeLists.txt`

```cmake
set(SRC_FILES
    testpybind/main.cpp
    testpybind/array_ops.cpp
)

target_sources(testpybind PRIVATE ${SRC_FILES})
target_include_directories(testpybind PRIVATE ${PYBIND11_INCLUDE_DIR})

```

---

## **3. Example C++ APIs**

### `main.cpp` – scalar addition

```cpp
#include <pybind11/pybind11.h>

namespace py = pybind11;

template<typename T>
T add_scalar(T a, T b) {
    return a + b;
}

void bind_scalar(py::module &m) {
    m.def("add_scalar", &add_scalar<int>, "Add two ints");
    m.def("add_scalar", &add_scalar<float>, "Add two floats");
    m.def("add_scalar", &add_scalar<double>, "Add two doubles");
}

// declare
void bind_array(pybind11::module &m);

PYBIND11_MODULE(testpybind, m) {
    m.doc() = "TestPyBind module";

    // Call sub-bind functions from each file
    bind_scalar(m);
    bind_array(m);
}
```

### `array_ops.cpp` – NumPy addition

```cpp
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <stdexcept>

namespace py = pybind11;

py::array_t<double> add_arrays(py::array_t<double> a, py::array_t<double> b) {
    if (a.size() != b.size())
        throw std::runtime_error("Arrays must have the same size");

    auto result = py::array_t<double>(a.size());
    auto buf_a = a.unchecked<1>();
    auto buf_b = b.unchecked<1>();
    auto buf_r = result.mutable_unchecked<1>();

    for (py::ssize_t i = 0; i < a.size(); ++i)
        buf_r(i) = buf_a(i) + buf_b(i);

    // Correct: convert shape to vector
    std::vector<py::ssize_t> shape(a.ndim());
    for (py::ssize_t i = 0; i < a.ndim(); ++i)
        shape[i] = a.shape(i);

    result.resize(shape);
    return result;
}

void bind_array(py::module &m) {
    m.def("add_arrays", &add_arrays, "Add two numpy arrays elementwise");
}

```

> **Theory**:
>
> * `unchecked<1>()` gives fast access without Python GIL overhead.
> * `py::array_t<double>` supports both numpy 1.x and 2.x.
> * Resizing the result preserves the original shape.

---

## **4. pyproject.toml for pip/PEP517**

document: https://pybind11.readthedocs.io/en/stable/compiling.html

```toml
[build-system]
requires = ["scikit-build-core", "pybind11"]
build-backend = "scikit_build_core.build"

[project]
name = "testpybind"
version = "0.1.0"
description = "Test project using pybind11 + CMake"
authors = [{name = "Bill", email = "you@example.com"}]
requires-python = ">=3.9"
```

---

## **5. Conda Multi-Python Build**

Create a shell script `scripts/build_wheels.sh`:

```bash
#!/bin/bash
PYTHON_VERSIONS=("39" "310" "311" "312")

for ver in "${PYTHON_VERSIONS[@]}"; do
    conda activate "py${ver}"
    python -m pip install --upgrade pip setuptools wheel cmake pybind11

    python -m build --wheel --outdir dist
done
```

> **Theory**:
>
> * Using Conda Python ensures consistent `libstdc++`.
> * Wheel is built per Python version.
> * No need to upgrade system GCC/libstdc++ inside Conda.

---

## **6. Handling Exceptions and Safety**

* Scalars: straightforward
* Arrays: raise `std::runtime_error` if shapes mismatch
* You could optionally broadcast arrays using NumPy semantics if needed.

---

## **7. Testing**

Python tests:

```python
import numpy as np
import pytest
import testpybind

def test_scalar():
    assert testpybind.add_scalar(1, 2) == 3
    assert testpybind.add_scalar(1.5, 2.5) == 4.0
    assert testpybind.add_scalar(1.2, 2.8) == pytest.approx(4.0)

def test_array():
    a = np.array([1, 2, 3], dtype=np.float64)
    b = np.array([4, 5, 6], dtype=np.float64)
    c = testpybind.add_arrays(a, b)
    np.testing.assert_array_equal(c, [5, 7, 9])
    print(f"a {a} +  b {b} => c {c}")

    # test size mismatch
    a2 = np.array([1, 2], dtype=np.float64)
    with pytest.raises(RuntimeError):
        testpybind.add_arrays(a2, b)

if __name__ == "__main__":
    test_scalar()
    test_array()
```

run test
```bash
pip install -v .
python test/test_api.py
pytest -v test/test_api.py
```

## **8. Distribution**


build
```bash
# unix 
script/build_wheels.sh
# windows
script/build_wheels.bat
```

## **8. libstdc++ **

Maximum portable binary**

    1. build by gcc 10, conda default libstdc++ is for gcc10
    2. build at any linux version, but better old linux versoin
    3. for each python version by conda activate special version env


why old linux old gcc:

    1. libstdc++ version decided by gcc version
    2. glibc version decided by os
    3. each libstdc++ have many binary for each glibc version
    4. os usually ship one libstdc++ version
    5. app can also ship one libstdc++ version
    6. bin depend on low libstdc++ version can run on high libstdc++ version
    7. libstdc++ version depend on low glibc version can run on high glibc version
    8. summary:  glibc version <--> libstdc++ version <--> app version

distribution strategy:

    1. strategy 1: app ship libstdc++:   most app use this stragegy
       build with any gcc version on low linux version
    2. strategy 2: app depend on system libstdc++:  python lib use this strategy
       build with low gcc version on any linux version


## **9. Key Points / Documentation**

1. **PyBind11**: binds C++ → Python, handles scalars and NumPy arrays.
2. **CMake**: manages multi-platform C++ compilation
3. **pybind11_add_module** must at root cmake
4. **cmake install** is mandatory: install(TARGETS testpybind LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
5. **PEP 517 (`pyproject.toml`)**: standard Python build system.
6. **NumPy arrays**: use `unchecked` for fast elementwise ops.
7. **Conda wheel**: build per Python version, avoids GCC/libstdc++ conflicts.

## **10. reference**

1. build system: docment: https://pybind11.readthedocs.io/en/stable/compiling.html

---

