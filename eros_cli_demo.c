#include "eros_cli_demo.h"
#include "eros_cli.h"
#include "eros_core.h"
#include <stdlib.h>
#include <errno.h>

#define CHECK_RETVAL(call) { \
    int retval = (call); \
    if(retval != 0) return retval; \
}


int getError(eros_cli_context_t *handle, char *args, void *context)
{
    if(1){
        eros_cli_printf(handle, "An error is forced upon you");
        return 1;
    }

    return 0;
}

float value = 3.1486;

int getFloat(eros_cli_context_t *handle, char *args, void *context)
{
    eros_cli_printf(handle, "%f", value);
    return 0;
}

int setFloat(eros_cli_context_t *handle, char *args, void *context)
{
    CHECK_RETVAL(eros_cli_get_float(handle, args, 1, &value));
    return 0;
}

int ping(eros_cli_context_t *handle, char *args, void *context)
{
    eros_cli_printf(handle, "pong");
    return 0;
}

int time(eros_cli_context_t *handle, char *args, void *context)
{
    eros_cli_printf(handle, "%lld", esp_timer_get_time());
    return 0;
}
void eros_cli_demo_add_bindings( EmbeddedCli * cli)
{
    eros_cli_add_binding(cli, "get-float", "Get a float valiue", false, NULL, getFloat);
    eros_cli_add_binding(cli, "get-error", "Get error", false, NULL, getError);
    eros_cli_add_binding(cli, "set-float", "Set a float value", false, NULL, setFloat);
    eros_cli_add_binding(cli, "ping", "Ping", false, NULL, ping);
    eros_cli_add_binding(cli, "time", "Get time", false, NULL, time);
}
