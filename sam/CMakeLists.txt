add_custom_command(
    OUTPUT template.yaml
    COMMAND cp ${PROJECT_SOURCE_DIR}/sam/template.yaml ${PROJECT_BINARY_DIR}/
)

add_custom_command(
    OUTPUT samconfig.toml
    COMMAND cp ${PROJECT_SOURCE_DIR}/sam/samconfig.toml ${PROJECT_BINARY_DIR}/
)

add_custom_command(
    OUTPUT sync
    COMMAND cp ${PROJECT_SOURCE_DIR}/sam/sync ${PROJECT_BINARY_DIR}/
    COMMAND chmod +x ${PROJECT_BINARY_DIR}/sync
)

add_custom_command(
    OUTPUT deploy
    COMMAND cp ${PROJECT_SOURCE_DIR}/sam/deploy ${PROJECT_BINARY_DIR}/
    COMMAND chmod +x ${PROJECT_BINARY_DIR}/deploy
)

add_custom_target(
    sam ALL
    DEPENDS template.yaml samconfig.toml sync deploy)