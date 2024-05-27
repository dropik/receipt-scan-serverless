find_package(ZLIB REQUIRED)
find_package(AWSSDK COMPONENTS core)

add_executable(rest_tests_run
    api_root_test.cpp
)

target_include_directories(rest_tests_run PUBLIC
    ${PROJECT_SOURCE_DIR}/rest/include
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR})

target_link_libraries(rest_tests_run
    ${AWSSDK_LINK_LIBRARIES}
    aws-lambda-cpp-common
    gtest
    gtest_main)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(rest_tests_run PRIVATE DEBUG)
endif()
