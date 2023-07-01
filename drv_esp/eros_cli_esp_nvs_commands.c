#include "nvs_flash.h"
#include "nvs.h"
#include "eros_cli.h"
#include "embedded_cli.h"

int cmd_nvs_format(eros_cli_context_t *handle, char *args, void *context)
{
    // This command will erase the whole NVS partition
    esp_err_t err = nvs_flash_erase();
    if (err != ESP_OK) {
        eros_cli_printf(handle,"Error (%s) erasing NVS partition\n", esp_err_to_name(err));
        return 1;
    }
    eros_cli_printf(handle,"NVS partition erased\n");
    return 0;
}


void eros_cli_esp_register_nvs_commands( EmbeddedCli * cli)
{
    eros_cli_add_binding(cli, "nvs-format", "Format NVS", false, NULL, cmd_nvs_format);
}