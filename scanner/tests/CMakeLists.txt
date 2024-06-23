find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3 textract bedrock-runtime)

add_executable(scanner_tests_run
    scanner_test.cpp
    ../src/models/bedrock_payload.hpp
    ../src/models/bedrock_response.hpp
    ../src/handler.hpp
    ../src/utils.hpp
    ../src/utils.cpp
    ../src/services/receipt_extractor.hpp
    ../src/services/receipt_repository.hpp
)

target_include_directories(scanner_tests_run PUBLIC
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/di/include"
)

target_link_libraries(scanner_tests_run
    lambda
    repository
    AWS::aws-lambda-runtime
    ${AWSSDK_LINK_LIBRARIES}
    aws-lambda-cpp-common
    gtest
    gtest_main)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(scanner_tests_run PRIVATE DEBUG)
endif()
