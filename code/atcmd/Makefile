
include $(RULES_DIR)/.config

AT_COMM_SOURCES =  ./at_cmd.c

AT_ESP_SOURCES	=  ./at_esp/at_esp_init.c    \
                    ./at_esp/at_network.c    \
                    ./at_esp/at_wlan.c

AT_TEST_SOURCES =  ./at_test/adc_test.c    \
                    ./at_test/at_test.c    \
                    ./at_test/flash_test.c    \
                    ./at_test/gpio_test.c    \
                    ./at_test/httpc_test.c    \
                    ./at_test/i2c_test.c    \
                    ./at_test/i2s_test.c    \
                    ./at_test/iperf_test.c    \
                    ./at_test/ipref_sockets.c    \
                    ./at_test/mqtt_test.c    \
                    ./at_test/ota_test.c    \
                    ./at_test/ping_test.c    \
                    ./at_test/rtc_test.c    \
                    ./at_test/spi_test.c    \
                    ./at_test/timer_test.c    \
                    ./at_test/uart_test.c    \
                    ./at_test/wdg_test.c    \
                    ./at_test/wlan_test.c 

AT_TEST_TUYA 	= ./at_test/tuya_test.c

MODULE_SOURCES = $(AT_COMM_SOURCES)
MODULE_SOURCES += $(AT_ESP_SOURCES)
MODULE_SOURCES += $(AT_TEST_SOURCES)

ifeq ($(CONFIG_AT_TEST_CUSTOM_TUYA), 1)
MODULE_SOURCES += $(AT_TEST_TUYA)
endif

include $(RULES_DIR)/make_objects.mk

