add_executable(rest_tests
    api_root_test.cpp
)

target_include_directories(rest_tests PUBLIC
    ${PROJECT_SOURCE_DIR}/rest/include
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR})

target_link_libraries(rest_tests
    rest
    gtest
    gtest_main)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(rest_tests PRIVATE DEBUG)
endif()
