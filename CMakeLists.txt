# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(hello_world)

target_sources(app PRIVATE src/main.c)

add_subdirectory(src/bme_sensor)
add_subdirectory(src/ml_sensor)
add_subdirectory(src/cjmcu_sensor)
add_subdirectory(src/ble_beacon)
add_subdirectory(src/mh_rd_sensor)
add_subdirectory(src/thread_worker)
add_subdirectory(src/util)
