cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(xiao_c6_zephyr_bringup)

if(CONFIG_WIFI AND CONFIG_MQTT_LIB)
    target_sources(app PRIVATE src/wifi_mqtt.c)
else()
    target_sources(app PRIVATE src/main.c)
endif()
