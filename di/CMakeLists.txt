add_library(di STATIC
    include/di/container.hpp
    include/di/scoped.hpp
    include/di/service_factory.hpp
    include/di/singleton.hpp
    include/di/transient.hpp
    include/di/parameter_manager.hpp
    src/parameter_manager.cpp
)

target_include_directories(di PUBLIC
    "include"
    "${PROJECT_SOURCE_DIR}/di/include"
)

target_link_libraries(di PUBLIC
    aws-cpp-sdk-core
    aws-cpp-sdk-ssm
    ${aws-lambda-cpp-common}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(di PRIVATE DEBUG)
endif()

if(WITH_TESTS)
    add_subdirectory(tests)
endif()
