BIN = sws
OBJ = net.o \
	  handle.o \
	  handle_static.o \
	  handle_cgi.o \
	  handle_other.o \
	  public.o \
	  HTTP_parser.o \
	  sws.o

#CFLAGS = -g -Wall -pedantic-errors
CFLAGS = -g -Wall
LIB = -pthread

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $^ $(LIB) -lbsd

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	-rm -fr $(BIN) $(OBJ)
