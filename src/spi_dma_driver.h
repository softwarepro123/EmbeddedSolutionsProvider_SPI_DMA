/**
 * @file spi_dma_driver.h
 * @brief Non-blocking SPI DMA Driver with FreeRTOS Synchronization
 * @author Eric L. Johnson | Embedded Solutions Provider, LLC
 * 
 * Hardware Target: RP2350 / Raspberry Pi Pico 2
 * Framework: Pico SDK + FreeRTOS
 */

#ifndef SPI_DMA_DRIVER_H
#define SPI_DMA_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "FreeRTOS.h"
#include "semphr.h"

#ifdef __cplusplus
extern "C" {
#endif

// Driver Return Status Codes
typedef enum {
    SPI_DMA_OK = 0,
    SPI_DMA_ERROR_INVALID_PARAM,
    SPI_DMA_ERROR_BUSY,
    SPI_DMA_ERROR_TIMEOUT,
    SPI_DMA_ERROR_HARDWARE
} spi_dma_status_t;

// Driver Configuration Structure
typedef struct {
    spi_inst_t *spi_instance;    // e.g., spi0 or spi1
    uint32_t baudrate_hz;        // Target SPI clock speed
    uint pin_sck;               // Serial Clock pin
    uint pin_tx;                // Master Out Slave In pin
    uint pin_rx;                // Master In Slave Out pin
    uint pin_cs;                // Chip Select pin
} spi_dma_config_t;

/**
 * @brief Initializes the SPI peripheral, GPIO pins, DMA channels, and RTOS semaphores.
 * @param config Pointer to the driver configuration structure.
 * @return SPI_DMA_OK on success, or appropriate error code.
 */
spi_dma_status_t spi_dma_init(const spi_dma_config_t *config);

/**
 * @brief Executes a non-blocking full-duplex SPI transfer using DMA.
 *        Yields the calling FreeRTOS task until transfer completion.
 * @param tx_buf Pointer to transmit data buffer (can be NULL if rx-only).
 * @param rx_buf Pointer to receive data buffer (can be NULL if tx-only).
 * @param len    Number of bytes to transfer.
 * @param timeout_ms Maximum time to wait for transfer completion in RTOS ticks.
 * @return SPI_DMA_OK on success, or SPI_DMA_ERROR_TIMEOUT on timeout.
 */
spi_dma_status_t spi_dma_transfer_async(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // SPI_DMA_DRIVER_H