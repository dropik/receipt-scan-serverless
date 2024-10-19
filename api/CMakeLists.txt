set(FUNCTION_NAME "api")
set(LAMBDA_ID "LApi")

add_executable(${FUNCTION_NAME}
    src/main.cpp
    src/responses/file.hpp
    src/api.hpp
    src/services/file_service.hpp
    src/api_errors.hpp
    src/identity.hpp
    src/services/user_service.hpp
    src/responses/receipt.hpp
    src/model_types.hpp
    src/services/receipt_service.hpp
    src/services/category_service.hpp
    src/responses/category.hpp
    src/parameters/put_receipt.hpp
    src/factories.hpp
    src/settings/s3_settings.hpp
    src/responses/user.hpp
    src/responses/user.cpp
    src/parameters/put_device.hpp
    src/services/budget_service.hpp
    src/parameters/put_budget.hpp
    src/parameters/put_budget.cpp
    src/responses/budget.hpp
    src/responses/budget.cpp
    src/parameters/put_category.hpp
    src/parameters/put_category.cpp
    src/responses/category.cpp
    src/parameters/put_receipt.cpp
    src/parameters/put_receipt_item.hpp
    src/parameters/put_receipt_item.cpp
    src/responses/receipt_item.hpp
    src/responses/receipt.cpp
    src/responses/receipt_item.cpp
    src/responses/change.hpp
    src/services/changes_service.hpp
    src/http_request.hpp
    src/settings/cognito_settings.hpp
    src/dependencies/jwt-cpp/jwt.h
    src/settings/google_api_settings.hpp
    src/services/google_api_auth_provider.hpp
    src/services/google_api_auth_provider.cpp
    src/parameters/rtdn.hpp
    src/parameters/gp_notification.hpp
    src/services/rtdn_service.hpp
)

target_include_directories(${FUNCTION_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/repository/include/"
    "${PROJECT_SOURCE_DIR}/rest/include/"
    "${PROJECT_SOURCE_DIR}/di/include/"
    "${PROJECT_SOURCE_DIR}/lambda/include/"
)

target_link_libraries(${FUNCTION_NAME} PUBLIC
    AWS::aws-lambda-runtime
    aws-cpp-sdk-core
    aws-cpp-sdk-s3
    aws-cpp-sdk-cognito-idp
    ${aws-lambda-cpp-common}
    repository
    rest
    lambda
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${FUNCTION_NAME} PRIVATE DEBUG)
endif()

file(WRITE "${PROJECT_BINARY_DIR}/${FUNCTION_NAME}/Makefile"
    "build-${LAMBDA_ID}:\n"
    "\tmkdir -p $(ARTIFACTS_DIR)/bin\n"
    "\tcp ${FUNCTION_NAME} $(ARTIFACTS_DIR)/bin\n"
    "\tln -sf bin/${FUNCTION_NAME} $(ARTIFACTS_DIR)/bootstrap\n")

if(WITH_TESTS)
  add_subdirectory(integration_tests)
endif()
