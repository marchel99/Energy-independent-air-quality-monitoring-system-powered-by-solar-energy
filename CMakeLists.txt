cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME 31_05_2024)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-bme280-master/bme280.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-ens160-master/ens160.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-sen05xx-master/sen05xx.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-max17048-master/max.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/epd4in2b.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/epdpaint.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/font8.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/font12.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/font16.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/font20.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/font24.c
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master/imagedata.c
    ${CMAKE_SOURCE_DIR}/Core/Src/user_interface.c
    ${CMAKE_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32l4xx_it.c
    ${CMAKE_SOURCE_DIR}/Core/Src/stm32l4xx_hal_msp.c
    ${CMAKE_SOURCE_DIR}/Core/Src/fatfs_sd_card.c
    ${CMAKE_SOURCE_DIR}/Core/Src/sensors.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-bme280-master
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-ens160-master
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-sen05xx-master
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-max17048-master
    ${CMAKE_SOURCE_DIR}/Libraries/stm32-epd4in2b-master
  
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
)

# Enable float support in newlib
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -u _printf_float -u _scanf_float")