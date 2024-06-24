add_executable(di_tests_run
    di_test.cpp
    ../include/di/service_factory.hpp
    ../include/di/transient.hpp
    ../include/di/singleton.hpp
    ../include/di/scoped.hpp
    ../include/di/container.hpp
)

target_include_directories(di_tests_run PUBLIC
    ${PROJECT_SOURCE_DIR}/di/include
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR})

target_link_libraries(di_tests_run
    gtest
    gtest_main)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(di_tests_run PRIVATE DEBUG)
endif()
