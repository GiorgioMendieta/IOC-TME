CFLAGS = -Wall -Werror -g
SOURCES = hashTab.c

dep : server client usage
all : clean dep
clean :
	rm server client hashTab.o test test_hashTab.o
usage :
	@echo ""
	@echo "Usage :"
	@echo "    On the server computer, start the server:"
	@echo "       ./server port"
	@echo "    On the client computer, start the client:"
	@echo "       ./client ipserver portserver"
	@echo ""

server: hashTab.o server.o
	gcc -o $@ $^

hashTab.o: hashTab.c
	gcc -c $^

server.o: server.c
	gcc -c $^

#test : $(SOURCES)
#	gcc -g -o $@ $^