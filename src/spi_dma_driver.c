/**
 * @file spi_dma_driver.c
 * @brief Implementation of Non-blocking SPI DMA Driver with FreeRTOS
 * @author Eric L. Johnson | Embedded Solutions Provider, LLC
 */

#include "spi_dma_driver.h"

// Private Driver Control Block
static struct {
    spi_inst_t *spi;
    int tx_dma_chan;
    int rx_dma_chan;
    SemaphoreHandle_t transfer_complete_sem;
    StaticSemaphore_t sem_buffer;
    bool is_initialized;
} g_spi_dma_ctx = {0};

// DMA Interrupt Service Routine
static void __isr spi_dma_isr(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Clear interrupt flag for RX DMA channel
    dma_hw->ints0 = (1u << g_spi_dma_ctx.rx_dma_chan);

    // Unblock the task waiting on transfer completion
    xSemaphoreGiveFromISR(g_spi_dma_ctx.transfer_complete_sem, &xHigherPriorityTaskWoken);

    // Context switch if needed
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

spi_dma_status_t spi_dma_init(const spi_dma_config_t *config) {
    if (!config || !config->spi_instance) {
        return SPI_DMA_ERROR_INVALID_PARAM;
    }

    g_spi_dma_ctx.spi = config->spi_instance;

    // 1. Initialize SPI HW & GPIOs
    spi_init(g_spi_dma_ctx.spi, config->baudrate_hz);
    gpio_set_function(config->pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(config->pin_tx,  GPIO_FUNC_SPI);
    gpio_set_function(config->pin_rx,  GPIO_FUNC_SPI);
    gpio_set_function(config->pin_cs,  GPIO_FUNC_SIO);
    gpio_set_dir(config->pin_cs, GPIO_OUT);
    gpio_put(config->pin_cs, 1); // CS High (Inactive)

    // 2. Create Static Binary Semaphore for RTOS Task Synchronization
    g_spi_dma_ctx.transfer_complete_sem = xSemaphoreCreateBinaryStatic(&g_spi_dma_ctx.sem_buffer);
    if (!g_spi_dma_ctx.transfer_complete_sem) {
        return SPI_DMA_ERROR_HARDWARE;
    }

    // 3. Claim Unused DMA Channels
    g_spi_dma_ctx.tx_dma_chan = dma_claim_unused_channel(true);
    g_spi_dma_ctx.rx_dma_chan = dma_claim_unused_channel(true);

    // 4. Configure DMA IRQ on RX Channel Completion
    dma_channel_set_irq0_enabled(g_spi_dma_ctx.rx_dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, spi_dma_isr);
    irq_set_enabled(DMA_IRQ_0, true);

    g_spi_dma_ctx.is_initialized = true;
    return SPI_DMA_OK;
}

spi_dma_status_t spi_dma_transfer_async(const uint8_t *tx_buf, uint8_t *rx_buf, size_t len, uint32_t timeout_ms) {
    if (!g_spi_dma_ctx.is_initialized || len == 0) {
        return SPI_DMA_ERROR_INVALID_PARAM;
    }

    static uint8_t dummy_tx = 0xFF;
    static uint8_t dummy_rx = 0x00;

    // Configure RX DMA Config
    dma_channel_config rx_cfg = dma_channel_get_default_config(g_spi_dma_ctx.rx_dma_chan);
    channel_config_set_transfer_data_size(&rx_cfg, DMA_SIZE_8);
    channel_config_set_dreq(&rx_cfg, spi_get_dreq(g_spi_dma_ctx.spi, false));
    channel_config_set_read_increment(&rx_cfg, false);
    channel_config_set_write_increment(&rx_cfg, rx_buf != NULL);

    dma_channel_configure(
        g_spi_dma_ctx.rx_dma_chan,
        &rx_cfg,
        rx_buf ? rx_buf : &dummy_rx,
        &spi_get_hw(g_spi_dma_ctx.spi)->dr,
        len,
        false
    );

    // Configure TX DMA Config
    dma_channel_config tx_cfg = dma_channel_get_default_config(g_spi_dma_ctx.tx_dma_chan);
    channel_config_set_transfer_data_size(&tx_cfg, DMA_SIZE_8);
    channel_config_set_dreq(&tx_cfg, spi_get_dreq(g_spi_dma_ctx.spi, true));
    channel_config_set_read_increment(&tx_cfg, tx_buf != NULL);
    channel_config_set_write_increment(&tx_cfg, false);

    dma_channel_configure(
        g_spi_dma_ctx.tx_dma_chan,
        &tx_cfg,
        &spi_get_hw(g_spi_dma_ctx.spi)->dr,
        tx_buf ? tx_buf : &dummy_tx,
        len,
        false
    );

    // Pull Chip Select Low
    gpio_put(5, 0); 

    // Start RX then TX DMA Channels
    dma_channel_start(g_spi_dma_ctx.rx_dma_chan);
    dma_channel_start(g_spi_dma_ctx.tx_dma_chan);

    // Block FreeRTOS task until ISR signals completion or timeout expires
    if (xSemaphoreTake(g_spi_dma_ctx.transfer_complete_sem, pdMS_TO_TICKS(timeout_ms)) != pdTRUE) {
        gpio_put(5, 1); // De-assert CS on error
        return SPI_DMA_ERROR_TIMEOUT;
    }

    // Pull Chip Select High
    gpio_put(5, 1);

    return SPI_DMA_OK;
}