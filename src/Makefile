USE_WAYLAND_DISPLAY   ?= FALSE
CYAN = FALSE

NOMBRE = PAJAROS_ENOJONES
EXT = -linux64.out

CC = gcc
FLAGS = -std=c99 -Wall -Wno-missing-braces -Wunused-result -D_DEFAULT_SOURCE
CFLAGS += -s -O2

ifeq ($(OS),Windows_NT)
	CYAN := TRUE
endif

ifeq ($(CYAN),TRUE)
	LDLIBS 	= -I ./libs/raylib-mingw64/include/ -L ./libs/raylib-mingw64/lib/
	LDLIBS += -lraylib -lopengl32 -lgdi32 -lwinmm
	LDLIBS += -static -lpthread
	EXT =  -win64.exe

else ifeq ($(MINGW), TRUE)
	CC = x86_64-w64-mingw32-gcc
	LDLIBS = -I ./libs/raylib-mingw64/include/ -L ./libs/raylib-mingw64/lib/
	LDLIBS += -lraylib -lopengl32 -lgdi32 -lwinmm
	LDLIBS += -static -lpthread
	EXT =  -win64.exe
else
	LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt
	LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
endif


all: main.c
	$(CC) $< -o ../build/$(NOMBRE)$(EXT) $(CFLAGS) $(LDLIBS)
	@echo "BUILT: '../build/$(NOMBRE)$(EXT)'"
