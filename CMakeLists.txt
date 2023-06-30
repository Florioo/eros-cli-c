idf_component_register(
    SRCS "eros_cli.c"  "eros_cli_demo.c"
    INCLUDE_DIRS .
    REQUIRES eros-core embedded-cli esp_timer
)