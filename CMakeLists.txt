cmake_minimum_required(VERSION 3.13)

# Include Pico SDK
include(pico_sdk_import.cmake OPTIONAL)

project(spi_dma_driver C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# Define executable from src directory
add_executable(spi_dma_example
    src/spi_dma_driver.c
)

# Target include directories
target_include_directories(spi_dma_example PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src
)

# Link FreeRTOS and Pico SDK libraries
target_link_libraries(spi_dma_example
    pico_stdlib
    hardware_spi
    hardware_dma
    FreeRTOS-Kernel
)

pico_add_extra_outputs(spi_dma_example)