

find_package(Python COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 REQUIRED)

message(STATUS "Found Python ${Python_VERSION} at ${Python_EXECUTABLE}")
message(STATUS "Found PyBind11 ${pybind11_VERSION} at ${pybind11_INCLUDE_DIR}")


add_executable(Demo ${CMAKE_CURRENT_SOURCE_DIR}/src/Test.cpp)
target_include_directories(Demo PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# target_link_directories(Demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/install/lib)
target_link_libraries(Demo
    myadd
    Python::Python
    pybind11::pybind11
)

if(MSVC)
    get_filename_component(PYTHON_DIR "${Python_EXECUTABLE}" DIRECTORY)
    set_target_properties(Demo PROPERTIES
        VS_DEBUGGER_ENVIRONMENT 
        "PATH=${PYTHON_DIR};%PATH%"
    )
endif()
