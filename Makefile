
OUT := emu
DIRS := src devices dtc/libfdt
OBJECTS := $(patsubst %.c,%.o,$(shell find $(DIRS) -name "*.c" -type f))
CFLAGS := -Iinclude -Wall -Wextra -lunicorn -lSDL2 -Idtc/libfdt
ifneq ($(DEBUG),)
	CFLAGS += -DDEBUG
endif
ifeq ($(DEBUG),m)
	CFLAGS += -DDEBUG_MEM
endif

$(OUT): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(OUT)
