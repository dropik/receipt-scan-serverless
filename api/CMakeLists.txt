set(FUNCTION_NAME "api")
set(LAMBDA_ID "LApi")

find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3)

add_executable(${FUNCTION_NAME}
    src/main.cpp
    src/parameters/put_file.hpp
    src/responses/file.hpp
    src/api.hpp
    src/services/file_service.hpp
    src/api_errors.hpp
    src/identity.hpp
    src/services/user_service.hpp
    src/responses/receipt_detail.hpp
    src/model_types.hpp
    src/services/receipt_service.hpp
    src/responses/receipt_list_item.hpp
    src/services/category_service.hpp
    src/responses/category.hpp
    src/parameters/put_receipt.hpp
    src/factories.hpp
    src/s3_settings.hpp
    src/responses/user.hpp
    src/responses/user.cpp
    src/services/device_service.hpp
    src/parameters/put_device.hpp
    src/services/budget_service.hpp
    src/parameters/put_budget.hpp
    src/parameters/put_budget.cpp
    src/responses/budget.hpp
    src/responses/budget.cpp
    src/parameters/put_category.hpp
    src/parameters/put_category.cpp
    src/responses/category.cpp
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
    ${AWSSDK_LINK_LIBRARIES}
    aws-lambda-cpp-common
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
