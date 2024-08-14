add_library(integration_tests_common STATIC
    src/main.cpp
    include/integration_tests_common/repository_integration_test.hpp
    src/repository_integration_test.cpp
)

target_include_directories(integration_tests_common PUBLIC
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/integration_tests_common/include"
    "${PROJECT_SOURCE_DIR}/lambda/include"
)

target_link_libraries(integration_tests_common
    gtest
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(integration_tests_common PRIVATE DEBUG)
endif()
