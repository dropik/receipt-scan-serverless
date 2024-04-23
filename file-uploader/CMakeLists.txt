set(FUNCTION_NAME "file-uploader")
set(DEBUG_PORT_BASE 7000)

configure_file(include/config.h.in
  "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h")

find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3)

add_executable(${FUNCTION_NAME} "src/main.cpp")

target_include_directories(${FUNCTION_NAME} PUBLIC
  "include")

target_link_libraries(${FUNCTION_NAME} PUBLIC
  AWS::aws-lambda-runtime
  ${AWSSDK_LINK_LIBRARIES}
  aws-lambda-cpp-common)

set_target_properties(${FUNCTION_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/bin")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG)

  # If building for local development we need a bootstrap that loads AWS credentials
  # to environment
  set(BOOTSTRAP_PATH "${PROJECT_SOURCE_DIR}/build/bootstrap")
  file(WRITE "${BOOTSTRAP_PATH}"
    "#!/bin/bash\n"
    "set -e\n"
    "export AWS_ACCESS_KEY_ID=$(cat /root/.aws/credentials | grep aws_access_key_id | sed 's/aws_access_key_id = //g')\n"
    "export AWS_SECRET_ACCESS_KEY=$(cat /root/.aws/credentials | grep aws_secret_access_key | sed 's/aws_secret_access_key = //g')\n")

  # Check if we want to debug this function
  string(REPLACE ":" ";" DEBUG_FUNCS_LIST "${DEBUG_FUNCS}")
  list(FIND DEBUG_FUNCS_LIST ${PROJECT_NAME} FOUND_DEBUG_INDEX)
  if (FOUND_DEBUG_INDEX GREATER_EQUAL 0)
    # Calculate debug port
    math(EXPR DEBUG_PORT "${DEBUG_PORT_BASE} + ${FOUND_DEBUG_INDEX}")

    # Creating debug entry point
    file(APPEND "${BOOTSTRAP_PATH}"
      "myip=$(/usr/sbin/ip -4 addr show eth0 | grep -oP '(?<=inet\\s)\\d+(\\.\\d+)\{3\}')\n"
      "echo \"Waiting for debugger of function on \$\{myip\}:${DEBUG_PORT} ...\"\n"
      "qemu-aarch64 -g ${DEBUG_PORT} bin/${PROJECT_NAME}") 
    
    # Creating remote debug script
    file(WRITE "${PROJECT_SOURCE_DIR}/build/debug"
      "#!/bin/bash\n"
      "aarch64-al2-linux-gnu-gdb -ex \"set architecture aarch64\" ${PROJECT_SOURCE_DIR}/build/bin/${PROJECT_NAME}")
    add_custom_target(debug ALL
      COMMAND chmod a+x debug)

  else()
    # If not debugging just run executable
    file(APPEND "${BOOTSTRAP_PATH}"
      "bin/${PROJECT_NAME}")
  endif()

  add_custom_target(bootstrap ALL
    COMMAND chmod a+x bootstrap)

else()
  # For release just make a symlink to executable
  add_custom_target(bootstrap ALL
    COMMAND ln -s "bin/${PROJECT_NAME}" bootstrap)
endif()

