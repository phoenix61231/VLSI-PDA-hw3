CC = gcc
CFLAGS = -Wall -ansi
SOURCES = main.c read_input.c find_floorplan.c
HEADERS = $(SOURCES:.c=.h)
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = ../bin/hw3

$(EXECUTABLE): $(OBJECTS)
	$(CC) -o $(EXECUTABLE) $(OBJECTS) -lm -I/usr/include/cairo/ -L/usr/lib -lcairo

main.o : main.c $(HEADERS)
	$(CC) $(CFLAGS) -c main.c -I/usr/include/cairo/ -L/usr/lib -lcairo

read_input.o : read_input.c read_input.h main.h
	$(CC) $(CFLAGS) -c read_input.c -I/usr/include/cairo/ -L/usr/lib -lcairo

find_floorplan.o : find_floorplan.c find_floorplan.h main.h
	$(CC) $(CFLAGS) -c find_floorplan.c -I/usr/include/cairo/ -L/usr/lib -lcairo

clean:
	rm $(EXECUTABLE) $(OBJECTS) 
