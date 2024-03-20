
TARGET_OS ?= $(OS)

OUT := emu
DIRS := src devices dtc/libfdt
SRCS := $(shell find $(DIRS) -name "*.c" -type f)
OBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.c,%.d,$(SRCS))
CFLAGS := -Iinclude -Wall -Wextra -Idtc/libfdt
LIBS := unicorn csfml-graphics csfml-system
ifneq ($(DEBUG),)
	CFLAGS += -DDEBUG
	ifeq ($(DEBUG),m)
		CFLAGS += -DDEBUG_MEM
	endif
endif
ifeq ($(TARGET_OS),Windows_NT)
	LIBS += ws2_32
endif

all: getObjects emu

$(OUT): $(OBJS)
	$(CC) $^ $(CFLAGS) $(addprefix -l,$(LIBS)) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -o $@ -c

%.d: %.c
	$(CC) -MM $< $(CFLAGS) -MT $(patsubst %.d,%.o,$@) -o $@


clean:
	rm -f $(DEPS) $(OBJS) $(OUT)

getObjects: $(DEPS)
include $(DEPS)
