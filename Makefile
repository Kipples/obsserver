LIBS=-lobs -lSDL2 -lulfius -lyder -lorcania -ljansson

default:
	gcc main.c $(LIBS)
