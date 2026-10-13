# Non-Blocking SPI DMA Driver — RP2350 / Raspberry Pi Pico 2

**Embedded Solutions Provider, LLC** | Author: Eric L. Johnson

---

## Overview

A production-quality, non-blocking full-duplex SPI driver for the **RP2350 microcontroller** (Raspberry Pi Pico 2), built on the **Raspberry Pi Pico SDK** and **FreeRTOS**.

Rather than polling or busy-waiting, this driver uses the RP2350 DMA controller to transfer data entirely in hardware while the calling FreeRTOS task yields the CPU. A binary semaphore coordinates the task and the DMA interrupt service routine — the task blocks on transfer start, the ISR fires on completion and unblocks the task. Zero CPU cycles are wasted during the transfer.

---

## Features

- Non-blocking full-duplex SPI transfers via RP2350 DMA controller
- FreeRTOS binary semaphore synchronization (task yields during transfer)
- DMA IRQ0 exclusive interrupt handler with `portYIELD_FROM_ISR()` context switch
- Configurable baudrate, GPIO pins, and SPI peripheral instance (spi0 / spi1)
- TX-only and RX-only modes via NULL buffer support
- Configurable per-transfer timeout with `SPI_DMA_ERROR_TIMEOUT` return
- Clean status code API — no magic numbers
- Single-file driver, easily portable to other RP2350 projects

---

## Hardware Target

| Parameter | Value |
|-----------|-------|
| Microcontroller | RP2350 (Raspberry Pi Pico 2) |
| SPI Peripheral | SPI0 |
| Baudrate | 1 MHz (configurable) |
| SCK | GP2 |
| MOSI (TX) | GP3 |
| MISO (RX) | GP4 |
| CS | GP5 (active LOW) |
| DMA | IRQ0, 8-bit transfer width |

---

## Repository Structure

```
EmbeddedSolutionsProvider_SPI_DMA/
├── src/
│   ├── spi_dma_driver.h       # Public API — config struct, status codes, function declarations
│   ├── spi_dma_driver.c       # Driver implementation — init, DMA config, ISR, transfer control
│   └── main.c                 # FreeRTOS test task demonstrating driver usage
├── Detailed_Design_Document/
│   └── SPI_DMA_FirmwareDesignDoc_v1.1.docx   # Full firmware design document with architecture diagrams
├── CMakeLists.txt             # Pico SDK CMake build configuration
└── README.md
```

---

## Build Instructions

### Prerequisites

- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel)
- CMake 3.13+
- ARM GCC toolchain (`arm-none-eabi-gcc`)

### Build

```bash
mkdir build && cd build
cmake .. -DPICO_SDK_PATH=/path/to/pico-sdk
make
```

Flash the resulting `spi_dma_example.uf2` to your Pico 2 by holding BOOTSEL and dragging the file onto the drive.

---

## API Summary

```c
// Initialize the driver — call once before any transfers
spi_dma_status_t spi_dma_init(const spi_dma_config_t *config);

// Execute a non-blocking full-duplex DMA transfer
// Pass NULL for tx_buf (RX-only) or rx_buf (TX-only)
spi_dma_status_t spi_dma_transfer_async(const uint8_t *tx_buf,
                                         uint8_t       *rx_buf,
                                         size_t         len,
                                         uint32_t       timeout_ms);
```

**Status codes:** `SPI_DMA_OK` · `SPI_DMA_ERROR_INVALID_PARAM` · `SPI_DMA_ERROR_BUSY` · `SPI_DMA_ERROR_TIMEOUT` · `SPI_DMA_ERROR_HARDWARE`

---

## Design Documentation

Full architecture documentation including sequence diagrams, DMA channel configuration flow, and FreeRTOS semaphore block/unblock diagrams is available in the `Detailed_Design_Document/` folder.

---

## License

Copyright © 2026 Embedded Solutions Provider, LLC. All rights reserved.

---

*For embedded firmware consulting inquiries, contact Embedded Solutions Provider, LLC.*
