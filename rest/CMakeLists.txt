add_library(rest STATIC
    include/rest/api_exception.hpp
    include/rest/api_resource.hpp
    include/rest/api_root.hpp
    include/rest/parsing.hpp
    include/rest/responses.hpp
    include/rest/types.hpp
    include/rest/utils.hpp
    src/responses.cpp
    src/utils.cpp
    src/parsing.cpp
    src/api_resource.cpp
    src/api_root.cpp
)

target_include_directories(rest PUBLIC
    include
    "${PROJECT_SOURCE_DIR}/lambda/include"
)

target_link_libraries(rest PUBLIC
    lambda
    aws-cpp-sdk-core
    AWS::aws-lambda-runtime
    ${aws-lambda-cpp-common}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(rest PRIVATE DEBUG)
endif()

if(WITH_TESTS)
    add_subdirectory(tests)
endif()
