cmake_minimum_required(VERSION 3.20)
project(TestPyBind LANGUAGES CXX)

# Find pybind11
find_package(pybind11 REQUIRED)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pybind11_add_module(testpybind)


# this munally install will not be removed by pip uninstall
# install(TARGETS testpybind
#         LIBRARY DESTINATION ${PYTHON_SITE_PACKAGES}
# )

if(SKBUILD)
    # Python wheel build, CMAKE_INSTALL_PREFIX to wheel directory
    set(INSTALL_BIN_DIR .)
    set(INSTALL_LIB_DIR .)
else()
    # Normal CMake build, to default CMAKE_INSTALL_PREFIX or fix place
    set(INSTALL_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/install/bin)
    set(INSTALL_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/install/lib)
endif()

# output dir
if(MSVC)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INSTALL_BIN_DIR}")     # .lib (static library)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${INSTALL_BIN_DIR}")     # 
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${INSTALL_BIN_DIR}")     # .exe, .dll runtime
else()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${INSTALL_LIB_DIR}")     # .lib (static library)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${INSTALL_LIB_DIR}")     # .so import lib
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${INSTALL_BIN_DIR}")     # .exe, runtime
endif()


# Add src
add_subdirectory(myadd)
add_subdirectory(src)
add_subdirectory(demo)

# install dir
if(MSVC)
    install(TARGETS testpybind myadd
        LIBRARY DESTINATION ${INSTALL_BIN_DIR}   # <- means "install into the wheel root"
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}   # <- for Windows .pyd
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}   # <- in case MSVC produces .lib
    )
else()
    install(TARGETS testpybind myadd
        LIBRARY DESTINATION ${INSTALL_LIB_DIR}   # <- means "install into the wheel root"
        RUNTIME DESTINATION ${INSTALL_BIN_DIR}   # <- for linux .so
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}   # <- linux static lib (.archived)
    )
endif()