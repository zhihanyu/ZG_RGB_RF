CUR_DIR := .

include $(RULES_DIR)/.config

ATCMD       :=$(CUR_DIR)/atcmd 
CMSIS       :=$(CUR_DIR)/cmsis 
CORE        :=$(CUR_DIR)/core 
NET         :=$(CUR_DIR)/net 
STARTUP     :=$(CUR_DIR)/startup
UTILS       :=$(CUR_DIR)/utils

#SUBDIRS := $(ATCMD) $(CMSIS) $(CORE) $(NET) $(STARTUP) $(UTILS)

ifeq ($(CONFIG_MODULE_ATCMD) , 1)
SUBDIRS := $(ATCMD)
endif
SUBDIRS += $(CMSIS) $(CORE) $(NET) $(STARTUP) $(UTILS)

all: $(SUBDIRS)
$(SUBDIRS): ECHO
	$(MAKE) -C $@
ECHO:
	$(QUIET)$(ECHO) "$(SUBDIRS)"
.PHONY:

clean:
	$(foreach N, $(SUBDIRS), $(MAKE) -C $(N) clean;)

