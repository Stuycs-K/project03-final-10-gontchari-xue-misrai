compile serv cli: server.o client.o networking.o colors.h
	@gcc -o cli client.o networking.o
	@gcc -o serv server.o networking.o

server: serv
	@./serv

client: cli
	@./cli

client.o: client.c networking.h colors.h
	@gcc -c client.c

server.o: server.c networking.h colors.h
	@gcc -c server.c

pipe_networking.o: networking.c networking.h colors.h
	@gcc -c networking.c

clean:
	@rm -f *.o
	@rm -f *~
	@rm -f cli
	@rm -f *.fifo
	@rm -f serv
	@rm -f mario
