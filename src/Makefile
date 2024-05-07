USE_WAYLAND_DISPLAY   ?= TRUE

CC = gcc
CFLAGS = -std=c99 -Wall -Wno-missing-braces -Wunused-result -D_DEFAULT_SOURCE
CFLAGS += -s -O2

LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt

ifeq ($(USE_WAYLAND_DISPLAY),TRUE)
  LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
endif

all: main.c
	$(CC) $< -o ../build/pajaros_locos.exe $(CFLAGS) $(LDLIBS)
