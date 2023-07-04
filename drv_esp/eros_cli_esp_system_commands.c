#include "eros_cli_demo.h"
#include "eros_cli.h"
#include "eros_core.h"
#include <stdlib.h>
#include <errno.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_system.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_spi_flash.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_chip_info.h"
#include "esp_idf_version.h"
#include "esp_app_desc.h"

#define CHECK_RETVAL(call) { \
    int retval = (call); \
    if(retval != 0) return retval; \
}

static const char *TAG = "SYS_CMD";

static int cmd_ping(eros_cli_context_t *handle, char *args, void *context)
{
    eros_cli_printf(handle, "pong");
    return 0;
}

static int cmd_time(eros_cli_context_t *handle, char *args, void *context)
{
    eros_cli_printf(handle, "%lld", esp_timer_get_time());
    return 0;
}

void eros_restart_task(void *pvParameter)
{
    ESP_LOGW(TAG, "Restarting in 3 seconds...");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    esp_restart();
}

static int cmd_restart(eros_cli_context_t *handle, char *args, void *context)
{
    eros_cli_printf(handle, "Restarting in 3 seconds...");

    xTaskCreate(eros_restart_task, "restart_task", 4096, NULL, 5, NULL);    
    return 0;
}

static int cmd_taskStats(eros_cli_context_t *handle, char *args, void *context)
{
    // Get number of tasks
    UBaseType_t numTasks = uxTaskGetNumberOfTasks();

    // Allocate the array to hold the task status.
    TaskStatus_t *taskStatusArray = pvPortMalloc(numTasks * sizeof(TaskStatus_t));

    if (taskStatusArray == NULL) {
        eros_cli_printf(handle, "Failed to allocate memory for task status array.\n");
        return -1;
    }

    // Now get the task status
    numTasks = uxTaskGetSystemState(taskStatusArray, numTasks, NULL);

    // Get total run-time (in ticks) since vTaskStartScheduler was called
    uint32_t totalRunTime = 0;
    for (UBaseType_t i = 0; i < numTasks; i++) {
        totalRunTime += taskStatusArray[i].ulRunTimeCounter;
    }

    // Print header
    eros_cli_printf(handle, "Task Name         Status  Prio   HWM       Runtime     Perct\n");
    // For each task, print out the desired information
    for (UBaseType_t i = 0; i < numTasks; i++) {
        // Calculate the percentage of the total run time consumed by the task
        float taskRunTimePercentage = taskStatusArray[i].ulRunTimeCounter / (float)totalRunTime * 100.0f;
        // 
        eros_cli_printf(handle, 
            "%-22s"     //pcTaskName
            "%2c"      //eCurrentState
            "%6d"      //uxCurrentPriority
            "%6d"      //usStackHighWaterMark
            "%14d"     //ulRunTimeCounter
            "%10.2f%%\n",//taskRunTimePercentage
            taskStatusArray[i].pcTaskName,
            taskStatusArray[i].eCurrentState + '0', // convert state enum to a character
            taskStatusArray[i].uxCurrentPriority,
            taskStatusArray[i].usStackHighWaterMark,
            taskStatusArray[i].ulRunTimeCounter,
            taskRunTimePercentage);
    }

    // Don't forget to free the allocated memory
    vPortFree(taskStatusArray);

    return 0;
}

static int cmd_memStats(eros_cli_context_t *handle, char *args, void *context)
{
    // Print heap info for all capabilities
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);

    // Get memory stats
    multi_heap_info_t heap_info;
    heap_caps_get_info(&heap_info, MALLOC_CAP_INTERNAL);

    // Print memory stats
    eros_cli_printf(handle, "Heap Summary for MALLOC_CAP_INTERNAL:\n");
    eros_cli_printf(handle, "- Total free size: %d bytes\n", heap_info.total_free_bytes);
    eros_cli_printf(handle, "- Total allocated size: %d bytes\n", heap_info.total_allocated_bytes);
    eros_cli_printf(handle, "- Largest free block: %d bytes\n", heap_info.largest_free_block);
    eros_cli_printf(handle, "- Minimum ever free size: %d bytes\n", heap_info.minimum_free_bytes);
    eros_cli_printf(handle, "- Number of free blocks: %d\n", heap_info.free_blocks);
    eros_cli_printf(handle, "- Number of allocated blocks: %d\n", heap_info.allocated_blocks);
    eros_cli_printf(handle, "- Total blocks: %d\n", heap_info.total_blocks);

    return 0;
}

static int cmd_systemInfo(eros_cli_context_t *handle, char *args, void *context)
{
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    eros_cli_printf(handle, "ESP Chip Information:\n");
    eros_cli_printf(handle, "- Model: %d\n", chip_info.model);
    eros_cli_printf(handle, "- Cores: %d\n", chip_info.cores);
    eros_cli_printf(handle, "- Features: WiFi%s%s\n", 
        (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", 
        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    eros_cli_printf(handle, "- Revision number: %d\n", chip_info.revision);
    return 0;
}

static int cmd_firmwareInfo(eros_cli_context_t *handle, char *args, void *context)
{
    eros_cli_printf(handle, "Firmware Information:\n");
    const char * version = esp_get_idf_version();
    eros_cli_printf(handle, "- IDF Version: %s\n", version);
    esp_app_desc_t * app_desc = esp_app_get_description();
    eros_cli_printf(handle, "- App Version: %s\n", app_desc->version);
    eros_cli_printf(handle, "- Project Name: %s\n", app_desc->project_name);
    eros_cli_printf(handle, "- Project Version: %s\n", app_desc->version);
    eros_cli_printf(handle, "- Compile Date: %s\n", app_desc->date);
    eros_cli_printf(handle, "- Compile Time: %s\n", app_desc->time);
    eros_cli_printf(handle, "- IDF Version: %s\n", app_desc->idf_ver);
    char sha_str[65];
    for (int i = 0; i < 32; ++i) {
        sprintf(&sha_str[i*2], "%02x", (unsigned int)app_desc->app_elf_sha256[i]);
    }
    eros_cli_printf(handle, "- SHA256: %s\n", sha_str);
    
    return 0;
}


void eros_cli_esp_register_system_commands( EmbeddedCli * cli)
{
    eros_cli_add_binding(cli, "sys-ping", "Responds with pong", false, NULL, cmd_ping);
    eros_cli_add_binding(cli, "sys-time", "Get time in microseconds", false, NULL, cmd_time);
    eros_cli_add_binding(cli, "sys-restart", "Restart the device", false, NULL, cmd_restart);
    eros_cli_add_binding(cli, "sys-tasks", "Get task stats", false, NULL, cmd_taskStats);
    eros_cli_add_binding(cli, "sys-mem", "Get memory stats", false, NULL, cmd_memStats);
    eros_cli_add_binding(cli, "sys-chip", "Get detailed system information", false, NULL, cmd_systemInfo);
    eros_cli_add_binding(cli, "sys-firmware", "Get firmware information", false, NULL, cmd_firmwareInfo);

}
