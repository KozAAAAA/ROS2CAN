cmake_minimum_required(VERSION 3.0.0)
project(RSO2CAN VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(USBCANlib src/modded.cpp src/rs232.c src/uCCButil.cpp src/uCCBlibC.cpp) 
target_compile_features(USBCANlib PRIVATE cxx_std_17)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
