LDFLAGS=`libpng-config --ldflags`
CFLAGS=-I. -Wall
OBJ = fbprovider.o Console.o

CC=g++

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

fbprovider: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

