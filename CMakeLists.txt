cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(receipt-recognition-service VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(file-uploader)
add_subdirectory(receipt-recognition-service)

