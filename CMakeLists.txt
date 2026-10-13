cmake_minimum_required(VERSION 3.13)

# Include Pico SDK bootstrap
include(pico_sdk_import.cmake OPTIONAL)

project(spi_dma_driver C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# FreeRTOS kernel path — set via -DFREERTOS_KERNEL_PATH=... or environment
if(NOT DEFINED FREERTOS_KERNEL_PATH)
    set(FREERTOS_KERNEL_PATH $ENV{FREERTOS_KERNEL_PATH})
endif()

# Add FreeRTOS kernel
add_subdirectory(${FREERTOS_KERNEL_PATH} FreeRTOS-Kernel)

# Define executable
add_executable(spi_dma_example
    src/spi_dma_driver.c
    src/main.c
)

# Include directories
target_include_directories(spi_dma_example PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/GCC/ARM_CM33_NTZ/non_secure
)

# Link libraries
target_link_libraries(spi_dma_example
    pico_stdlib
    hardware_spi
    hardware_dma
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

# Generate UF2, bin, hex, map outputs
pico_add_extra_outputs(spi_dma_example)

# Optional: enable USB or UART stdio for debug output
pico_enable_stdio_usb(spi_dma_example 1)
pico_enable_stdio_uart(spi_dma_example 0)
