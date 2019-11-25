
COMPILER := $(ROOT_DIR)/build/compiler/Win32/bin/arm-none-eabi-
CMD     := $(ROOT_DIR)/build/cmd/win32

EXECUTABLE_SUFFIX   := .exe


export AR   := $(COMPILER)ar$(EXECUTABLE_SUFFIX)
export CP   := cp
export CC   := $(COMPILER)gcc$(EXECUTABLE_SUFFIX)
export COPY := $(COMPILER)objcopy$(EXECUTABLE_SUFFIX)
export DUMP := $(COMPILER)objdump$(EXECUTABLE_SUFFIX)
export LD   := $(COMPILER)ld$(EXECUTABLE_SUFFIX)
export MAKE := $(CMD)/make
export MK   := $(CMD)/mkdir
export MV   := mv
export RM   := $(CMD)/rm
export SED  := sed


export QUIET:=@

export ECHO              := echo

