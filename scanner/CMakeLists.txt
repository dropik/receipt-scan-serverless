set(FUNCTION_NAME "scanner")
set(LAMBDA_ID "LScanner")

find_package(ZLIB REQUIRED)
find_package(aws-lambda-runtime REQUIRED)
find_package(AWSSDK COMPONENTS core s3 ssm textract bedrock-runtime)

set(Boost_USE_STATIC_LIBS ON) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME ON) 
find_package(Boost COMPONENTS locale) 

add_executable(${FUNCTION_NAME}
        src/main.cpp
        src/handler.hpp
        src/handler.cpp
        src/repository/repository.cpp
        src/utils.hpp
        src/utils.cpp
        src/models/bedrock_payload.hpp
        src/models/bedrock_response.hpp)

target_include_directories(${FUNCTION_NAME} PUBLIC
  "${PROJECT_SOURCE_DIR}/include"
  "include"
  "${CMAKE_SYSROOT}/usr/local/include/mariadb"
  ${Boost_INCLUDE_DIRS})

target_link_libraries(${FUNCTION_NAME} PUBLIC
  AWS::aws-lambda-runtime
  ${AWSSDK_LINK_LIBRARIES}
  aws-lambda-cpp-common
  "${CMAKE_SYSROOT}/usr/local/lib/mariadb/libmariadbcpp.a"
  "${CMAKE_SYSROOT}/usr/local/lib/mariadb/libmariadb.a"
  ${Boost_LIBRARIES}
  ssl
  gssapi_krb5)

set_target_properties(${FUNCTION_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/bin")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${FUNCTION_NAME} PRIVATE DEBUG)
endif()

set(BOOTSTRAP "${FUNCTION_NAME}-bootstrap")
add_custom_target(${BOOTSTRAP} ALL
  COMMAND ln -sf "bin/${FUNCTION_NAME}" "${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/bootstrap")
  
add_dependencies(${BOOTSTRAP} ${FUNCTION_NAME})

set(MAKEFILE_PATH "${PROJECT_SOURCE_DIR}/out/${FUNCTION_NAME}/Makefile")
file(WRITE "${MAKEFILE_PATH}"
  "build-${LAMBDA_ID}:\n"
  "\tcp -r ${PROJECT_SOURCE_DIR}/build/${FUNCTION_NAME}/* $(ARTIFACTS_DIR)")
