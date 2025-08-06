CC = gcc
CFLAGS = -Wall -Wextra -Weffc++ -Wcast-qual -Wchar-subscripts -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat -Wformat-nonliteral -Wformat-security -Wmissing-declarations -Wnon-virtual-dtor -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-aliasing -Wstrict-overflow -Wswitch-default -Wswitch-enum -Wunused -std=c99
BUILD_DIR = build
TARGET = memalloc

SRCS = main.c memory.c
OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

rebuild:
	make clean
	make

.PHONY: all clean
