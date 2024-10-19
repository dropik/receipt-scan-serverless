find_package(mariadb)

add_library(repository STATIC
    include/repository/client.hpp
    include/repository/models/common.hpp
    include/repository/models/category.hpp
    include/repository/models/receipt.hpp
    include/repository/models/receipt_item.hpp
    include/repository/configurations/common/base_property_configuration.hpp
    include/repository/configurations/common/base_repository_configuration.hpp
    include/repository/configurations/common/id_configuration.hpp
    include/repository/configurations/common/property_configuration.hpp
    include/repository/selector.hpp
    include/repository/configurations/common/table_configuration.hpp
    include/repository/configurations/repository_configuration.hpp
    include/repository/configurations/category_configuration.hpp
    include/repository/configurations/receipt_configuration.hpp
    include/repository/configurations/receipt_item_configuration.hpp
    include/repository/models/user.hpp
    include/repository/configurations/user_configuration.hpp
    include/repository/base_query.hpp
    src/base_query.cpp
    src/statement.cpp
    include/repository/connection_settings.hpp
    include/repository/factories.hpp
    include/repository/receipt_repository.hpp
    include/repository/configurations/registry.hpp
    include/repository/category_repository.hpp
    include/repository/configurations/common/version_configuration.hpp
    include/repository/models/budget.hpp
    include/repository/configurations/budget_configuration.hpp
    include/repository/exceptions.hpp
)

target_include_directories(repository PUBLIC
    "include"
    "${PROJECT_SOURCE_DIR}/di/include"
    "${PROJECT_SOURCE_DIR}/lambda/include"
)

target_link_libraries(repository PUBLIC
    lambda
    aws-cpp-sdk-core
    aws-cpp-sdk-ssm
    ${MARIADB_LIBS}
    ${aws-lambda-cpp-common}
    ${gssapi_krb5}
    ssl
    ${ZSTD_LIBRARIES}
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(repository PRIVATE DEBUG)
endif()

if(WITH_TESTS)
    add_subdirectory(integration_tests)
endif()
