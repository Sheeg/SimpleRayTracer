
BIN := raytracer

CC := g++
LD := g++

SRC_EXT := cpp

SRC_DIR := src
OBJ_DIR := obj


CFLAGS := -Wall -Wextra -std=c++11 -g -Ilib -O3
LDFLAGS = 

# OS specific flags
ifeq ($(OS),Windows_NT)
	# Windows
    LDFLAGS += 
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		# Linux
		LDFLAGS += -lm 
		CFLAGS +=
	endif
	ifeq ($(UNAME_S),Darwin)
		# OSX
		# CoreFoundation API is needed for SOIL
		LDFLAGS += -L/usr/local/lib
		CFLAGS += -D__APPLE__
	endif
endif

SOURCES := $(shell find $(SRC_DIR)/ -name '*.$(SRC_EXT)')
OBJECTS := $(SOURCES:$(SRC_DIR)/%.$(SRC_EXT)=%.o)
OBJECTS := $(OBJECTS:%.o=$(OBJ_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)

.PHONY: all
all: $(BIN)

# link objects together to make executable
# LDFLAGS need to come at the end on ubuntu
$(BIN) : $(OBJECTS)
	$(LD) -o $(BIN) $(OBJECTS) $(LDFLAGS)

# make objects and dependencies
$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT)
#	if the obj_dir exists -p suppresses the error
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -MP -MMD $< -o $@

# handle dependencies
-include $(DEPS)

# remove the compiled objects and the binary to clean up
.PHONY: clean
clean:
	rm -f $(OBJECTS) $(BIN) $(DEPS)

# archive the project
.PHONY: archive
archive:
	zip  -r $(BIN) $(SRC_DIR) res lib obj Makefile README.md

# delete the archive
.PHONY: del_archive
del_archive:
	rm -f $(BIN).zip
