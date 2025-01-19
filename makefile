compile serv cli: server.o client.o networking.o colors.h
	@gcc -o cli client.o networking.o -lncurses
	@gcc -o serv server.o networking.o -lncurses

server: serv
	@./serv

client: cli
	@./cli

client.o: client.c universal.h colors.h client.h
	@gcc -c client.c 

server.o: server.c universal.h colors.h
	@gcc -c server.c 

networking.o: networking.c universal.h colors.h
	@gcc -c networking.c

clean:
	@rm -f *.o
	@rm -f *~
	@rm -f cli
	@rm -f *.fifo
	@rm -f serv
	@rm -f mario
