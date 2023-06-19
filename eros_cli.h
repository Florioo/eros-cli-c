#include "stdint.h"
#include "embedded_cli.h"
#include "eros.h"
#ifndef EROS_CLI_H
#define EROS_CLI_H

typedef struct eros_cli_config {
    uint8_t main_channel;
    uint8_t aux_channel;
    eros_stream_t * eros;
} eros_cli_config_t;


void eros_cli_printf(EmbeddedCli *cli, const char *format, ...);
EmbeddedCliConfig * eros_cli_init(eros_stream_t * eros, uint8_t main_channel, uint8_t aux_channel);
int eros_cli_add_binding(EmbeddedCli *cli, char *command, char *description, bool hidden, void *context, int (*callback)(EmbeddedCli *cli, char *args, void *context));
int eros_cli_get_float(EmbeddedCli *cli, const char *args, uint16_t pos, float * value);

#endif // EROS_CLI_H