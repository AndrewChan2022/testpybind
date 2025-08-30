# qick start

```bash
git clone https://github.com/AndrewChan2022/testpybind
cd testpybind

# install
pip install -v .
pip install numpy pytest

# test numpy 1.x
pip install numpy==1.23.5
python test/test_api.py

# test numpy 2.x
pip install --upgrade "numpy>=2.0.0"
python test/test_api.py
pytest -v test/test_api.py
```


# testpybind


## overview

features

    1. pyproject.toml + cmake for build and install
    2. script to build wheels for conda py39~py312
    3. auto install lib and dependency to wheel root
    4. bind c++ with pybind11
    5. interface for both numpy 1 and numpy 2
    6. same libstdc++ with conda default

table of content

    pyproject.toml + cmake to build wheel
        pybind11_add_module at root
        install to add binary to wheel:  install(TARGETS testpybind LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
    multiple python version build
        conda env
    pybind11
        bind c++
        numpy 1 and numpy 2 parameter
    max portable
        low os version
        low gcc version
    dependency lib
        cmake install collect lib and dependency
        rpath: todo

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

# this munally install will not be removed by pip uninstall
# install(TARGETS testpybind
#         LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})

install(TARGETS testpybind
        LIBRARY DESTINATION .   # <- means "install into the wheel root"
        RUNTIME DESTINATION .   # <- for Windows .pyd
        ARCHIVE DESTINATION .)  # <- in case MSVC produces .lib
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

## **3. pyproject.toml for pip/PEP517**

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


## **4. auto binary and dependency into wheel**

Python build system not collect libs and its dependency, This is done by cmake install.

### Install lib to wheel 

Install lib to wheel directory instead of site-pakcage directory.

**Insall dir != output dir**

```bash
# this munally install will not be removed by pip uninstall
# install(TARGETS testpybind
#         LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}
# )

install(TARGETS testpybind
    LIBRARY DESTINATION .  # <- means "install into the wheel root"
    RUNTIME DESTINATION .  # <- for Windows .pyd
    ARCHIVE DESTINATION .  # <- in case MSVC produces .lib
)
```


### Install dependency lib to wheel

add dependency targets to install list

```bash
install(TARGETS testpybind myadd
    ...
)
```

### judge wheel build and normal c++ build

```bash
if(SKBUILD)
    # Python wheel build, CMAKE_INSTALL_PREFIX to wheel directory
    set(INSTALL_BIN_DIR .)
    set(INSTALL_LIB_DIR .)
else()
    # Normal CMake build, to default CMAKE_INSTALL_PREFIX or fix place
    set(INSTALL_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/install/bin)
    set(INSTALL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/install/lib)
endif()
```

### gcc choose

Some lib build on high version gcc version, so you need choose the minimum version that compatible with your 3rd party libs.


For example:

    1. binary lib without source build on c++ 20,  you have to upgrade your gcc version.
    2. 3rd party source code with feature of c++ 20, you have to upgrade your gcc version.



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


## **6. Example C++ APIs**

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

## **7. Handling Exceptions and Safety**

* Scalars: straightforward
* Arrays: raise `std::runtime_error` if shapes mismatch
* You could optionally broadcast arrays using NumPy semantics if needed.

---

## **8. Testing**

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

## **9. Distribution**


build
```bash
# unix 
script/build_wheels.sh
# windows
script/build_wheels.bat
```

## **10. libstdc++**


### **Maximum Portable Binary**

1. Build with **GCC 10**, because conda’s default `libstdc++` is for GCC 10.
2. Can be built on any Linux version, but **older Linux versions are preferable** (for wider compatibility).
3. Must be built for each Python version separately (use `conda activate` with the desired Python version).

### **Why old Linux / old GCC is recommended**

1. `libstdc++` version is determined by the GCC version used to build your code.
2. `glibc` version is determined by the OS.
3. Each `libstdc++` binary is linked against a specific glibc version. i.e. You have many libstdc++ binary for one gcc version against each glibc version.
4. Most Linux distributions ship **only one glibc** and one `libstdc++` version.
5. Applications can ship their own `libstdc++` if needed.
6. Binaries built against a **lower `libstdc++` version** will generally run on systems with a **higher `libstdc++` version**.
7. Binaries built against a **lower glibc version** will generally run on systems with a **higher glibc version**.
8. **Summary:** `glibc version ↔ libstdc++ version ↔ application binary compatibility`.

```txt
           +--------------------+
           |     Application    |
           |   (built binary)   |
           +--------------------+
                     |
        Depends on linked libstdc++ version
                     |
           +--------------------+
           |     libstdc++      |
           | (from GCC used to  |
           |   build app)       |
           +--------------------+
                     |
   Requires minimum glibc version on target OS
                     |
           +--------------------+
           |       glibc        |
           |   (from Linux OS)  |
           +--------------------+
```

### **Distribution strategy**

1. **Ship your own `libstdc++` (most apps do this)**

   * Build with **any GCC version** on a **low Linux version**.
   * Ensures your app runs anywhere, independent of system libraries.
2. **Depend on system `libstdc++` (used by most Python packages)**

   * Build with **low GCC version** on **any Linux version**.
   * Guarantees compatibility with system libraries.


✅ **Key takeaway:**

* Building on old Linux with old GCC gives maximum portability because the resulting `libstdc++` and glibc dependencies are minimal.
* Shipping your own `libstdc++` removes dependency on the system, at the cost of slightly larger binaries.

---

## **12. rpath**

todo:

```bash
# Make installed executables look in ../lib
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")

# Also keep RPATH when running from build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
```

or set per target
```bash
set_target_properties(myexe PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
)
```


## **11. Key Points / Documentation**

1. **PyBind11**: binds C++ → Python, handles scalars and NumPy arrays.
2. **CMake**: manages multi-platform C++ compilation
3. **pybind11_add_module** must at root cmake
4. **cmake install** is mandatory: install(TARGETS testpybind LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES})
5. **dependency** need collect dependency in cmake install
6. **PEP 517 (`pyproject.toml`)**: standard Python build system.
7. **NumPy arrays**: use `unchecked` for fast elementwise ops.
8. **Conda wheel**: build per Python version, avoids GCC/libstdc++ conflicts.
9. **rpath**: todo

## **12. reference**

1. build system: docment: https://pybind11.readthedocs.io/en/stable/compiling.html

---

