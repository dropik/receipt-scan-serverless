cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(receipt-scan-serverless VERSION 1.0.0 LANGUAGES CXX)

configure_file(include/config.h.in
  "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h")

add_subdirectory(api)
add_subdirectory(scanner)
