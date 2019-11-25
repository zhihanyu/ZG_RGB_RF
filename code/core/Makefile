
SUBDIRS := os 

all: $(SUBDIRS)
$(SUBDIRS): ECHO
	$(MAKE) -C $@
ECHO:
	$(QUIET)$(ECHO) "Enter $(SUBDIRS) to compile."
