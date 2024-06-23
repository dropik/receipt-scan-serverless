find_package(ZLIB REQUIRED)
find_package(AWSSDK COMPONENTS core ssm)

add_executable(repository_integration_tests_run
    main.cpp
    receipt_repository_test.cpp
    base_repository_integration_test.hpp
    base_repository_integration_test.cpp
    client_test.cpp
)

target_include_directories(repository_integration_tests_run PUBLIC
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/repository/include"
    "${PROJECT_SOURCE_DIR}/di/include"
)

target_link_libraries(repository_integration_tests_run
    repository
    lambda
    ${AWSSDK_LINK_LIBRARIES}
    gtest
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(repository_integration_tests_run PRIVATE DEBUG)
endif()
