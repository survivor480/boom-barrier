// Using W5500 for thernet services

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "driver/spi_master.h"


#define CONFIG_EXAMPLE_ETH_SPI_CS                           15
#define CONFIG_EXAMPLE_ETH_SPI_INT                          4
#define CONFIG_EXAMPLE_ETH_SPI_PHY_RST                      -1
#define CONFIG_EXAMPLE_ETH_SPI_PHY_ADDR                     1



#define CONFIG_EXAMPLE_ETH_SPI_MISO_GPIO                    12
#define CONFIG_EXAMPLE_ETH_SPI_MOSI_GPIO                    13
#define CONFIG_EXAMPLE_ETH_SPI_SCLK_GPIO                    14


#define CONFIG_EXAMPLE_SPI_ETHERNETS_NUM                    1
#define CONFIG_EXAMPLE_ETH_SPI_HOST                         1
#define CONFIG_EXAMPLE_ETH_SPI_CLOCK_MHZ                    12
#define CONFIG_EXAMPLE_USE_SPI_ETHERNET                     1
#define CONFIG_EXAMPLE_USE_W5500                            1



static const char *TAG = "ethernet";

#define INIT_SPI_ETH_MODULE_CONFIG(eth_module_config, num)                      \
    do {                                                                        \
        eth_module_config[num].spi_cs_gpio = CONFIG_EXAMPLE_ETH_SPI_CS;         \
        eth_module_config[num].int_gpio = CONFIG_EXAMPLE_ETH_SPI_INT;           \
        eth_module_config[num].int_gpio = CONFIG_EXAMPLE_ETH_SPI_PHY_RST;       \
        eth_module_config[num].phy_addr = CONFIG_EXAMPLE_ETH_SPI_PHY_ADDR;      \
    } while(0)

typedef struct {
    uint8_t spi_cs_gpio;
    uint8_t int_gpio;
    int8_t phy_reset_gpio;
    uint8_t phy_addr;
} spi_eth_module_config_t;

// Event Handler for ethernet event
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);


/** Event handler for IP_EVENT_ETH_GOT_IP */
static void got_ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

// Start the ethernet program
void ethernet_task_start();


