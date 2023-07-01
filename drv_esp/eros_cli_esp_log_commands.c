#include "esp_log.h"
#include "eros_cli.h"
#include "embedded_cli.h"

int cmd_get_log_level(eros_cli_context_t *handle, char *args, void *context)
{
    const char * tag;
    if (eros_cli_get_str(handle, args, 1, &tag)) {
        return 1;
    }
    int log_level = esp_log_level_get(tag);

    eros_cli_printf(handle,"Log level for tag '%s' is '%d'\n", tag, log_level);
    return 0;
}

int cmd_set_log_level(eros_cli_context_t *handle, char *args, void *context)
{
    const char * tag;
    if (eros_cli_get_str(handle, args, 1, &tag)) {
        return 1;
    }

    int log_level = 0;
    if (eros_cli_get_int(handle, args, 2, &log_level)) {
        return 1;
    }

    esp_log_level_set(tag, log_level);
    eros_cli_printf(handle,"Log level for tag '%s' set to '%d'\n", tag, log_level);
    return 0;
}

void eros_cli_esp_register_log_commands(EmbeddedCli *cli)
{
    eros_cli_add_binding(cli, "log-set-level", "Get current log level", true, NULL, cmd_set_log_level);
    eros_cli_add_binding(cli, "log-get-level", "Set a new logcmd_get_log_level level", true, NULL, cmd_get_log_level);
    // eros_cli_add_binding(cli, "log-test-level", "Test", true, NULL, cmd_test);
}