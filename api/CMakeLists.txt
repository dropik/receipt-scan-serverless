set(FUNCTION_NAME "api")
set(LAMBDA_ID "LApi")

find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3)

add_executable(${FUNCTION_NAME}
    src/main.cpp
    src/models/upload_file_params.hpp
    src/models/file.hpp
    src/api.hpp
    src/services/file_service.hpp
    src/api_errors.hpp
    src/models/identity.hpp
    src/services/user_service.hpp
    src/models/receipt_detail.hpp
    src/models/model_types.hpp
    src/services/receipt_service.hpp
    src/models/receipt_list_item.hpp
    src/services/category_service.hpp
    src/models/category.hpp
    src/models/receipt_put_params.hpp
    src/factories.hpp
    src/models/s3_settings.hpp
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
