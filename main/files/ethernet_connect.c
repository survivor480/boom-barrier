#include "../header/ethernet_connect.h"
#include "../header/http_server.h"

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}

/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

    ESP_LOGI(TAG, "Ethernet Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");

    http_server_start();
}

void ethernet_task_start(void){
    // Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create instance(s) of esp-netif for SPI Ethernet(s)
    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
    esp_netif_config_t cfg_spi = {
        .base = &esp_netif_config,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    };
    esp_netif_t *eth_netif_spi[CONFIG_EXAMPLE_SPI_ETHERNETS_NUM] = { NULL };
    char if_key_str[10];
    char if_desc_str[10];
    char num_str[3];
    for (int i = 0; i < CONFIG_EXAMPLE_SPI_ETHERNETS_NUM; i++) {
        itoa(i, num_str, 10);
        strcat(strcpy(if_key_str, "ETH_SPI_"), num_str);
        strcat(strcpy(if_desc_str, "eth"), num_str);
        esp_netif_config.if_key = if_key_str;
        esp_netif_config.if_desc = if_desc_str;
        esp_netif_config.route_prio = 30 - i;
        eth_netif_spi[i] = esp_netif_new(&cfg_spi);
    };

    // Init MAC and PHY configs to default
    eth_mac_config_t mac_config_spi = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config_spi = ETH_PHY_DEFAULT_CONFIG();

    // Install GPIO ISR handler to be able to service SPI Eth modlues interrupts
    gpio_install_isr_service(0);

    // Init SPI bus
    spi_bus_config_t buscfg = {
        .miso_io_num = CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO,
        .mosi_io_num = CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO,
        .sclk_io_num = CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_EXAMPLE_ETH_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Init specific SPI Ethernet module configuration from Kconfig (CS GPIO, Interrupt GPIO, etc.)
    spi_eth_module_config_t spi_eth_module_config[CONFIG_EXAMPLE_SPI_ETHERNETS_NUM];
    INIT_SPI_ETH_MODULE_CONFIG(spi_eth_module_config, 0);
    #if CONFIG_EXAMPLE_SPI_ETHERNETS_NUM > 1
        INIT_SPI_ETH_MODULE_CONFIG(spi_eth_module_config, 1);
    #endif

    // Configure SPI interface and Ethernet driver for specific SPI module
    esp_eth_mac_t *mac_spi[CONFIG_EXAMPLE_SPI_ETHERNETS_NUM];
    esp_eth_phy_t *phy_spi[CONFIG_EXAMPLE_SPI_ETHERNETS_NUM];
    esp_eth_handle_t eth_handle_spi[CONFIG_EXAMPLE_SPI_ETHERNETS_NUM] = { NULL };
    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .queue_size = 20
    };
    for (int i = 0; i < CONFIG_EXAMPLE_SPI_ETHERNETS_NUM; i++) {
        // Set SPI module Chip Select GPIO
        spi_devcfg.spics_io_num = spi_eth_module_config[i].spi_cs_gpio;
        // Set remaining GPIO numbers and configuration used by the SPI module
        phy_config_spi.phy_addr = spi_eth_module_config[i].phy_addr;
        eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(CONFIG_EXAMPLE_ETH_SPI_HOST, &spi_devcfg);
        w5500_config.int_gpio_num = spi_eth_module_config[i].int_gpio;
        mac_spi[i] = esp_eth_mac_new_w5500(&w5500_config, &mac_config_spi);
        phy_spi[i] = esp_eth_phy_new_w5500(&phy_config_spi);
        esp_eth_config_t eth_config_spi = ETH_DEFAULT_CONFIG(mac_spi[i], phy_spi[i]);
        ESP_ERROR_CHECK(esp_eth_driver_install(&eth_config_spi, &eth_handle_spi[i]));

        /* The SPI Ethernet module might not have a burned factory MAC address, we cat to set it manually.
       02:00:00 is a Locally Administered OUI range so should not be used except when testing on a LAN under your control.
        */
        ESP_ERROR_CHECK(esp_eth_ioctl(eth_handle_spi[i], ETH_CMD_S_MAC_ADDR, (uint8_t[]) {
            0x02, 0x00, 0x00, 0x12, 0x34, 0x56 + i
        }));

        // attach Ethernet driver to TCP/IP stack
        ESP_ERROR_CHECK(esp_netif_attach(eth_netif_spi[i], esp_eth_new_netif_glue(eth_handle_spi[i])));
    }

    // Register user defined event handers
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    /* start Ethernet driver state machine */
    #if CONFIG_EXAMPLE_USE_SPI_ETHERNET
        for (int i = 0; i < CONFIG_EXAMPLE_SPI_ETHERNETS_NUM; i++) {
            ESP_ERROR_CHECK(esp_eth_start(eth_handle_spi[i]));
        }
    #endif // CONFIG_EXAMPLE_USE_SPI_ETHERNET
}



