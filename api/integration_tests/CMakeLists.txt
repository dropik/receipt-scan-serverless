find_package(ZLIB REQUIRED)
find_package(AWSSDK COMPONENTS core ssm)

add_executable(api_integration_tests
    ../src/responses/category.hpp
    ../src/responses/file.hpp
    ../src/identity.hpp
    ../src/model_types.hpp
    ../src/responses/receipt_detail.hpp
    ../src/responses/receipt_list_item.hpp
    ../src/parameters/put_receipt.hpp
    ../src/s3_settings.hpp
    ../src/parameters/put_file.hpp
    ../src/services/category_service.hpp
    ../src/services/file_service.hpp
    ../src/services/receipt_service.hpp
    ../src/services/user_service.hpp
    ../src/api.hpp
    ../src/api_errors.hpp
    ../src/factories.hpp
    mocks/mock_s3_client.hpp
    mocks/mock_s3_client.cpp
    base_api_integration_test.hpp
    base_api_integration_test.cpp
    mocks/factories.hpp
    user_test.cpp
    ../src/responses/user.hpp
    ../src/responses/user.cpp
    devices_test.cpp
    ../src/services/device_service.hpp
    ../src/parameters/put_device.hpp
    budget_test.cpp
    ../src/services/budget_service.hpp
    ../src/parameters/put_budget.hpp
    ../src/parameters/put_budget.cpp
    ../src/responses/budget.hpp
    ../src/responses/budget.cpp
)

target_include_directories(api_integration_tests PUBLIC
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/repository/include"
    "${PROJECT_SOURCE_DIR}/di/include"
    "${PROJECT_SOURCE_DIR}/rest/include"
    "${PROJECT_SOURCE_DIR}/integration_tests_common/include"
)

target_link_libraries(api_integration_tests
    repository
    lambda
    rest
    ${AWSSDK_LINK_LIBRARIES}
    gtest
    integration_tests_common
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(api_integration_tests PRIVATE DEBUG)
endif()
