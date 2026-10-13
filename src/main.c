#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "spi_dma_driver.h"

#define TEST_BUFFER_LEN 64

static void spi_dma_test_task(void *pvParameters) {
    uint8_t tx_data[TEST_BUFFER_LEN];
    uint8_t rx_data[TEST_BUFFER_LEN];

    // Fill transmit buffer with test pattern
    for (int i = 0; i < TEST_BUFFER_LEN; i++) {
        tx_data[i] = (uint8_t)(i & 0xFF);
        rx_data[i] = 0;
    }

    // Configure SPI DMA settings for RP2350
    spi_dma_config_t config = {
        .spi_instance = spi0,
        .baudrate_hz  = 1000000, // 1 MHz
        .pin_sck      = 2,       // GP2
        .pin_tx       = 3,       // GP3
        .pin_rx       = 4,       // GP4
        .pin_cs       = 5        // GP5
    };

    // Initialize driver
    if (spi_dma_init(&config) != SPI_DMA_OK) {
        printf("SPI DMA Initialization Failed!\n");
        vTaskDelete(NULL);
    }

    printf("SPI DMA Initialized Successfully. Starting transfer loop...\n");

    while (1) {
        // Perform non-blocking transfer with 100ms timeout
        spi_dma_status_t status = spi_dma_transfer_async(tx_data, rx_data, TEST_BUFFER_LEN, 100);

        if (status == SPI_DMA_OK) {
            printf("DMA Transfer completed successfully!\n");
        } else {
            printf("DMA Transfer error or timeout: %d\n", status);
        }

        // Delay 1 second between test transfers
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    stdio_init_all();
    
    // Create FreeRTOS test task
    xTaskCreate(spi_dma_test_task, "SPIDMATest", 256, NULL, 2, NULL);

    // Start FreeRTOS scheduler
    vTaskStartScheduler();

    while (1);
    return 0;
}