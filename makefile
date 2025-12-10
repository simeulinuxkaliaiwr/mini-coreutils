CC = gcc
CFLAGS = -march=native -Wall -Wextra -std=gnu11
LDFLAGS =

SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

LIB_NAME = mini
LIB_FILE = $(OBJ_DIR)/lib$(LIB_NAME).a

# Lista TODOS os arquivos fonte da biblioteca
LIB_SOURCES = \
    $(SRC_DIR)/lib/lib.c \
    $(SRC_DIR)/lib/sys/guicall.c

# Gera a lista de objetos a partir das fontes
LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(LIB_SOURCES))

# Adiciona include paths DEPOIS de definir CFLAGS base
CFLAGS += -I$(SRC_DIR)/lib -I$(SRC_DIR)/lib/sys

# Descobre projetos automaticamente
DIRS = $(wildcard $(SRC_DIR)/*)
NON_EXECUTABLES = lib
PROJECTS = $(filter-out $(NON_EXECUTABLES), $(notdir $(DIRS)))
BINARIES = $(addprefix $(BIN_DIR)/, $(PROJECTS))

all: $(LIB_FILE) $(BINARIES)

# Regra para compilar QUALQUER arquivo .c da biblioteca para .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling: $< -> $@"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Cria a biblioteca estática com TODOS os objetos
$(LIB_FILE): $(LIB_OBJECTS)
	@echo "Creating Static Library: $@"
	@mkdir -p $(dir $@)
	ar rcs $@ $^

# Regra para os executáveis
define PROJECT_RULES
$(BIN_DIR)/$(1): $(OBJ_DIR)/$(1).o $(LIB_FILE)
	@echo "Linking $$@"
	@mkdir -p $(BIN_DIR)
	$$(CC) $$(LDFLAGS) $$< $$(LIB_FILE) -o $$@

$(OBJ_DIR)/$(1).o: $(SRC_DIR)/$(1)/$(1).c $(SRC_DIR)/lib/lib.h $(SRC_DIR)/lib/sys/guicall.h
	@echo "Compiling $$< -> $$@"
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
