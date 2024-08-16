cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(receipt-scan-serverless VERSION 1.1.1 LANGUAGES CXX)

option(WITH_TESTS "Whether to enable or disable building tests" ON)

configure_file(include/config.h.in
  "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h")

find_package(ZLIB REQUIRED)
include(Findzstd.cmake)
find_library(aws-lambda-cpp-common aws-lambda-cpp-common)
message("Found aws-lambda-cpp-common: ${aws-lambda-cpp-common}")
find_library(gssapi_krb5 gssapi_krb5)
message("Found gssapi_krb5: ${gssapi_krb5}")
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3 textract bedrock-runtime ssm)

if(WITH_TESTS)
    add_subdirectory(dependencies/test/googletest-1.14.0)
    add_subdirectory(integration_tests_common)
endif()
add_subdirectory(sam)
add_subdirectory(repository)
add_subdirectory(rest)
add_subdirectory(di)
add_subdirectory(lambda)
add_subdirectory(api)
add_subdirectory(scanner)
