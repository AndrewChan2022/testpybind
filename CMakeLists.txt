cmake_minimum_required(VERSION 3.20)
project(TestPyBind LANGUAGES CXX)

# Find pybind11
find_package(pybind11 REQUIRED)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================= msvc Settings =========================
if (MSVC)
    # msvc parallel build
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /showIncludes")   # debug header include chain
    add_definitions(-DNOMINMAX)                                 # no MIN MAX
    add_definitions(-D_WIN32_WINNT=0x0A00)                      # windows version, both win10 and win11 is 0x0A00
    add_compile_options(/bigobj)                                # error of not enough section
    add_compile_options(/permissive-)                           # Qt introduce this, but Qt was removed on windows
    add_compile_definitions(_USE_MATH_DEFINES)                  # M_PI
    add_compile_options(/utf-8)
    # ──────────────────────────────────────────────────────────────
    # debug code for release config
    # uncomment the second line to force ON and overwrite cache
    # ──────────────────────────────────────────────────────────────
    option(MSVC_RELEASE_DEBUG "Turn off /O2, enable debug info" OFF) # declare cache entry, default OFF
    # set(MSVC_RELEASE_DEBUG ON CACHE BOOL "Turn off /O2, enable debug info" FORCE) # uncomment this to overwrite cache
    if (MSVC_RELEASE_DEBUG)

        message(WARNING "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")
        message(WARNING "XXXXXXXXXX enable debug for release XXXXXXXXXXXXXX")
        message(WARNING "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX")

        string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "/O2" "/Od" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        add_compile_options(/Zi)
        add_link_options(/DEBUG)

        message("C++ Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
        message("C Flags: ${CMAKE_C_FLAGS_RELEASE}")
    endif()

    # vcpkg toolchain
    # set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

    # cgal gmp/mpfr lib
    # set(GMP_INCLUDE_DIR "C:/dev/gmp/include" CACHE PATH "Path to GMP include directory")
    # set(GMP_LIBRARIES "C:/dev/gmp/lib/libgmp-10.lib" CACHE FILEPATH "Path to GMP library")
    # set(MPFR_INCLUDE_DIR "C:/dev/gmp/include" CACHE PATH "Path to GMP include directory")
    # set(MPFR_LIBRARIES "C:/dev/gmp/lib/libmpfr-4.lib" CACHE FILEPATH "Path to GMP library")
else()
    # set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")                 # position-independent code for unix
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")             # position-independent code for unix
endif()

# Extend CMake's module search path so that "include()" and "find_package()"
# can also look inside our project's custom 'cmake/' directory at the root.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ========================= Global Settings =========================

# ========================= Project Settings =========================

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