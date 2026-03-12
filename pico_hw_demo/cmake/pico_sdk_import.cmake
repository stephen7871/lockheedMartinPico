# Minimal Pico SDK import shim.
# This file expects PICO_SDK_PATH to be set in the environment.

if (DEFINED ENV{PICO_SDK_PATH})
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
else()
    message(FATAL_ERROR "PICO_SDK_PATH is not set. Export it before configuring a Pico firmware build.")
endif()
