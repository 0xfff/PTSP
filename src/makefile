
CC = g++-4.7 -std=c++11 -fgnu-tm -o3
CFLAGS = -g 
LFLAGS = -fopenmp -lpthread 
SOURCES = dsf.cpp tsp.c

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE = tsp

all:$(SOURCES) $(EXECUTABLE)

$(EXECUTABLE):$(OBJECTS)
	$(CC) -o$@	 $(OBJECTS) $(LFLAGS)

clean:
	-rm  -f $(OBJECTS) 

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<
.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

#g++ -g  dsf.cpp tsp.c -o tsp
