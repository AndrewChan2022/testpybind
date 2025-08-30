pybind11_add_module(testpybind
    testpybind/main.cpp
    testpybind/array_ops.cpp
)

target_include_directories(testpybind PRIVATE ${PYBIND11_INCLUDE_DIR})
