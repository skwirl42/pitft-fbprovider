LDFLAGS=`libpng-config --ldflags` -lrt
CFLAGS=-I. -Wall
OBJ = fbprovider.o Console.o ConsoleFBRenderer.o

CC=g++

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

fbprovider: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

