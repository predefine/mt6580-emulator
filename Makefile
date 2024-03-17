
OUT := emu
DIRS := src devices dtc/libfdt
SRCS := $(shell find $(DIRS) -name "*.c" -type f)
OBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.c,%.d,$(SRCS))
CFLAGS := -Iinclude -Wall -Wextra -lunicorn -lSDL2 -Idtc/libfdt
ifneq ($(DEBUG),)
	CFLAGS += -DDEBUG
	ifeq ($(DEBUG),m)
		CFLAGS += -DDEBUG_MEM
	endif
endif

all: getObjects emu

$(OUT): $(OBJS)
	gcc $^ $(CFLAGS) -o $@

%.o: %.c
	gcc $< $(CFLAGS) -o $@ -c

%.d: %.c
	gcc -MM $< $(CFLAGS) -MT $(patsubst %.d,%.o,$@) -o $@


clean:
	rm -f $(DEPS) $(OBJS)

getObjects: $(DEPS)
include $(wildcard $(DEPS))
