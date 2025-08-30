# qick start

```bash
pip install .
python test/test_api.py
```


# testpybind


## overview

1. build system with cmake
2. pip install with pyproject.toml
3. script to build wheels for conda py39~py312
4. same libstdc++ with conda default
5. bind python with pybind11
6. interface support both numpy 1 and numpy 2

## build

1. prerequesite: conda and pybind11 lib
    ```bash
    conda create -n py310 python=3.10
    conda activate py310
    # lib
    conda install -c conda-forge pybind11 -y
    # setup
    python -m pip install --upgrade pip setuptools wheel scikit-build pybind11
    ```

2. install and test
    ```bash
    pip install -v .
    python test/test_api.py
    ```

4. c++ build
    ```bash 
    # unix
    script/build_wheels.sh
    # windows
    script/build_wheels.bat
    ```




# implement 

Perfect! We can build a full **PyBind11 C++ project** with CMake, PyProject-based installation, multi-Python support (via Conda), and example APIs that handle both scalar addition and NumPy arrays. I’ll guide you step by step, including theory, reasoning, and practical implementation.

---

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
* `pyproject.toml` → PEP 517 build system
* `scripts/build_wheels.sh` → build for multiple Python versions
* `scripts/build_wheels.bat` → build for multiple Python versions
* `CMakeLists.txt` → main build system

---

## **2. CMake + PyBind11**

### Root `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.20)
project(TestPyBind LANGUAGES CXX)

# PyBind11
find_package(pybind11 REQUIRED)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add src
add_subdirectory(src)
```

### `src/CMakeLists.txt`

```cmake
pybind11_add_module(testpybind
    main.cpp
    array_ops.cpp
)

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

```toml
[build-system]
requires = ["setuptools>=61", "wheel", "pybind11>=2.10", "cmake"]
build-backend = "setuptools.build_meta"
```

---

## **5. Conda Multi-Python Build**

Create a shell script `scripts/build_wheels.sh`:

```bash
#!/bin/bash
PYTHON_VERSIONS=("3.9" "3.10" "3.11" "3.12")

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
import testpybind

# Scalar
print(testpybind.add_scalar(1, 2))
print(testpybind.add_scalar(1.5, 2.5))

# Array
a = np.array([1, 2, 3], dtype=np.float64)
b = np.array([4, 5, 6], dtype=np.float64)
print(testpybind.add_arrays(a, b))  # [5, 7, 9]
```

run test
```bash
pip install -v .
python test/test_api.py
```


build
```bash
# unix 
script/build_wheels.sh
# windows
script/build_wheels.bat
```

---

## **8. Build portable binary**

1. build by gcc 10
2. at any linux version, but better old linux versoin.
3. for each python version by conda activate special version env

conda default libstdc++ is for gcc10

## **9. Key Points / Documentation**

1. **PyBind11**: binds C++ → Python, handles scalars and NumPy arrays.
2. **CMake**: manages multi-platform C++ compilation.
3. **PEP 517 (`pyproject.toml`)**: standard Python build system.
4. **NumPy arrays**: use `unchecked` for fast elementwise ops.
5. **Conda wheel**: build per Python version, avoids GCC/libstdc++ conflicts.

---

