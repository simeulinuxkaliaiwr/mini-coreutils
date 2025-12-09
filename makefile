CC = gcc
CFLAGS = -march=native -Wall -Wextra -std=c11
LDFLAGS =

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

LIB_NAME = mini
LIB_FILE = $(OBJ_DIR)/lib$(LIB_NAME).a
LIB_SRC = $(SRC_DIR)/lib/lib.c
LIB_OBJ = $(OBJ_DIR)/lib.o
CFLAGS += -I$(SRC_DIR)/lib

DIRS = $(wildcard $(SRC_DIR)/*)

NON_EXECUTABLES = lib
PROJECTS = $(filter-out $(NON_EXECUTABLES), $(notdir $(DIRS)))
BINARIES = $(addprefix $(BIN_DIR)/, $(PROJECTS))

all: $(LIB_FILE) $(BINARIES)

$(LIB_OBJ): $(LIB_SRC)
	@echo "Compiling Library Source: $< to $@"
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_FILE): $(LIB_OBJ)
	@echo "Creating Static Library: $@"
	@mkdir -p $(OBJ_DIR)
	ar rcs $@ $^

define PROJECT_RULES
$(BIN_DIR)/$(1): $(OBJ_DIR)/$(1).o $(LIB_FILE)
	@echo "Linking $$@"
	@mkdir -p $(BIN_DIR)
	$$(CC) $$(LDFLAGS) $$< $$(LIB_FILE) -o $$@

$(OBJ_DIR)/$(1).o: $(SRC_DIR)/$(1)/$(1).c $(SRC_DIR)/lib/lib.h
	@echo "Compiling $$< to $$@"
	@mkdir -p $(OBJ_DIR)
	$$(CC) $$(CFLAGS) -c $$< -o $$@
endef

$(foreach proj,$(PROJECTS),$(eval $(call PROJECT_RULES,$(proj))))

.PHONY: $(PROJECTS)
$(PROJECTS): %: $(BIN_DIR)/%

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

rebuild: clean all

list:
	@echo "Available projects:"
	@for proj in $(PROJECTS); do echo "  - $$proj"; done

.PHONY: all clean rebuild list

