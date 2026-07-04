cmake_minimum_required(VERSION 3.16)

# Toolchain via -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake
set(CMAKE_ASM_COMPILER_FORCED TRUE)
set(CMAKE_ASM_COMPILER_WORKS TRUE)

project(LED13 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# MCU: STM32F103C8T6

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.s"
    "${CMAKE_SOURCE_DIR}/src/*.S"
)
list(FILTER SOURCES EXCLUDE REGEX "CMakeFiles")

list(FILTER SOURCES EXCLUDE REGEX "startup\\.c$")
list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/startup.c)

add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

target_compile_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m3 -mthumb
    -Wall -Wextra -O0 -g
    -ffunction-sections -fdata-sections
)

target_link_options(${PROJECT_NAME} PRIVATE
    -mcpu=cortex-m3 -mthumb
    -Wl,--gc-sections
    -T${CMAKE_SOURCE_DIR}/stm32f103.ld
    -nostdlib
    -lgcc
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND arm-none-eabi-objcopy -O ihex   $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
    COMMAND arm-none-eabi-objcopy -O binary $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
    COMMAND arm-none-eabi-size $<TARGET_FILE:${PROJECT_NAME}>
)
