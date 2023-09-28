#
# Libraries for configurations
#

TARGET_ESP_CHIPS = \
	esp32s3 \
	esp32s2 \
	esp32

CROSS_COMPILE ?= ""
CC = $(CROSS_COMPILE)gcc
CXX = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar

lib: libxtensaconfig-default.a libxtensaconfig-gdb.a $(patsubst %,xtensaconfig-%.so,$(TARGET_ESP_CHIPS))

OBJ_DIR=./obj

LIBCONFIG-GDB_SOURCES = \
         src/dynconfig.c \
         src/option_gdb.c

LIBCONFIG-DEFAULT_SOURCES = \
         lib_config/xtensa-config.c

.PHONY: lib

RELEASE_FLAGS = -O2 -Wall -Wextra -Wpedantic -D_GNU_SOURCE

LIB_SRCS = lib_src/xtensa-config.c \
	       config/xtensa_%/binutils/bfd/xtensa-modules.c \
	       config/xtensa_%/gdb/gdb/xtensa-config.c \
	       config/xtensa_%/gdb/gdb/xtensa-xtregs.c

COMMON_INCLUDE = -Ilib_include -Iinclude

# Include chip depended directory first
LIB_INCLUDE = -Iconfig/xtensa_$*/binutils/include ${COMMON_INCLUDE}

LIB_FLAGS = -nostdlib -shared -fPIC $(RELEASE_FLAGS) $(CFLAGS)

libxtensaconfig-default.a: $(patsubst %.c,$(OBJ_DIR)/%.o,$(LIBCONFIG-DEFAULT_SOURCES))
	$(AR) rcs $@ $^

libxtensaconfig-gdb.a: $(patsubst %.c,$(OBJ_DIR)/%.o,$(LIBCONFIG-GDB_SOURCES))
	$(AR) rcs $@ $^

$(OBJ_DIR)/%.o: %.c dirmake
	$(CC) -c $(RELEASE_FLAGS) $(CFLAGS) $(COMMON_INCLUDE) -o $@  $<

dirmake:
	@mkdir -p $(OBJ_DIR)/src $(OBJ_DIR)/lib_config

xtensaconfig-esp.so:
	@echo dummy

xtensaconfig-%.so: $(LIB_SRCS)
	@echo $@
	@echo $^
	@echo $(CFLAGS)
	$(CC) $(LIB_FLAGS) $(LIB_INCLUDE) $^ -o $@

clean:
	rm -fr *.so *.a $(OBJ_DIR)


install: lib
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp -f *.so $(DESTDIR)$(PREFIX)/lib
