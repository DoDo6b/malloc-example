CC = gcc
CFLAGS = -Wall -Wextra -Wcast-qual -Wchar-subscripts -Wconversion \
         -Wempty-body -Wfloat-equal -Wformat -Wformat-nonliteral \
         -Wformat-security -Wmissing-declarations -Wpointer-arith \
         -Wredundant-decls -Wshadow -Wstrict-aliasing -Wstrict-overflow \
         -Wswitch-default -Wswitch-enum -Wunused -std=c99

BUILD_DIR = .build
TARGET = execute

SRC_DIRS = . allocator GC logger
SRCS = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

DIRS = $(foreach dir,$(SRC_DIRS),$(BUILD_DIR)/$(dir))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
