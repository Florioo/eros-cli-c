#include "eros.h"
#include "embedded_cli.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
#include "eros_cli.h"
#include "esp_timer.h"

EmbeddedCli *cli;
#include "esp_log.h"

int eros_cli_callback(eros_stream_t * eros, uint8_t *data, uint16_t length)
{

    // Process the cli command
    for (int i = 0; i < length; i++)
        embeddedCliReceiveChar(cli, data[i]);

    // Process the received data
    embeddedCliProcess(cli);
    return 0;
}


int eros_cli_get_float(EmbeddedCli *cli, const char *args, uint16_t pos, float * value)
{
    //This does not check for trailing characters

    const char * arg = embeddedCliGetToken(args, 1);
    
    if (arg == NULL) {
        eros_cli_printf(cli, "Missing argument");
        return 1;
    }

    int num_converted = sscanf(arg, "%f", value);

    if (num_converted != 1) {
        eros_cli_printf(cli, "Invalid float");
        return 2;
    }

    return 0;
}


void eros_cli_printf(EmbeddedCli *cli, const char *format, ...)
{
    eros_cli_config_t * cli_config = (eros_cli_config_t *) cli->appContext;
    static char buffer[128];
    
    // Set the first byte to 0 to indicate that is just data
    buffer[0] = 0x00;

    va_list args;
    va_start(args, format);
    int size = 1+vsprintf(buffer +1 , format, args);
    eros_transmit_inplace(cli_config->eros, cli_config->main_channel, (uint8_t *) buffer, size);
    va_end(args);
}

void writeChar(EmbeddedCli *embeddedCli, char c){
    eros_cli_config_t * cli_config = (eros_cli_config_t *) cli->appContext;
    static char buffer[32];
    static uint8_t bufferIndex = 0;

    buffer[bufferIndex++] = c;
    
    if (c == '\n' || c == '\r' || bufferIndex == sizeof(buffer) - 1 || 1) {
        eros_transmit(cli_config->eros, cli_config->aux_channel, (uint8_t *) buffer, bufferIndex);
        bufferIndex = 0;
        return;
    }

}
void writeString(EmbeddedCli *embeddedCli,const char *s){
    eros_cli_config_t * cli_config = (eros_cli_config_t *) cli->appContext;
    eros_transmit(cli_config->eros, cli_config->aux_channel, (uint8_t *) s, strlen(s));
}


int eros_cli_add_binding(EmbeddedCli *cli, char *command, char *description, bool hidden, void *context, int (*callback)(EmbeddedCli *cli, char *args, void *context))
{
    CliCommandBinding binding = {command, description, hidden, context, callback};
    return embeddedCliAddBinding(cli, binding);
}


// int eros_cli_init(eros_stream_t * eros, uint8_t main_channel, uint8_t aux_channel)
// {
//     eros_cli_config_t * cli_config = malloc(sizeof(eros_cli_config_t));
//     cli_config->main_channel = main_channel;
//     cli_config->aux_channel = aux_channel;
//     cli_config->eros = eros;

//     cli = embeddedCliInit(writeChar, writeString, postCommand, cli_config);
//     eros_attach_receive_callback(eros, main_channel, eros_cli_callback);

//     eros_cli_add_binding(cli, "get", "Get the value", false, NULL, getFloat);
//     eros_cli_add_binding(cli, "error", "Get the value", false, NULL, getError);

//     return 0;
// }

void postCommand(EmbeddedCli *cli, uint8_t result)
{
    result++;
    // Print timestamp microseconds
    eros_cli_config_t * cli_config = (eros_cli_config_t *) cli->appContext;
    eros_transmit(cli_config->eros, cli_config->main_channel, &result, 1);
}


EmbeddedCliConfig * eros_cli_init(eros_stream_t * eros, uint8_t main_channel, uint8_t aux_channel)
{
    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->enableAutoComplete = false;

    cli = embeddedCliNew(config);

    if (cli == NULL) {
        return 1;
    }
    
    eros_cli_config_t * eros_cli_config = malloc(sizeof(eros_cli_config_t));
    eros_cli_config->main_channel = main_channel;
    eros_cli_config->aux_channel = aux_channel;
    eros_cli_config->eros = eros;

    cli->writeChar = writeChar;
    cli->appContext = eros_cli_config;
    cli->writeString = writeString;
    cli->postCommand = postCommand;
 

    // Attach the callback on the specified channel
    eros_attach_receive_callback(eros, main_channel, eros_cli_callback);
    return cli;
}