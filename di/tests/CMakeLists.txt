add_executable(di_tests
    di_test.cpp
    ../include/di/service_factory.hpp
    ../include/di/transient.hpp
    ../include/di/singleton.hpp
    ../include/di/scoped.hpp
    ../include/di/container.hpp
    ../include/di/parameter_manager.hpp
)

target_include_directories(di_tests PUBLIC
    ${PROJECT_SOURCE_DIR}/di/include
    ${gtest_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR})

target_link_libraries(di_tests
    gtest
    gtest_main
    di
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(di_tests PRIVATE DEBUG)
endif()
