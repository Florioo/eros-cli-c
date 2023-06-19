#include "eros.h"
#include "embedded_cli.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
EmbeddedCli *cli;

#define CLI_BUFFER_SIZE 164
#define CLI_RX_BUFFER_SIZE 16
#define CLI_CMD_BUFFER_SIZE 32
#define CLI_HISTORY_SIZE 32
#define CLI_BINDING_COUNT 3

CLI_UINT cliBuffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

// Process the cli command
int eros_cli_callback(eros_stream_t * eros, uint8_t *data, uint16_t length)
{
    // Process the cli command
    for (int i = 0; i < length; i++)
        embeddedCliReceiveChar(cli, data[i]);
    
    embeddedCliProcess(cli);

    return 0;
}

void writeChar(EmbeddedCli *embeddedCli, char c){
    eros_stream_t * eros = (eros_stream_t *) embeddedCli->appContext;
    static char buffer[32];
    static uint8_t bufferIndex = 0;

    buffer[bufferIndex++] = c;
    
    if (c == '\n' || c == '\r' || bufferIndex == sizeof(buffer) - 1|| 1) {
        eros_transmit(eros, 6, (uint8_t *) buffer, bufferIndex);
        bufferIndex = 0;
        return;
    }

}
void writeString(EmbeddedCli *embeddedCli,const char *s){
    eros_stream_t * eros = (eros_stream_t *) embeddedCli->appContext;
    eros_transmit(eros, 6, (uint8_t *) s, strlen(s));
}


void onLed(EmbeddedCli *cli, char *args, void *context) {
    // printf("WQEQWEQWE");
    
}

int eros_cli_init(eros_stream_t * eros, uint8_t channel)
{
    
    // EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    // config->cliBuffer = cliBuffer;
    // config->cliBufferSize = CLI_BUFFER_SIZE;
    // config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    // config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    // config->historyBufferSize = CLI_HISTORY_SIZE;
    // config->maxBindingCount = CLI_BINDING_COUNT;
    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->enableAutoComplete = false;

    cli = embeddedCliNew(config);
    // cli = embeddedCliNewDefault();
    // if (cli == NULL) {
    //     printf("ERROR: Cli was not created. Check sizes!");
    //     return 1;
    // }
    // printf("Cli was created %p\n", cli);
    cli->writeChar = writeChar;
    cli->appContext = eros;
    cli->writeString = writeString;
    // EmbeddedCli *cli = embeddedCliNewDefault();

    if (cli == NULL) {
        printf("Cli was not created. Check sizes!");
        return 1;
    }
    CliCommandBinding binding = { "get-led", "Get led status", false, NULL, onLed};
    embeddedCliAddBinding(cli, binding);

    // Attach the callback on the specified channel
    eros_attach_receive_callback(eros, channel, eros_cli_callback);
    return 0;
}