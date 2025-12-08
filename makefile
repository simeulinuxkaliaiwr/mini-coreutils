CC = gcc
CFLAGS = -march=native -Wall -Wextra -std=c11
LDFLAGS =

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

DIRS = $(wildcard $(SRC_DIR)/*)
PROJECTS = $(notdir $(DIRS))
BINARIES = $(addprefix $(BIN_DIR)/, $(PROJECTS))

all: $(BINARIES)

define PROJECT_RULES
$(BIN_DIR)/$(1): $(OBJ_DIR)/$(1).o
	@echo "Linking $$@"
	@mkdir -p $(BIN_DIR)
	$$(CC) $$(LDFLAGS) $$< -o $$@

$(OBJ_DIR)/$(1).o: $(SRC_DIR)/$(1)/$(1).c
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
