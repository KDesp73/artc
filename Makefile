# Compiler and flags
UNAME_S := $(shell uname -s)
CC = gcc
AR ?= ar
LIBRARY_NAME = artc
A_NAME = lib$(LIBRARY_NAME).a
INCLUDE = -Iinclude -Ideps/include
CFLAGS =  -Wall -fPIC $(INCLUDE)
ifeq ($(UNAME_S),Darwin)
# Homebrew: brew install sdl2 sdl2_ttf sdl2_image lua libmagic
BREW := $(shell command -v brew 2>/dev/null)
BREW_PREFIX := $(if $(BREW),$(shell $(BREW) --prefix 2>/dev/null),/opt/homebrew)
ifeq ($(shell test -d '$(BREW_PREFIX)' && echo 1),)
BREW_PREFIX := /usr/local
endif
PKGCF := $(BREW_PREFIX)/lib/pkgconfig
HAVE_SDL_PCS := $(shell export PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(PKGCF)"; \
	pkg-config --exists sdl2 SDL2_ttf SDL2_image 2>/dev/null && echo 1)
ifeq ($(HAVE_SDL_PCS),1)
SDL_DARWIN_CFLAGS := $(shell export PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(PKGCF)"; \
	pkg-config --cflags sdl2 SDL2_ttf SDL2_image 2>/dev/null)
SDL_DARWIN_LIBS := $(shell export PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:$(PKGCF)"; \
	pkg-config --libs sdl2 SDL2_ttf SDL2_image 2>/dev/null)
else
SDL_DARWIN_CFLAGS := -I$(BREW_PREFIX)/include/SDL2 -I$(BREW_PREFIX)/include
SDL_DARWIN_LIBS := -L$(BREW_PREFIX)/lib -lSDL2 -lSDL2_ttf -lSDL2_image
endif
CFLAGS += $(SDL_DARWIN_CFLAGS)
# brew --prefix lua can point to an uninstalled prefix; only use it if the library exists
# (deps/lib/liblua.a is a GNU ar build for Linux and cannot be linked on macOS; use 'brew install lua')
LUA_PFX_CAND := $(if $(BREW),$(shell $(BREW) --prefix lua 2>/dev/null),)
LUA_PFX := $(if $(LUA_PFX_CAND),$(shell t='$(LUA_PFX_CAND)'; if test -d "$$t/lib" && ( test -e "$$t/lib/liblua.a" || test -e "$$t/lib/liblua.dylib" ); then echo "$$t"; fi),)
ifeq ($(LUA_PFX),)
$(error On macOS, install Lua with Homebrew: 'brew install lua' (vendored deps/lib/liblua.a is for Linux only))
endif
LUA_DARWIN_LIBS := -L$(LUA_PFX)/lib -llua
ifeq ($(BREW),)
MAGIC_LIBDIR := $(BREW_PREFIX)/lib
else
MAGIC_LIBDIR := $(shell if test -d "$(BREW_PREFIX)/opt/libmagic/lib"; then \
	printf '%s' "$(BREW_PREFIX)/opt/libmagic/lib"; \
	else printf '%s' "$(BREW_PREFIX)/lib"; fi)
endif
LDFLAGS = $(SDL_DARWIN_LIBS) $(LUA_DARWIN_LIBS) -lm -L$(MAGIC_LIBDIR) -lmagic
EXE_LDLIBS = -L. -l$(LIBRARY_NAME)
SO_NAME = lib$(LIBRARY_NAME).dylib
SHLIB_FLAG = -dynamiclib
# Objects expect globals (scene, view) and single-translation-unit CLI; treat like a loadable module
SHLIB_DARWIN_EXTRA = -undefined dynamic_lookup
else
# Linux: vendored .so in deps; -l: needs GNU ld
LDFLAGS = -Ldeps/lib -lSDL2 -lSDL2_ttf -lSDL2_image -lm -l:liblua.a -lmagic
EXE_LDLIBS = -L. -l:$(A_NAME)
SO_NAME = lib$(LIBRARY_NAME).so
SHLIB_FLAG = -shared
SHLIB_DARWIN_EXTRA =
endif

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DIST_DIR = dist

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
	@bash ./scripts/check_deps.sh -q

$(BUILD_DIR): ## Create the build directory if it doesn't exist
	@echo "[INFO] Creating build directory"
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/lua
	mkdir -p $(BUILD_DIR)/entities
	mkdir -p $(BUILD_DIR)/assets

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c ## Compile source files with progress
	$(eval counter=$(shell echo $$(($(counter)+1))))
	@echo "[$(counter)/$(TOTAL_FILES)] Compiling $< -> $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(BUILD_DIR) static ## Build executable using static library
	@echo "[INFO] Building executable: $(TARGET)"
	@$(CC) src/main.c -o $(TARGET) $(EXE_LDLIBS) $(LDFLAGS) $(INCLUDE)

.PHONY: shared
shared: $(BUILD_DIR) $(OBJ_FILES) ## Build shared library
	@echo "[INFO] Building shared library: $(SO_NAME)"
	@$(CC) $(SHLIB_FLAG) $(CFLAGS) -o $(SO_NAME) $(OBJ_FILES) $(LDFLAGS) $(SHLIB_DARWIN_EXTRA)

.PHONY: static
static: $(BUILD_DIR) $(OBJ_FILES) ## Build static library
	@echo "[INFO] Building static library: $(A_NAME)"
	@$(AR) rcs $(A_NAME) $(OBJ_FILES)

.PHONY: clean
clean: ## Remove all build files and the executable
	@echo "[INFO] Cleaning up build directory and executable."
	rm -rf $(BUILD_DIR) $(TARGET) $(SO_NAME) $(A_NAME)

PREFIX=/usr/local
.PHONY: install
install: all ## Install the executable
	sudo cp $(TARGET) $(PREFIX)/bin
	

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

.PHONY: autocomplete
autocomplete: ## Generate autocomplete scripts for bash, zsh and fish shells
	complgen  --zsh ./docs/autocomplete/_artc.zsh  ./docs/autocomplete/artc.usage
	complgen --bash ./docs/autocomplete/_artc.bash ./docs/autocomplete/artc.usage
	complgen --fish ./docs/autocomplete/_artc.fish ./docs/autocomplete/artc.usage

.PHONY: help
help: ## Show this help message
	@echo "Available commands:"
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
