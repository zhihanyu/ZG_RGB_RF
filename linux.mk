ROOT_DIR := $(abspath .)

OFFSET=\x1b[41G
COLOR=\x1b[1;34m
RESET=\x1b[0m
CLEAR=\x1b[H\x1b[J

#copmiling dirs
CODE_DIR = $(ROOT_DIR)/code

OBJS_DIR  = $(ROOT_DIR)/out/GCC/Debug/Obj
BINS_DIR  = $(ROOT_DIR)/out/GCC/Debug/Exe
RULES_DIR = $(ROOT_DIR)/example/main/project/GCC/build_rules/

SUBDIRS  = $(CODE_DIR)

include $(RULES_DIR)/host_cmd_linux.mk
include $(RULES_DIR)/.config

CORE=4
CPUFLAGS :=  -mcpu=cortex-m4 -mthumb -g2 -w -O2 -Wno-pointer-sign -fno-common -fmessage-length=0  -ffunction-sections -fdata-sections -fomit-frame-pointer -fno-short-enums -DF_CPU=166000000L -std=gnu99 -fsigned-char
#-mthumb -mcpu=cortex-m$(CORE) -nostartfiles -Wl,--no-enum-size-warning -Wl,--no-wchar-size-warning -Wl,--gc-sections -Wl,--cref
#-mfloat-abi=soft  -mthumb-interwork -specs=nosys.specs -specs=nano.specs -specs=rdimon.specs -g -fno-common -fmessage-length=0

FLAGS    := 
#-g -Wall

#header files links
ifeq ($(CONFIG_CUSTOME_TUYA), 1)
CUSTOM_TUYA_INC = -I $(CODE_DIR)/coustom/tuya_iot/include/common       \
            -I $(CODE_DIR)/coustom/tuya_iot/include    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_adapter  \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_adapter/base_nw_intf   \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_adapter/storage    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_adapter/system    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_adapter/utilities    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_base/kv_storge    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_base/kv_storge/file    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_base/kv_storge/flash    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_base/sys_serv    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_base/uf_file    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_base/uf_file/flash    \
            -I $(CODE_DIR)/coustom/tuya_iot/include    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/ai_spearker    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/base_sdk    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/com_sdk    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/local_linkage    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/public_cloud    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/sys_rpc    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/third_cloud/coap/coap2    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/third_cloud/coap    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/third_cloud/operator/operators/operator_cmcc    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/third_cloud/operator/operators    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/third_cloud/operator    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/tuya_cloud    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/wifi_cfg_serv    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_iot_sdk/wifi_sdk    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_middleware/httpc    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_middleware/mqtt    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_middleware/tls    \
            -I $(CODE_DIR)/coustom/tuya_iot/include/tuya_middleware/tls/mbedtls    \
            -I $(CODE_DIR)/coustom/tuya_iot/tuya_app/tuya_api/tuya_adapter_include    \
            -I $(CODE_DIR)/coustom/tuya_iot/tuya_app/tuya_api/tuya_adapter_include/storage    \
            -I $(CODE_DIR)/coustom/tuya_iot/tuya_app/tuya_api/tuya_adapter_include/system    \
            -I $(CODE_DIR)/coustom/tuya_iot/tuya_app/tuya_api/tuya_adapter_include/wifi_intf    \
            -I $(CODE_DIR)/coustom/tuya_iot/tuya_app/user/tuya_common/include/driver    \
            -I $(CODE_DIR)/coustom/tuya_iot/tuya_app/user/tuya_common/include    \
            -I $(CODE_DIR)/coustom/tuya_iot/tuya_app/user/tuya_user/s9070_common_config/include
endif

CFLAGS    = -I $(CODE_DIR)/system        \
           -I $(CODE_DIR)/utils         \
           -I $(CODE_DIR)/cmsis         \
           -I $(CODE_DIR)/cmsis/inc     \
           -I $(CODE_DIR)/core/os       \
           -I $(CODE_DIR)/core/os/freertos_v10.0.1/Source/include                \
           -I $(CODE_DIR)/core/os/freertos_v10.0.1/Source/portable/GCC/ARM_CM4F  \
           -I $(CODE_DIR)/core/wlan     \
           -I $(CODE_DIR)/core/hal      \
           -I $(CODE_DIR)/atcmd/at_esp  \
           -I $(CODE_DIR)/atcmd/at_test \
           -I $(CODE_DIR)/atcmd         \
           -I $(CODE_DIR)/net           \
           -I $(CODE_DIR)/net/ping      \
           -I $(CODE_DIR)/net/dhcpc     \
           -I $(CODE_DIR)/net/dhcps     \
           -I $(CODE_DIR)/net/httpc     \
           -I $(CODE_DIR)/net/mqtt/MQTTPacket/src     \
           -I $(CODE_DIR)/net/mqtt/MQTTPacket/samples/  \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include              \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include/netif        \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include/posix        \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include/posix/sys/   \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include/ipv4/        \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include/ipv6/        \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include/lwip/        \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/include/lwip/apps    \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/src/apps/sntp      \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/port/sci           \
           -I $(CODE_DIR)/net/lwip/lwip_v1.4.1/port/sci/freertos  \
           -I $(CODE_DIR)/net/mbedtls/include              \
           -I $(CODE_DIR)/net/mbedtls/include/mbedtls

export CC COPY DUMP AR MK MV SED CP ROOT_DIR CFLAGS OBJS_DIR CPUFLAGS FLAGS MAKE LD
export RULES_DIR CUSTOM_TUYA_INC

all: COMPILE_BOJS LINK_OUTPUT
	$(QUIET)$(ECHO) "Call bat to build ota bin."
	sudo sh $(ROOT_DIR)/script/links/GCC/s9070_linux.sh $(ROOT_DIR)/out/GCC $(ROOT_DIR)/out/GCC

COMPILE_BOJS :
	$(QUIET)$(ECHO) "$(SUBDIRS)"
	$(MAKE) -C $(SUBDIRS)

LINK_OUTPUT: 
	$(QUIET)$(ECHO)
	$(QUIET)$(ECHO) "*********************************************"
	$(QUIET)$(ECHO) "*              build elf...                 *"
	$(QUIET)$(ECHO) "*********************************************"
	$(QUIET)$(ECHO)
	$(QUIET)$(CP) $(RULES_DIR)/make_target.mk	$(OBJS_DIR)/Makefile
	$(QUIET)$(MAKE) -C $(OBJS_DIR) clean
	$(QUIET)$(MAKE) -C $(OBJS_DIR)

.PHONY :

clean :
	$(QUIET)$(RM) -f $(OBJS_DIR)/*.o
	$(QUIET)$(RM) -f $(OBJS_DIR)/*.d
	$(QUIET)$(ECHO) "Clean objects and depends completed."
	
help:
	@echo "*******************MAKEFILE HELP*************************"
	@echo "* the project include these compoments:                 *"
	@echo "* atcmd cmsis core net utils startup                    *"
	@echo "* input: make , start building project                  *"
	@echo "* input: make clean, can remove all objects             *"
	@echo "* input: make +compoment can clean the compoment        *"
	@echo "* for exmaple: make atcmd can clean the objects of atcmd*"
	@echo "*******************SCI-MAKEFILE**************************"
atcmd :
	cd $(CODE_DIR)/atcmd; make clean 
cmsis :
	cd $(CODE_DIR)/cmsis; make clean 
core  :
	cd $(CODE_DIR)/core; make clean 
net   :
	cd $(CODE_DIR)/net; make clean 
utils :
	cd $(CODE_DIR)/utils; make clean 
startup :
	cd $(CODE_DIR)/startup; make clean 


	
