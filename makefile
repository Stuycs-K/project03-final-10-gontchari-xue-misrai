compile serv cli: server.o client.o pipe_networking.o colors.h
	@gcc -o cli client.o pipe_networking.o
	@gcc -o serv server.o pipe_networking.o

server: serv
	@./serv

client: cli
	@./cli

client.o: client.c pipe_networking.h colors.h
	@gcc -c client.c

server.o: server.c pipe_networking.h colors.h
	@gcc -c server.c

pipe_networking.o: pipe_networking.c pipe_networking.h colors.h
	@gcc -c pipe_networking.c

clean:
	@rm -f *.o
	@rm -f *~
	@rm -f cli
	@rm -f *.fifo
	@rm -f serv
	@rm -f mario
