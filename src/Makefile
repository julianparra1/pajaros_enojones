USE_WAYLAND_DISPLAY   ?= FALSE
LUIS = FALSE

CC = gcc
CFLAGS = -std=c99 -Wall -Wno-missing-braces -Wunused-result -D_DEFAULT_SOURCE
CFLAGS += -s -O2

ifeq ($(LUIS),TRUE)
        LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
        LDLIBS += -static -lpthread
else
        LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt
        LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
endif


all: main.c
	$(CC) $< -o ../build/pajaros_locos.exe $(CFLAGS) $(LDLIBS)
	@echo "\033[96;1mSi eres luis... y lees esto... tienes que poner '../build/pajaros_locos.exe' en tu linea de comandos\033[0m"
