#include "eros_cli_demo.h"
#include "eros_cli.h"
#include "eros.h"
#include <stdlib.h>
#include <errno.h>
#include <esp_timer.h>

#define CHECK_RETVAL(call) { \
    int retval = (call); \
    if(retval != 0) return retval; \
}


int getError(EmbeddedCli *cli, char *args, void *context)
{
    if(1){
        eros_cli_printf(cli, "An error is forced upon you");
        return 1;
    }

    return 0;
}

float value = 3.1486;

int getFloat(EmbeddedCli *cli, char *args, void *context)
{
    eros_cli_printf(cli, "%f", value);
    return 0;
}

int setFloat(EmbeddedCli *cli, char *args, void *context)
{
    CHECK_RETVAL(eros_cli_get_float(cli, args, 1, &value));
    return 0;
}

int ping(EmbeddedCli *cli, char *args, void *context)
{
    eros_cli_printf(cli, "pong");
    return 0;
}

void eros_cli_demo_add_bindings( EmbeddedCli * cli)
{
    eros_cli_add_binding(cli, "get-float", "Get a float valiue", false, NULL, getFloat);
    eros_cli_add_binding(cli, "get-error", "Get error", false, NULL, getError);
    eros_cli_add_binding(cli, "set-float", "Set a float value", false, NULL, setFloat);
    eros_cli_add_binding(cli, "ping", "Ping", false, NULL, ping);
}
    