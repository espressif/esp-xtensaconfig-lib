#
# Libraries for configurations
#

TARGET_ESP_CHIPS = \
	esp8266 \
	esp32s2 \
	esp32
TARGET_ESP_ARCH ?= xtensa

CC ?= gcc
AR ?= ar

ifeq ($(TARGET_ESP_ARCH), xtensa)
TARGET_ESP_ARCH_NUM = 0
lib: libxtensaconfig-default.a libxtensaconfig-gdb.a $(patsubst %,xtensaconfig-%.so,$(TARGET_ESP_CHIPS))
else
TARGET_ESP_ARCH_NUM = 1
TARGET_ESP_CHIPS = esp
lib:
endif

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


ifeq ($(PLATFORM),)
install: lib
else
ifeq ($(PLATFORM),windows)
install: lib gdb_exe_win
else
install: lib gdb_exe_unix
endif
endif
ifeq ($(TARGET_ESP_ARCH), xtensa)
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp -f *.so $(DESTDIR)$(PREFIX)/lib
endif

gdb_exe_unix:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	for TARGET_ESP_CHIP in ${TARGET_ESP_CHIPS} ; do \
		OUTPUT_FILE=$(DESTDIR)$(PREFIX)/bin/$${TARGET_ESP_ARCH}-$${TARGET_ESP_CHIP}-elf-gdb; \
		if [ ${TARGET_ESP_ARCH} == "xtensa" ]; then \
			sed -e 's/TARGET_ESP_MCPU_OPTION/--mcpu='$${TARGET_ESP_CHIP}'/g' -e 's/TARGET_ESP_ARCH/'$${TARGET_ESP_ARCH}'/g' bin_wrappers/$$PLATFORM > $$OUTPUT_FILE ; \
		else \
			sed -e 's/TARGET_ESP_MCPU_OPTION//g' -e 's/TARGET_ESP_ARCH/'$${TARGET_ESP_ARCH}'/g' bin_wrappers/$$PLATFORM > $$OUTPUT_FILE ; \
		fi; \
		chmod +x $$OUTPUT_FILE; \
	done

gdb_exe_win:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	for TARGET_ESP_CHIP in ${TARGET_ESP_CHIPS} ; do \
		OUTPUT_FILE=$(DESTDIR)$(PREFIX)/bin/$${TARGET_ESP_ARCH}-$${TARGET_ESP_CHIP}-elf-gdb.exe; \
		$(CC) -DTARGET_ESP_ARCH=${TARGET_ESP_ARCH_NUM} bin_wrappers/$${PLATFORM}.c -o $$OUTPUT_FILE; \
	done
