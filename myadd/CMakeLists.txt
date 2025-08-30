
set(SRC_FILES
    src/fadd.cpp
)

add_library(myadd SHARED)
target_sources(myadd PRIVATE ${SRC_FILES})
target_include_directories(myadd PUBLIC include)

target_compile_definitions(myadd PRIVATE MYADD_EXPORTS)
