
OUT := emu
OBJECTS := $(patsubst %.c,%.o,$(shell find -name "*.c" -type f ! -path "./test/*"))
CFLAGS := -I. -Wall -Wextra -lunicorn
ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG
endif

$(OUT): $(OBJECTS)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f $(OBJECTS) $(OUT)
