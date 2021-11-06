#
# Libraries for configurations
#

TARGET_CHIPS = \
	esp8266 \
	esp32s2 \
	esp32

.PHONY: lib
lib: $(patsubst %,xtensaconfig-%.so,$(TARGET_CHIPS))

RELEASE_FLAGS = -O2 -Wall -Wextra -Wpedantic -D_GNU_SOURCE

LIB_SRCS = lib_src/xtensa-config.c \
	       config/xtensa_%/binutils/bfd/xtensa-modules.c \
	       config/xtensa_%/gdb/gdb/xtensa-config.c \
	       config/xtensa_%/gdb/gdb/xtensa-xtregs.c

LIB_INCLUDE = -Ilib_include -Iinclude \
	   -Iconfig/xtensa_$*/binutils/include

LIB_FLAGS = -shared -fPIC $(RELEASE_FLAGS) $(CFLAGS)

xtensaconfig-%.so: $(LIB_SRCS)
	@echo $@
	@echo $^
	@echo $(CFLAGS)
	$(CC) $(LIB_FLAGS) $(LIB_INCLUDE) $^ -o $@

clean:
	rm -f *.so


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

#
# Tests for the common code
#
# See: https://www.throwtheswitch.org/build/make

COMMON_PATH = src/
UNITY_PATH = test/Unity-2.4.3/
TEST_PATH = test/src/
BUILD_PATH = test/build/

COMMON_SRCS = $(wildcard $(COMMON_PATH)*.c)
COMMON_OBJS = $(patsubst $(COMMON_PATH)%.c,$(BUILD_PATH)%.o,$(COMMON_SRCS) )

TEST_COMMON_SRC = $(TEST_PATH)test_main.c
TEST_RESULT = $(patsubst $(TEST_PATH)%.c,$(BUILD_PATH)%.txt,$(TEST_COMMON_SRC) )
TEST_BINARY = $(patsubst $(TEST_PATH)%.c,$(BUILD_PATH)%.out,$(TEST_COMMON_SRC) )

PASSED = `grep -s PASS $(BUILD_PATH)*.txt`
FAIL = `grep -s FAIL $(BUILD_PATH)*.txt`
IGNORE = `grep -s IGNORE $(BUILD_PATH)*.txt`

TEST_CFLAGS = $(RELEASE_FLAGS) $(CFLAGS) -I$(UNITY_PATH) -T$(TEST_PATH) \
	-DUTEST -DTEST_BINDIR="$(shell readlink -f $(BUILD_PATH))"

.PHONY: test
test: $(BUILD_PATH) $(TEST_RESULT)
	@echo -e "-----------------------\nIGNORES:\n-----------------------"
	@echo -e "$(IGNORE)"
	@echo -e "-----------------------\nFAILURES:\n-----------------------"
	@echo -e "$(FAIL)"
	@echo -e "-----------------------\nPASSED:\n-----------------------"
	@echo -e "$(PASSED)"
	@echo -e "\nDONE"

$(BUILD_PATH)%.txt: $(BUILD_PATH)%.out
	-./$< > $@ 2>&1

$(BUILD_PATH)%.out: $(BUILD_PATH)%.o $(COMMON_OBJS) $(BUILD_PATH)unity.o
	$(CC) -o $@ $^

$(BUILD_PATH)%.o:: $(TEST_PATH)%.c
	$(CC) -c $(TEST_CFLAGS) $< -o $@

$(BUILD_PATH)%.o:: $(COMMON_PATH)%.c
	$(CC) -c $(TEST_CFLAGS) $< -o $@

$(BUILD_PATH)%.o:: $(UNITY_PATH)%.c $(UNITY_PATH)%.h
	$(CC) -c $(TEST_CFLAGS) $< -o $@

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

.PRECIOUS: $(BUILD_PATH)%.out
.PRECIOUS: $(BUILD_PATH)%.o
.PRECIOUS: $(BUILD_PATH)%.txt
