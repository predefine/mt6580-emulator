
OUT := emu
DIRS := src devices dtc/libfdt
SRCS := $(shell find $(DIRS) -name "*.c" -type f ! -name "framebuffer_*.c")
FRONTEND := raylib
CFLAGS := -Iinclude -Wall -Wextra -Idtc/libfdt
LIBS := unicorn
LDFLAGS := 
ifneq ($(DEBUG),)
	CFLAGS += -DDEBUG
	ifeq ($(DEBUG),m)
		CFLAGS += -DDEBUG_MEM
	endif
endif
include frontend/$(FRONTEND).mk
OBJS := $(patsubst %.c,%.o,$(SRCS))
DEPS := $(patsubst %.c,%.d,$(SRCS))

all: getObjects emu

$(OUT): $(OBJS)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) $(addprefix -l,$(LIBS)) -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -o $@ -c

%.d: %.c
	$(CC) -MM $< $(CFLAGS) -MT $(patsubst %.d,%.o,$@) -o $@


clean:
	rm -f $(DEPS) $(OBJS) $(OUT)

getObjects: $(DEPS)
include $(wildcard $(DEPS))
