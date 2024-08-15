CC    := gcc

_DIR_INC := includes/
_DIR_SRC := src/
_DIR_BLD := build/
_DIR_MODULES := vkCore/ vkExample/

INCLUDE := -I$(_DIR_INC)
SOURCES := $(foreach module,$(_DIR_MODULES),$(wildcard $(_DIR_SRC)$(module)*.c))
OBJECTS := $(patsubst $(_DIR_SRC)%.c,$(_DIR_BLD)%.o,$(SOURCES))
TARGET  := Example

LIBRARIES := -lm -lSDL2 -lvulkan -lubsan

CFLAGS := -g -Wall -Wextra -fsanitize=undefined $(INCLUDE)

# Create build directories if they do not exist
$(shell mkdir -p $(addprefix $(_DIR_BLD), $(_DIR_MODULES)))

$(_DIR_BLD)$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LIBRARIES)

# Rule to compile source files into object files
$(_DIR_BLD)%.o: $(_DIR_SRC)%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean debug run

debug: 
	cd build && gdb ./Example && cd ..;

run:
	cd build && ./Example && cd ..;

clean:
	rm -f $(_DIR_BLD)$(TARGET) $(OBJECTS)
