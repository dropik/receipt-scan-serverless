
add_library(lambda STATIC
    "include/lambda/runtime.hpp"
    "src/runtime.cpp"
)

target_include_directories(lambda PUBLIC
        "include")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(repository PRIVATE DEBUG)
endif()