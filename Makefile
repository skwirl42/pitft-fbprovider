LDFLAGS=`libpng-config --ldflags`
CFLAGS=-I.
OBJ = fbprovider.o

CC=g++

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)

fbprovider: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

