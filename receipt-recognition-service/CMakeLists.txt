set(FUNCTION_NAME "receipt-recognition-service")
set(BOOTSTRAP "${FUNCTION_NAME}-bootstrap")

find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3 ssm textract)

add_executable(${FUNCTION_NAME}
  "src/main.cpp"
  "src/receipt-recognition-service.cpp")

target_include_directories(${FUNCTION_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
  "include"
  "${CMAKE_SYSROOT}/usr/local/include/mariadb")

target_link_libraries(${FUNCTION_NAME} PUBLIC
  AWS::aws-lambda-runtime
  ${AWSSDK_LINK_LIBRARIES}
  aws-lambda-cpp-common
  "${CMAKE_SYSROOT}/usr/local/lib/mariadb/libmariadbcpp.a"
  "${CMAKE_SYSROOT}/usr/local/lib/mariadb/libmariadb.a"
  ssl
  gssapi_krb5)

set_target_properties(${FUNCTION_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/bin")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${FUNCTION_NAME} PRIVATE DEBUG)

  # If building for local development we need a bootstrap that loads AWS credentials
  # to environment
  set(BOOTSTRAP_PATH "${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/bootstrap")
  file(WRITE "${BOOTSTRAP_PATH}"
    "#!/bin/bash\n"
    "set -e\n"
    "export AWS_ACCESS_KEY_ID=$(cat /root/.aws/credentials | grep aws_access_key_id | sed 's/aws_access_key_id = //g')\n"
    "export AWS_SECRET_ACCESS_KEY=$(cat /root/.aws/credentials | grep aws_secret_access_key | sed 's/aws_secret_access_key = //g')\n"
    "export DB_CONNECTION_STRING=\"jdbc:mariadb://192.168.8.136:3306/receipt-scan?user=receipt-scan&password=QydqG5Ge2u61\"\n"
    "bin/${FUNCTION_NAME}")

  add_custom_target(${BOOTSTRAP} ALL
    COMMAND chmod a+x ${BOOTSTRAP_PATH})

else()
  # For release just make a symlink to executable
  add_custom_target(${BOOTSTRAP} ALL
    COMMAND ln -s "bin/${FUNCTION_NAME}" bootstrap)
endif()

