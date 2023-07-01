idf_component_register(
    SRCS "drv_esp/eros_cli_esp_network_commands.c"
         "drv_esp/eros_cli_esp_system_commands.c"
         "drv_esp/eros_cli_esp_nvs_commands.c"
         "drv_esp/eros_cli_esp_log_commands.c"
         "eros_cli.c"
    INCLUDE_DIRS .
    REQUIRES eros-core
            embedded-cli
            esp_timer
            lwip
            esp_app_format
            esp_wifi
            nvs_flash
            spi_flash
)