
CUR_DIRS         =  ./           \
                    ./freertos_v10.0.1/Source/      \
                    ./freertos_v10.0.1/Source/portable/GCC/ARM_CM4F/ 

MODULE_INCLUDES    =  -I./          \
                    -I./freertos_v10.0.1/Source/      \
                    -I./freertos_v10.0.1/Source/include/  \
                    -I./freertos_v10.0.1/Source/portable/GCC/ARM_CM4F/

MODULE_SOURCES     = ./freertos_callback.c    \
                    ./freertos_v10.0.1/Source/croutine.c	\
                    ./freertos_v10.0.1/Source/event_groups.c	\
                    ./freertos_v10.0.1/Source/list.c	\
                    ./freertos_v10.0.1/Source/queue.c	\
                    ./freertos_v10.0.1/Source/stream_buffer.c	\
                    ./freertos_v10.0.1/Source/tasks.c	\
                    ./freertos_v10.0.1/Source/timers.c	\
                    ./freertos_v10.0.1/Source/portable/MemMang/heap_4.c  

MODULE_ASSEM_SOURCES   = ./freertos_v10.0.1/Source/portable/GCC/ARM_CM4F/port.s 

include $(RULES_DIR)/make_objects.mk

