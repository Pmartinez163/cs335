CFLAGS = -I../include -pthread -O1
LIB = ../lib/x11/libglfw.a
LFLAGS = $(LIB) -lrt -lX11 -lGLU -lGL -pthread -lm 

all: midterm

midterm: midterm.o ../lib/x11/libglfw.a
	g++ midterm.o $(LFLAGS) -o midterm

midterm.o: midterm.cpp
	g++ -c $(CFLAGS) midterm.cpp

clean:
	rm -f midterm
	rm -f *.o
