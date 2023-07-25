#include "stdint.h"
#include "embedded_cli.h"
#include "eros_core.h"
#ifndef EROS_CLI_H
#define EROS_CLI_H

#define MAX_REPL_CLIENTS 4

typedef struct {
    uint8_t main_channel;
    uint8_t aux_channel;
    eros_stream_t * eros;
} eros_cli_client_config_t;

typedef struct {
    EmbeddedCli * cli;
    eros_cli_client_config_t clients[MAX_REPL_CLIENTS];
    uint8_t clients_count;
} eros_cli_context_t;


void eros_cli_printf(eros_cli_context_t *cli_context, const char *format, ...);
EmbeddedCli * eros_cli_init();
int eros_cli_add_binding(EmbeddedCli *cli, char *command, char *description, bool tokenize_args, void *context, int (*callback)(eros_cli_context_t *cli_context, char *args, void *context));
int eros_cli_get_float(eros_cli_context_t *cli_context, const char *args, uint16_t pos, float * value);
int eros_cli_get_str(eros_cli_context_t *cli_context, const char *args, uint16_t pos, const char ** value);
int eros_cli_get_int(eros_cli_context_t *cli_context, const char *args, uint16_t pos, int * value);
void eros_cli_add_repl_binding(EmbeddedCli * cli, eros_stream_t * eros, uint8_t main_channel, uint8_t aux_channel);
void eros_cli_add_machine_binding(EmbeddedCli * cli, eros_stream_t * eros, uint8_t channel);
void eros_cli_write(eros_cli_context_t *cli_context, const char * data, size_t size);

#endif // EROS_CLI_H