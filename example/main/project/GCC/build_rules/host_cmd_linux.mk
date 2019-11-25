
COMPILER := $(ROOT_DIR)/build/compiler/Linux64/bin/arm-none-eabi-
CMD     := $(ROOT_DIR)/build/cmd/linux64

#EXECUTABLE_SUFFIX   := .exe


export AR   := $(COMPILER)ar
export CP   := cp
export CC   := $(COMPILER)gcc
export COPY := $(COMPILER)objcopy
export DUMP := $(COMPILER)objdump
export LD   := $(COMPILER)ld
export MAKE := $(CMD)/make
export MK   := $(CMD)/mkdir
export MV   := mv
export RM   := $(CMD)/rm
export SED  := sed


export QUIET:=@

export ECHO              := echo

