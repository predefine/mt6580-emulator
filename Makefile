
OUT := emu
OBJECTS := $(patsubst %.c,%.o,$(shell find -name "*.c" -type f ! -path "./test/*"))
CFLAGS := -I. -Wall -Wextra -lunicorn -lSDL2
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
