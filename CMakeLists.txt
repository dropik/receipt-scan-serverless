cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(receipt-scan-serverless VERSION 1.0.0 LANGUAGES CXX)

option(WITH_TESTS "Whether to enable or disable building tests" ON)

configure_file(include/config.h.in
  "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h")

if(WITH_TESTS)
    add_subdirectory(dependencies/test/googletest-1.14.0)
endif()
add_subdirectory(sam)
add_subdirectory(repository)
add_subdirectory(rest)
add_subdirectory(di)
add_subdirectory(lambda)
add_subdirectory(api)
add_subdirectory(scanner)
