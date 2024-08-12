set(FUNCTION_NAME "scanner")
set(LAMBDA_ID "LScanner")

add_executable(${FUNCTION_NAME}
    src/main.cpp
    src/handler.hpp
    src/utils.hpp
    src/utils.cpp
    src/models/bedrock_payload.hpp
    src/models/bedrock_response.hpp
    src/factories.hpp
    src/services/receipt_extractor.hpp
    src/services/categorizer.hpp
)

target_include_directories(${FUNCTION_NAME} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/repository/include"
    "${PROJECT_SOURCE_DIR}/lambda/include"
    "${PROJECT_SOURCE_DIR}/di/include"
)

target_link_libraries(${FUNCTION_NAME} PUBLIC
    AWS::aws-lambda-runtime
    aws-cpp-sdk-core
    aws-cpp-sdk-s3
    aws-cpp-sdk-textract
    aws-cpp-sdk-bedrock-runtime
    repository
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
