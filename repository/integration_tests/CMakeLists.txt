add_executable(repository_integration_tests
    receipt_repository_test.cpp
    ../../integration_tests_common/include/integration_tests_common/repository_integration_test.hpp
    ../../integration_tests_common/src/repository_integration_test.cpp
    client_test.cpp
    ../include/repository/configurations/common/version_configuration.hpp
    ../include/repository/models/budget.hpp
    ../include/repository/configurations/budget_configuration.hpp
    base_repository_integration_test.hpp
    base_repository_integration_test.cpp
    ../include/repository/exceptions.hpp
)

target_include_directories(repository_integration_tests PUBLIC
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/repository/include"
    "${PROJECT_SOURCE_DIR}/di/include"
    "${PROJECT_SOURCE_DIR}/integration_tests_common/include"
)

target_link_libraries(repository_integration_tests
    repository
    lambda
    gtest
    integration_tests_common
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(repository_integration_tests PRIVATE DEBUG)
endif()
