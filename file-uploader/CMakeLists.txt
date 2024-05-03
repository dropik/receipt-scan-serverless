set(FUNCTION_NAME "file-uploader")
set(LAMBDA_ID "FileUploader")

find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3)

add_executable(${FUNCTION_NAME} "src/main.cpp")

target_include_directories(${FUNCTION_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
  "include")

target_link_libraries(${FUNCTION_NAME} PUBLIC
  AWS::aws-lambda-runtime
  ${AWSSDK_LINK_LIBRARIES}
  aws-lambda-cpp-common)

set_target_properties(${FUNCTION_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/bin")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${FUNCTION_NAME} PRIVATE DEBUG)
endif()

set(BOOTSTRAP "${FUNCTION_NAME}-bootstrap")
add_custom_target(${BOOTSTRAP} ALL
  COMMAND ln -sf "bin/${FUNCTION_NAME}" "${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/bootstrap")

set(MAKEFILE_PATH "${PROJECT_SOURCE_DIR}/out/${FUNCTION_NAME}/Makefile")
file(WRITE "${MAKEFILE_PATH}"
  "build-${LAMBDA_ID}:\n"
  "\tcp -r ${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/* $(ARTIFACTS_DIR)")
