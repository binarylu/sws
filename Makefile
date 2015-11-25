BIN = sws
OBJ = net.o \
	  handle.o \
	  handle_response.o \
	  handle_static.o \
	  handle_cgi.o \
	  handle_other.o \
	  public.o \
	  HTTP_parser.o \
	  sws.o

#CFLAGS = -g -Wall -pedantic-errors
CFLAGS = -g -Wall
#LIB = -pthread

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $^ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test_handle: 
	$(CC) -o test handle_cgi.c public.c test_request.c HTTP_parser.c handle_response.c
.PHONY: clean
clean:
	-rm -fr $(BIN) $(OBJ)
