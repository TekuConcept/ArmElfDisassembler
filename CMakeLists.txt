CMAKE_MINIMUM_REQUIRED(VERSION 3.8.0)
PROJECT(arm_elf)

SET(CMAKE_CXX_STANDARD 11)

INCLUDE_DIRECTORIES(Include opcodes)

FILE(GLOB_RECURSE OPCODE_CFILES "opcodes/*.c")
FILE(GLOB_RECURSE OPCODE_CXXFILES "opcodes/*.cpp")
ADD_LIBRARY(opcodes ${OPCODE_CFILES} ${OPCODE_CXXFILES})
TARGET_COMPILE_DEFINITIONS(opcodes PUBLIC ARCH_arm)

FILE(GLOB_RECURSE PROJECT_EXTENSION_FILES "Source/arm/*.cpp")
ADD_LIBRARY(${PROJECT_NAME}_a STATIC ${PROJECT_EXTENSION_FILES})

FILE(GLOB_RECURSE PROJECT_FILES "Source/*.cpp")
ADD_EXECUTABLE(${PROJECT_NAME} ${PROJECT_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} opcodes)

FILE(GLOB_RECURSE BIN2ASM_FILES "bin2asm/*.cpp")
ADD_EXECUTABLE(bin2asm ${BIN2ASM_FILES})
TARGET_LINK_LIBRARIES(bin2asm opcodes)

FILE(GLOB_RECURSE RUN_FILES "extra01/*.cpp")
ADD_EXECUTABLE(run ${RUN_FILES})
TARGET_LINK_LIBRARIES(run opcodes)

FILE(GLOB_RECURSE ELF2ASM_FILES "elf2asm/Source/*.cpp")
ADD_EXECUTABLE(elf2asm ${ELF2ASM_FILES} "elf2asm/main.cpp")
TARGET_LINK_LIBRARIES(elf2asm opcodes)
TARGET_INCLUDE_DIRECTORIES(elf2asm PUBLIC "elf2asm/Include")
