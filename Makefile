#
# Libraries for configurations
#

TARGET_CHIPS = \
	esp8266 \
	esp32s2 \
	esp32

CC ?= gcc
AR ?= ar

OBJ_DIR=./obj

LIBCONFIG-GDB_SOURCES = \
         src/dynconfig.c \
         src/option_gdb.c

LIBCONFIG-DEFAULT_SOURCES = \
         lib_config/xtensa-config.c

.PHONY: lib
lib: libxtensaconfig-default.a libxtensaconfig-gdb.a $(patsubst %,xtensaconfig-%.so,$(TARGET_CHIPS))

RELEASE_FLAGS = -O2 -Wall -Wextra -Wpedantic -D_GNU_SOURCE

LIB_SRCS = lib_src/xtensa-config.c \
	       config/xtensa_%/binutils/bfd/xtensa-modules.c \
	       config/xtensa_%/gdb/gdb/xtensa-config.c \
	       config/xtensa_%/gdb/gdb/xtensa-xtregs.c

COMMON_INCLUDE = -Ilib_include -Iinclude

# Include chip depended directory first
LIB_INCLUDE = -Iconfig/xtensa_$*/binutils/include ${COMMON_INCLUDE}

LIB_FLAGS = -shared -fPIC $(RELEASE_FLAGS) $(CFLAGS)

libxtensaconfig-default.a: $(patsubst %.c,$(OBJ_DIR)/%.o,$(LIBCONFIG-DEFAULT_SOURCES))
	$(AR) rcs $@ $^

libxtensaconfig-gdb.a: $(patsubst %.c,$(OBJ_DIR)/%.o,$(LIBCONFIG-GDB_SOURCES))
	$(AR) rcs $@ $^

$(OBJ_DIR)/%.o: %.c dirmake
	$(CC) -c $(RELEASE_FLAGS) $(CFLAGS) $(COMMON_INCLUDE) -o $@  $<

dirmake:
	@mkdir -p $(OBJ_DIR)/src $(OBJ_DIR)/lib_config

xtensaconfig-%.so: $(LIB_SRCS)
	@echo $@
	@echo $^
	@echo $(CFLAGS)
	$(CC) $(LIB_FLAGS) $(LIB_INCLUDE) $^ -o $@

clean:
	rm -fr *.so *.a $(OBJ_DIR)


ifeq ($(PLATFORM),)
install: lib
else
ifeq ($(PLATFORM),windows)
install: lib gdb_exe_win
else
install: lib gdb_exe_unix
endif
endif
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp *.so $(DESTDIR)$(PREFIX)/lib

gdb_exe_unix:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	for TARGET_CHIP in ${TARGET_CHIPS} ; do \
		OUTPUT_FILE=$(DESTDIR)$(PREFIX)/bin/xtensa-$${TARGET_CHIP}-elf-gdb; \
		sed 's/TARGET_CHIP/'$${TARGET_CHIP}'/g' bin_wrappers/$$PLATFORM > $$OUTPUT_FILE ; \
		chmod +x $$OUTPUT_FILE; \
	done

gdb_exe_win:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	for TARGET_CHIP in ${TARGET_CHIPS} ; do \
		OUTPUT_FILE=$(DESTDIR)$(PREFIX)/bin/xtensa-$${TARGET_CHIP}-elf-gdb.exe; \
		$(CC) bin_wrappers/$${PLATFORM}.c -o $$OUTPUT_FILE; \
	done
