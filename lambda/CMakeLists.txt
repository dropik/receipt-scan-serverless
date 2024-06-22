
add_library(lambda STATIC
    include/lambda/lambda.hpp
    src/lambda.cpp
    include/lambda/factories.hpp
    include/lambda/log.hpp
    src/log.cpp
)

target_include_directories(lambda PUBLIC
    "include"
    "${PROJECT_SOURCE_DIR}/di/include"
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(repository PRIVATE DEBUG)
endif()