add_executable(scanner_integration_tests
    scanner_test.cpp
    ../src/models/bedrock_payload.hpp
    ../src/models/bedrock_response.hpp
    ../src/handler.hpp
    ../src/utils.hpp
    ../src/utils.cpp
    ../src/services/receipt_extractor.hpp
    ../src/services/categorizer.hpp
)

target_include_directories(scanner_integration_tests PUBLIC
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/di/include"
    "${PROJECT_SOURCE_DIR}/integration_tests_common/include"
)

target_link_libraries(scanner_integration_tests
    lambda
    repository
    aws-cpp-sdk-core
    aws-cpp-sdk-s3
    aws-cpp-sdk-textract
    aws-cpp-sdk-bedrock-runtime
    AWS::aws-lambda-runtime
    gtest
    integration_tests_common
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(scanner_integration_tests PRIVATE DEBUG)
endif()
