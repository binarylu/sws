BIN = sws
OBJ = net.o \
	  handle.o \
	  handle_static.o \
	  handle_cgi.o \
	  handle_other.o \
	  HTTP_parser.o \
	  sws.o

CFLAGS = -g -Wall -pedantic-errors
LIB = -pthread

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $^ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	-rm -fr $(BIN) $(OBJ)