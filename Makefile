CC = g++
path = src
LIBS = `pkg-config --cflags --libs gstreamer-1.0`

all: lib.o gplay.so exe

lib.o:	$(path)/lib.cpp
	$(CC) -c $(path)/lib.cpp $(LIBS) -fPIC -I ./include/
	
gplay.so:	lib.o
	$(CC) -shared -o libgplay.so lib.o $(LIBS)

exe: $(path)/main.cpp 
	$(CC) -o exe $(path)/main.cpp -lgplay $(LIBS) -I ./include/ -L .

clean:
	rm -rf *.o *.so exe
