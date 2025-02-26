BIN:=bin
BUILD:=build
TEST_BUILD:=$(BUILD)/tests
SRC_DIR:=src
TEST_DIR:=tests
TEST_COMMON_DIR:=$(TEST_DIR)/common

MAIN:=main.c

#OPT_FLAGS:=-O3 -flto
OPT_FLAGS:=-O0
CFLAGS:= -std=c2x -pedantic -Wall -Werror -Wno-newline-eof -Wno-gnu-binary-literal -g $(OPT_FLAGS) -DDEBUG

TEST_BIN:=$(BIN)/test
LIB_TARGET:=$(BIN)/$(LIBNAME)

HEADERS:=$(wildcard $(SRC_DIR)/*.h $(SRC_DIR)/*/*.h)
SRC:=$(wildcard $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c)

TEST_COMMON:=$(wildcard $(TEST_COMMON_DIR)/*.c)
TEST_SRC:=$(wildcard $(TEST_DIR)/*.c)

OBJ:=$(patsubst $(SRC_DIR)/%.c, $(BUILD)/%.o, $(SRC))
TEST_COMMON_OBJ:=$(patsubst $(TEST_COMMON_DIR)/%.c, $(TEST_BUILD)/%, $(TEST_COMMON))
TESTS:=$(patsubst $(TEST_DIR)/%.c, $(TEST_BIN)/%, $(TEST_SRC))

UNAME:=$(shell uname -s)

all: $(TARGET) Makefile

.PHONY: clean

OBJ_MSG_PRINTED:=1
TEST_COMMON_MSG_PRINTED:=1
TEST_MSG_PRINTED:=1
TARGET_MSG_PRINTED:=1

$(BUILD)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(OBJ_MSG_PRINTED)),, \
	$(eval OBJ_MSG_PRINTED:=0) \
	@echo "\nCompiling object files"))

	@$(CC) $(CFLAGS) -c $< -o $@
	@printf " - %-25s <- %s\n" "$@" " $<"

$(TEST_BUILD)/%.o: $(TEST_COMMON_DIR)/%.c $(HEADERS)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(TEST_COMMON_MSG_PRINTED)),, \
	$(eval TEST_COMMON_MSG_PRINTED:=0) \
	@echo "\nCompiling test common files"))

	@$(CC) $(CFLAGS) -c $< -o $@
	@printf " - %-25s <- %s\n" "$@" " $<"

$(TARGET): $(OBJ) $(MAIN)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(TARGET_MSG_PRINTED)),, \
	$(eval TARGET_MSG_PRINTED:=0) \
	@echo "\nBuilding library"))

	@$(CC) $(CFLAGS) -o $@ $^
	@printf " - %-25s <- %s\n" "$@" " $^"

$(TEST_BIN)/%: $(TEST_DIR)/%.c $(TEST_COMMON_OBJ) $(OBJ)
	@mkdir -p $(dir $@)

	$(if $(filter 0,$(MAKELEVEL)), $(if $(filter 0,$(TEST_MSG_PRINTED)),, \
	$(eval TEST_MSG_PRINTED:=0) \
	@echo "\nCompiling tests"))

	$(CC) $(CFLAGS) -o $@ $^
	@printf " - %-25s <- %s\n" "$@" " $^"

run-tests: $(TESTS)
	@mkdir -p files
	@echo "\nRunning tests\n"
	@pass=0; fail=0; \
	green='\033[32m'; red='\033[31m'; reset='\033[0m'; \
	for test in $(TESTS); do \
		if $$test; then \
			printf "%-50s %bPASSED%b\n" "Test $$(basename $$test):" "$${green}" "$${reset}"; \
			pass=$$((pass+1)); \
		else \
			printf "%-50s %bFAILED%b\n" "Test $$(basename $$test):" "$${red}" "$${reset}"; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo "\nSummary: $${green}$$pass passed$${reset}, $${red}$$fail failed$${reset}";

clean:
	rm -rf $(BIN)
	rm -rf $(BUILD)
