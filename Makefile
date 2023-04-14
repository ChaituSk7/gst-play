CC = g++
LIBS = `pkg-config --cflags --libs gstreamer-1.0`

all: lib.o gplay.so exe

lib.o:	lib.cpp
	$(CC) -c lib.cpp $(LIBS) -fPIC
gplay.so:	lib.o
	$(CC) -shared -o libgplay.so lib.o $(LIBS)
exe: main.cpp 
	$(CC) -o exe main.cpp -lgplay $(LIBS) -I . -L .
clean:
	rm -rf *.o *.so exe
