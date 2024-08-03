find_package(ZLIB REQUIRED)
find_package(AWSSDK COMPONENTS core ssm)

add_executable(api_integration_tests
    ../src/models/category.hpp
    ../src/models/file.hpp
    ../src/models/identity.hpp
    ../src/models/model_types.hpp
    ../src/models/receipt_detail.hpp
    ../src/models/receipt_list_item.hpp
    ../src/models/receipt_put_params.hpp
    ../src/models/s3_settings.hpp
    ../src/models/upload_file_params.hpp
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
    ${AWSSDK_LINK_LIBRARIES}
    gtest
    integration_tests_common
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(api_integration_tests PRIVATE DEBUG)
endif()
