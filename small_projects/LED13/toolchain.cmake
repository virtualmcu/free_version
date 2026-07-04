# toolchain.cmake — arm-none-eabi toolchain for STM32F103C8T6
set(TOOLCHAIN_BIN "")

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(TOOLCHAIN_BIN)
    set(TOOLCHAIN_PREFIX "${TOOLCHAIN_BIN}/arm-none-eabi-")
else()
    set(TOOLCHAIN_PREFIX "arm-none-eabi-")
endif()

set(EXE ".exe")
set(CMAKE_C_COMPILER   "${TOOLCHAIN_PREFIX}gcc${EXE}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++${EXE}")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc${EXE}")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PREFIX}objcopy${EXE}")
set(CMAKE_SIZE         "${TOOLCHAIN_PREFIX}size${EXE}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
