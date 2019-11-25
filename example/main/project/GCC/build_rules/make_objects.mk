
MODULE_OBJS=$(patsubst %.c, $(OBJS_DIR)/%.o, $(notdir $(MODULE_SOURCES)))
MODULE_DEPS=$(patsubst %.c, $(OBJS_DIR)/%.d, $(notdir $(MODULE_SOURCES)))

MODULE_OBJS+=$(patsubst %.s, $(OBJS_DIR)/%.o, $(notdir $(MODULE_ASSEM_SOURCES)))
MODULE_DEPS+=$(patsubst %.s, $(OBJS_DIR)/%.d, $(notdir $(MODULE_ASSEM_SOURCES)))

GET_SOURCE_NAME=$(strip $(foreach path_file, $(MODULE_SOURCES) $(MODULE_ASSEM_SOURCES), $(if $(filter $(patsubst %.o, %.s, $(notdir $1)) $(patsubst %.o, %.c, $(notdir $1)), $(notdir $(path_file))), $(path_file))))
#GET_SOURCE_NAME_ASSEM=$(strip $(foreach path_file, $(MODULE_SOURCES), $(if $(filter $(patsubst %.o, %.s, $(notdir $1))), $(notdir $(path_file))), $(path_file))))

all: module_make
-include $(MODULE_DEPS)
#$(OBJS_DIR)/%.o : $(patsubst %.o, %c, $(notdir $@))$(CUR_DIRS)/%.c
#$(MODULE_OBJS) $(MODULE_OBJS_ASSEM) : 
#$(MODULE_OBJS) :
$(MODULE_OBJS) :
	$(QUIET)$(ECHO) "Compiling $(call GET_SOURCE_NAME, $@)..."
	$(QUIET)$(CC) -c $(call GET_SOURCE_NAME, $@) -o $@ -MD -MF $(OBJS_DIR)/$(notdir $*).d -MP $(CFLAGS) $(CPUFLAGS) $(FLAGS) $(MODULE_INCLUDES)
module_make : $(MODULE_OBJS)
	$(QUIET)$(ECHO) "$(CUR_NAME) built completed."

.PHONY:

clean:
	$(RM) $(MODULE_OBJS)
