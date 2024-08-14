add_library(lambda STATIC
    include/lambda/lambda.hpp
    src/lambda.cpp
    include/lambda/factories.hpp
    include/lambda/log.hpp
    src/log.cpp
    include/lambda/utils.hpp
    src/utils.cpp
)

target_include_directories(lambda PUBLIC
    "include"
    "${PROJECT_SOURCE_DIR}/di/include"
    aws-sdk-cpp-core
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(lambda PRIVATE DEBUG)
endif()
