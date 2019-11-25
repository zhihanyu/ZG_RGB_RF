CUR_DIR := .

TARGET := s907x.elf

OBJECTS := ${wildcard *.o}

LINK_LIBS	=
LINK_LIBS   += -l:s907x_platform.a
LINK_LIBS   += -l:s907x_net.a

LDFLAGS    := -L $(ROOT_DIR)/script/links/GCC
LDFLAGS    += -L $(ROOT_DIR)/script/links/GCC/s907x_bin_api.txt
LDFLAGS    += -L $(ROOT_DIR)/library/GCC

LD_SCRIPT = -Wl,--cref -T $(ROOT_DIR)/script/links/GCC/s907x_link_ota.ld

AR_ARGS = rcs

LINK_OPTS = -Wl,-Map,s907x.map
LINK_OPTS += -Wl,--whole-archive -Wl,--start-group
LINK_OPTS += $(LINK_LIBS)
LINK_OPTS += $(OBJECTS)
LINK_OPTS += -Wl,--end-group -Wl,-no-whole-archive -Wl,--gc-sections
LINK_OPTS += $(LD_SCRIPT)
LINK_OPTS += $(LDFLAGS)
LINK_OPTS += -mcpu=cortex-m4 -mthumb 
LINK_OPTS += -Os -nostartfiles -Wl,--no-enum-size-warning -Wl,--no-wchar-size-warning -Wl,--gc-sections -Wl,--cref --specs=nano.specs -u _printf_float

$(TARGET) : $(OBJECTS)
	$(QUIET)$(ECHO) "Link objects to output $@"
	$(QUIET)$(ECHO) "$(LINK_OPTS)" > link.opts
	$(CC) $(LINK_OPTS) -o $@

clean:
	-$(QUIET)$(RM) $(TARGET)

