CC			= gcc
CFLAGS		= -Wall	-Wextra	-g
LDFLAGS		= -lpthread

all:	server	client

server:	server.c	httpserver.c	filemanage.c
		$(CC)	$(CFLAGS)	-o	server	server.c	httpserver.c	filemanage.c	$(LDFLAGS)
		@mkdir	-p	server_files

client:	client.c
		$(CC)	$(CFLAGS)	-o	client	client.c 

clean:	
		rm	-f	server	client