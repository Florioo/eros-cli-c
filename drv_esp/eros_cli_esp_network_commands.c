#include <stdlib.h>
#include <errno.h>

#include "eros_core.h"
#include "eros_cli.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"

static const char *TAG = "NET_CMD";

static int cmd_wifi_info(eros_cli_context_t *handle, char *args, void *context)
{
    wifi_config_t wifi_config;
    esp_err_t err = esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);

    if (err) {
        eros_cli_printf(handle, "Failed to get WiFi configuration. Error: %s\n", esp_err_to_name(err));
        return err;
    }
    eros_cli_printf(handle, "WiFi Configuration:\n");
    eros_cli_printf(handle, "SSID: %s\n", wifi_config.sta.ssid);
    eros_cli_printf(handle, "Password: %s\n", wifi_config.sta.password);
    return 0;
}

static int cmd_wifi_stats(eros_cli_context_t *handle, char *args, void *context)
{
    wifi_ap_record_t ap_info;
    esp_err_t err = esp_wifi_sta_get_ap_info(&ap_info);

    if (err) {
        eros_cli_printf(handle, "Failed to get AP information. Error: %s\n", esp_err_to_name(err));
        return err;
    }

    eros_cli_printf(handle, "Connected to AP:\n");
    eros_cli_printf(handle, "SSID: %s\n", ap_info.ssid);
    eros_cli_printf(handle, "BSSID: "MACSTR"\n", MAC2STR(ap_info.bssid));
    eros_cli_printf(handle, "Channel: %d\n", ap_info.primary);
    eros_cli_printf(handle, "RSSI: %d\n", ap_info.rssi);
    return 0;
}


static int cmd_ip_info(eros_cli_context_t *handle, char *args, void *context)
{
    esp_netif_t *netif = NULL;

    for (int i = 0; i < esp_netif_get_nr_of_ifs(); i++) {
        netif = esp_netif_next(netif);

        if (netif == NULL) {
            return 1;
        }
        esp_netif_ip_info_t ip_info;
        esp_err_t err = esp_netif_get_ip_info(netif, &ip_info);

        uint8_t mac[6] = {0};
        esp_netif_get_mac(netif, mac);
        const char *hostname;
        esp_netif_get_hostname(netif,&hostname);

        eros_cli_printf(handle, "Interface: %s\n", esp_netif_get_desc(netif));
        eros_cli_printf(handle, "- Hostname: %s\n", hostname);
        eros_cli_printf(handle, "- Is up: %d\n", esp_netif_is_netif_up(netif)); 

        if (err != ESP_OK) {
            eros_cli_printf(handle, "- Failed to get IP information. ");
            continue;
        }       
        eros_cli_printf(handle, "- MAC address: "MACSTR"\n", MAC2STR(mac));
        eros_cli_printf(handle, "- IP address: "IPSTR"\n", IP2STR(&ip_info.ip));
        eros_cli_printf(handle, "- Netmask: "IPSTR"\n", IP2STR(&ip_info.netmask));
        eros_cli_printf(handle, "- Gateway: "IPSTR"\n", IP2STR(&ip_info.gw));
    }
    return 0;
}

void eros_cli_esp_register_network_commands( EmbeddedCli * cli)
{
    eros_cli_add_binding(cli, "net-wifi-config",    "Get Wi-Fi configuraiton details",  false, NULL, cmd_wifi_info);
    eros_cli_add_binding(cli, "net-wifi-stats",     "Get Wi-Fi connection details",     false, NULL, cmd_wifi_stats);
    eros_cli_add_binding(cli, "net-ip-info",        "Get IP information",               false, NULL, cmd_ip_info);
}
