CFLAGS = -g -Wall
LIB = -lmagic

sws: handle.o handle_cgi.o handle_response.o handle_static.o HTTP_parser.o index.o net.o public.o sws.o
	$(CC) -o sws handle.o handle_cgi.o handle_response.o handle_static.o HTTP_parser.o index.o net.o public.o sws.o $(LIB)

handle.o: handle.c
	$(CC) $(CFLAGS) -c -o handle.o handle.c
handle_cgi.o: handle_cgi.c
	$(CC) $(CFLAGS) -c -o handle_cgi.o handle_cgi.c
handle_response.o: handle_response.c
	$(CC) $(CFLAGS) -c -o handle_response.o handle_response.c
handle_static.o: handle_static.c
	$(CC) $(CFLAGS) -c -o handle_static.o handle_static.c
HTTP_parser.o: HTTP_parser.c
	$(CC) $(CFLAGS) -c -o HTTP_parser.o HTTP_parser.c
index.o: index.c
	$(CC) $(CFLAGS) -c -o index.o index.c
net.o: net.c
	$(CC) $(CFLAGS) -c -o net.o net.c
public.o: public.c
	$(CC) $(CFLAGS) -c -o public.o public.c
sws.o: sws.c
	$(CC) $(CFLAGS) -c -o sws.o sws.c

.PHONY: clean
clean:
	-rm -fr sws *.o magic.mgc
