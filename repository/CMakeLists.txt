find_package(ZLIB REQUIRED)
find_package(AWSSDK COMPONENTS core ssm)

add_library(repository STATIC
    include/repository/client.hpp
    src/client.cpp
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
)

target_include_directories(repository PUBLIC
        "include")

target_link_libraries(repository PUBLIC
        ${AWSSDK_LINK_LIBRARIES}
        "${CMAKE_SYSROOT}/usr/local/lib/mariadb/libmariadbcpp.a"
        "${CMAKE_SYSROOT}/usr/local/lib/mariadb/libmariadb.a"
        aws-lambda-cpp-common
        gssapi_krb5
        ssl)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(repository PRIVATE DEBUG)
endif()