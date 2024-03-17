
OUT := emu
DIRS := src devices dtc/libfdt
SRCS := $(shell find $(DIRS) -name "*.c" -type f)
OBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.c,%.d,$(SRCS))
CFLAGS := -Iinclude -Wall -Wextra -lunicorn -lcsfml-graphics -lcsfml-system -Idtc/libfdt
ifneq ($(DEBUG),)
	CFLAGS += -DDEBUG
	ifeq ($(DEBUG),m)
		CFLAGS += -DDEBUG_MEM
	endif
endif

all: getObjects emu

$(OUT): $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -o $@ -c

%.d: %.c
	$(CC) -MM $< $(CFLAGS) -MT $(patsubst %.d,%.o,$@) -o $@


clean:
	rm -f $(DEPS) $(OBJS) $(OUT)

getObjects: $(DEPS)
include $(DEPS)
