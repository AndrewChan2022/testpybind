set(SRC_FILES
    testpybind/main.cpp
    testpybind/array_ops.cpp
)

target_sources(testpybind PRIVATE ${SRC_FILES})
target_include_directories(testpybind PRIVATE ${PYBIND11_INCLUDE_DIR})
target_link_libraries(testpybind PRIVATE 
    myadd
)

