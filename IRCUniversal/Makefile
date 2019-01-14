CFLAGS = -g

IRCUniversal : main.o client.o server.o errorCheck.o networkHelp.o communication.o build

main.o : main.c main.h client.h server.h
	gcc $(CFLAGS) -c main.c
client.o : client.c client.h errorCheck.h networkHelp.h
	gcc $(CFLAGS) -c client.c
server.o : server.c server.h errorCheck.h networkHelp.h
	gcc $(CFLAGS) -c server.c
errorCheck.o : errorCheck.c errorCheck.h
	gcc $(CFLAGS) -c errorCheck.c
networkHelp.o : networkHelp.c networkHelp.h
	gcc $(CFLAGS) -c networkHelp.c
communication.o : communication.c communication.h
    gcc &(CFLAGS) -c communication.c
build : 
	gcc -Wall *.o -o IRCUniversal
	rm *.o
clean :
	rm IRCUniversal
