find_package(ZLIB REQUIRED)
find_package(AWSSDK COMPONENTS core)

add_executable(rest_tests
    api_root_test.cpp
)

target_include_directories(rest_tests PUBLIC
    ${PROJECT_SOURCE_DIR}/rest/include
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR})

target_link_libraries(rest_tests
    rest
    lambda
    ${AWSSDK_LINK_LIBRARIES}
    aws-lambda-cpp-common
    gtest
    gtest_main)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(rest_tests PRIVATE DEBUG)
endif()
