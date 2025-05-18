# Compiler and flags
CC = gcc
INCLUDE = -Iinclude -Ideps/include
CFLAGS = -Wall -fPIC $(INCLUDE)
LDFLAGS = -Ldeps/lib -lSDL2 $(shell pkg-config --libs sdl2) -lm

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DIST_DIR = dist

LIBRARY_NAME = artc
SO_NAME = lib$(LIBRARY_NAME).so
A_NAME = lib$(LIBRARY_NAME).a

# Target and version info
TARGET = $(LIBRARY_NAME)
version_file = include/version.h
VERSION_MAJOR = $(shell sed -n -e 's/\#define VERSION_MAJOR \([0-9]*\)/\1/p' $(version_file))
VERSION_MINOR = $(shell sed -n -e 's/\#define VERSION_MINOR \([0-9]*\)/\1/p' $(version_file))
VERSION_PATCH = $(shell sed -n -e 's/\#define VERSION_PATCH \([0-9]*\)/\1/p' $(version_file))
VERSION = $(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

# Determine the build type
ifeq ($(type), RELEASE)
	CFLAGS += -O3
else
	# SANITIZERS = -fsanitize=address,undefined
	CFLAGS  += -DDEBUG -ggdb
	CFLAGS  += $(SANITIZERS)
	LDFLAGS += $(SANITIZERS)
endif

# Source and object files
SRC_FILES := $(shell find $(SRC_DIR) -name '*.c' ! -name 'main.c')
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Default target
.DEFAULT_GOAL := help

# Total source file count
TOTAL_FILES := $(words $(SRC_FILES))

# Counter to track progress
counter = 0

# Targets

.PHONY: all
all: check_tools $(BUILD_DIR) static shared $(TARGET)## Build the project
	@echo "Build complete."

.PHONY: check_tools
check_tools: ## Check if necessary tools are available
	@command -v gcc >/dev/null 2>&1 || { echo >&2 "[ERRO] gcc is not installed."; exit 1; }
	@command -v bear >/dev/null 2>&1 || { echo >&2 "[WARN] bear is not installed. Skipping compile_commands.json target."; }

$(BUILD_DIR): ## Create the build directory if it doesn't exist
	@echo "[INFO] Creating build directory"
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c ## Compile source files with progress
	$(eval counter=$(shell echo $$(($(counter)+1))))
	@echo "[$(counter)/$(TOTAL_FILES)] Compiling $< -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(BUILD_DIR) static ## Build executable using static library
	@echo "[INFO] Building executable: $(TARGET)"
	@$(CC) src/main.c -o $(TARGET) -L. -l:$(A_NAME) $(LDFLAGS) $(INCLUDE)

.PHONY: shared
shared: $(BUILD_DIR) $(OBJ_FILES) ## Build shared library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) -shared $(CFLAGS) -o $(SO_NAME) $(OBJ_FILES)

.PHONY: static
static: $(BUILD_DIR) $(OBJ_FILES) ## Build static library
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

.PHONY: clean
clean: ## Remove all build files and the executable
	@echo "[INFO] Cleaning up build directory and executable."
	rm -rf $(BUILD_DIR) $(TARGET) $(SO_NAME) $(A_NAME)

.PHONY: distclean
distclean: clean ## Perform a full clean, including backup and temporary files
	@echo "[INFO] Performing full clean, removing build directory, dist files, and editor backups."
	rm -f *~ core $(SRC_DIR)/*~ $(DIST_DIR)/*.tar.gz

.PHONY: dist
dist: $(SRC_FILES) ## Create a tarball of the project
	@echo "[INFO] Creating a tarball for version $(VERSION)"
	mkdir -p $(DIST_DIR)
	tar -czvf $(DIST_DIR)/$(TARGET)-$(VERSION).tar.gz $(SRC_DIR) $(INCLUDE_DIR) Makefile README.md

.PHONY: compile_commands.json
compile_commands.json: $(SRC_FILES) ## Generate compile_commands.json
	@echo "[INFO] Generating compile_commands.json"
	bear -- make all

.PHONY: help
help: ## Show this help message
	@echo "Available commands:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
