CFLAGS =

isfix32.dll : isfix32.c
	$(CC) /LD isfix32.c detours32.lib user32.lib

isfixload.exe : isfixload.c
	$(CC) isfixload.c detours32.lib user32.lib

all: isfix32.dll isfixload.exe
