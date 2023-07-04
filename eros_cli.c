#include "eros_core.h"
#include "embedded_cli.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"
#include "esp_timer.h"
#include "errno.h"
#include "esp_log.h"

#include "eros_cli.h"


int eros_cli_get_float(eros_cli_context_t *cli_context, const char *args, uint16_t pos, float * value)
{
    //This does not check for trailing characters

    const char * arg = embeddedCliGetToken(args, pos);
    
    if (arg == NULL) {
        eros_cli_printf(cli_context, "Missing argument on position %d", pos);
        return 1;
    }

    int num_converted = sscanf(arg, "%f", value);

    if (num_converted != 1) {
        eros_cli_printf(cli_context, "float: Invalid float");
        return 2;
    }

    return 0;
}
int eros_cli_get_int(eros_cli_context_t *cli_context, const char *args, uint16_t pos, int * value)
{
    const char * arg = embeddedCliGetToken(args, pos);
    
    if (arg == NULL) {
        eros_cli_printf(cli_context, "int: Missing argument on position %d", pos);
        return 1;
    }

    int num_converted = sscanf(arg, "%d", value);

    if (num_converted != 1) {
        eros_cli_printf(cli_context, "Invalid int");
        return 2;
    }

    return 0;
}



int eros_cli_get_str(eros_cli_context_t *cli_context, const char *args, uint16_t pos, const char ** value)
{
    //This does not check for trailing characters

    const char * arg = embeddedCliGetToken(args, pos);
    
    if (arg == NULL) {
        eros_cli_printf(cli_context, "str: Missing argument on position %d", pos);
        return 1;
    }

    *value = arg;

    return 0;
}


void eros_cli_printf(eros_cli_context_t *cli_context, const char *format, ...)
{
    static char buffer[128];
    
    // Set the first byte to 0 to indicate that is just data
    buffer[0] = 0x00;

    va_list args;
    va_start(args, format);
    int size = 1+vsprintf(buffer +1 , format, args);

    for (int i = 0; i < cli_context->clients_count; i++)
        eros_transmit(cli_context->clients[i].eros, cli_context->clients[i].main_channel, (uint8_t *) buffer, size);
    
    va_end(args);
}

void eros_cli_repl_write_char(EmbeddedCli *cli, char c)
{
    eros_cli_context_t * repl_context = (eros_cli_context_t *) cli->appContext;

    //Transmit the character to each eros repl client
    for (int i = 0; i < repl_context->clients_count; i++)
        eros_transmit(repl_context->clients[i].eros, repl_context->clients[i].aux_channel, (uint8_t *) &c, 1);
}

void eros_cli_repl_write_string(EmbeddedCli *cli, const char *string)
{
    eros_cli_context_t * repl_context = (eros_cli_context_t *) cli->appContext;

    //Transmit the string to each eros repl client
    for (int i = 0; i < repl_context->clients_count; i++)
        eros_transmit(repl_context->clients[i].eros, repl_context->clients[i].aux_channel, (uint8_t *) string, strlen(string));

}


int eros_cli_add_binding(EmbeddedCli *cli, char *command, char *description, bool tokenize_args, void *context, int (*callback)(eros_cli_context_t *cli_context, char *args, void *context))
{
    CliCommandBinding binding = {command, description, tokenize_args, context, (void*)callback};
    return embeddedCliAddBinding(cli, binding);
}


// int eros_cli_init(eros_stream_t * eros, uint8_t main_channel, uint8_t aux_channel)
// {
//     eros_cli_context_t * cli_context = malloc(sizeof(eros_cli_context_t));
//     cli_context->main_channel = main_channel;
//     cli_context->aux_channel = aux_channel;
//     cli_context->eros = eros;

//     cli = embeddedCliInit(eros_cli_repl_write_char, writeString, postCommand, cli_context);
//     eros_attach_receive_callback(eros, main_channel, eros_cli_callback);

//     eros_cli_add_binding(cli, "get", "Get the value", false, NULL, getFloat);
//     eros_cli_add_binding(cli, "error", "Get the value", false, NULL, getError);

//     return 0;
// }

void postCommand(void *handle, uint8_t result)
{
    // Post the result to the eros stream
    eros_cli_context_t * cli_context = (eros_cli_context_t *) handle;
    result++;
    
    for (int i = 0; i < cli_context->clients_count; i++)
        eros_transmit(cli_context->clients[i].eros, cli_context->clients[i].main_channel, &result, 1);

}


/**
 * @brief Initialize a new EmbeddedCli instance
 * This will create the buffers required for the cli to work
 * Commands need to be registers to each instance separately
*/
EmbeddedCli * eros_cli_init()
{
    // Create the config for the cli
    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->enableAutoComplete = false;
    config->maxBindingCount = 32;
    // Create the cli instance
    EmbeddedCli *cli = embeddedCliNew(config);

    if (cli == NULL) {
        return NULL;
    }

    // Create the config for eros
    eros_cli_context_t * eros_repl_context = calloc(1,sizeof(eros_cli_context_t));
    

    // Method to write a character to the output in repl mode
    cli->appContext = eros_repl_context;
    cli->writeChar = eros_cli_repl_write_char;
    cli->writeString = eros_cli_repl_write_string;
    cli->postCommand = postCommand;
 
    // Attach the callback on the specified channel
    return cli;
}

/**
 * @brief Callback for the eros repl channel
 * This will receive the data from the eros stream and process it
 * @param eros Pointer to the eros stream instance
 * @param data Pointer to the data received
 * @param length Length of the data received
 * @param context Pointer to the eros_cli_context_t instance 
*/
int eros_cli_repl_callback(eros_stream_t * eros, uint8_t *data, uint16_t length, void * context)
{
    eros_cli_context_t *cli_context = (eros_cli_context_t *) context;

    // Process the cli command
    for (int i = 0; i < length; i++)
        embeddedCliReceiveChar(cli_context->cli, data[i]);
    
    // Process the received data
    embeddedCliProcess(cli_context->cli, context);
    return 0;
}

int eros_cli_machine_callback(eros_stream_t * eros, uint8_t *data, uint16_t length, void * context)
{

    eros_cli_context_t *cli_context = (eros_cli_context_t *) context;

    // Process the received data
    if(embeddedCliParseDirectCommand(cli_context->cli, data, length, context)){

        // Fail to find the command, send an error
        // Send the error to the eros stream
        eros_cli_printf(cli_context, "Command not found\n");
        postCommand(context, 1);
    }
    return 0;
}

/**
 * @brief Add a new repl binding to the cli
 * This will register the eros stream to the cli and attach the callback to the specified channel
 * The repl is shared accross all the eros streams , data will be duplicated to each stream
 * @param cli Pointer to the cli instance
 * @param eros Pointer to the eros stream instance
 * @param main_channel Main channel to use for the repl
 * @param aux_channel Aux channel to use for the repl
*/
void eros_cli_add_repl_binding(EmbeddedCli * cli, eros_stream_t * eros, uint8_t main_channel, uint8_t aux_channel)
{
    // Register the client to the repl list
    eros_cli_context_t * eros_cli_context = (eros_cli_context_t *) (cli->appContext);
    // printf("eros_cli_context handle = %p\n", eros_cli_context);

    eros_cli_context->clients[eros_cli_context->clients_count].main_channel = main_channel;
    eros_cli_context->clients[eros_cli_context->clients_count].aux_channel = aux_channel;
    eros_cli_context->clients[eros_cli_context->clients_count].eros = eros;
    eros_cli_context->clients_count++;
    eros_cli_context->cli = cli;

    // Attach the callback on the specified channel
    eros_attach_receive_callback(eros, main_channel, eros_cli_repl_callback, eros_cli_context);
} 

/**
 * @brief Add a new machine binding to the cli
 * This will register the eros stream to the cli and attach the callback to the specified channel
 * The machine interface uses the server-client model, which means is a 1 to 1 connection
 * @param cli Pointer to the cli instance
 * @param eros Pointer to the eros stream instance
 * @param channel Channel to use
*/
void eros_cli_add_machine_binding(EmbeddedCli * cli, eros_stream_t * eros, uint8_t channel)
{
    // Register the client to the list
    eros_cli_context_t * eros_cli_context = calloc(1,sizeof(eros_cli_context_t));
    eros_cli_context->clients[0].main_channel = channel;
    eros_cli_context->clients[0].aux_channel = 0;
    eros_cli_context->clients[0].eros = eros;
    eros_cli_context->clients_count = 1;
    eros_cli_context->cli = cli;

    // Attach the callback on the specified channel
    eros_attach_receive_callback(eros, channel, eros_cli_machine_callback, eros_cli_context);
} 
