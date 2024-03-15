CFLAGS = -Wall -Werror -g
SOURCES = test_hashTab.c hashTab.c
dep : server client usage
all : clean dep
clean :
	rm server client
usage :
	@echo ""
	@echo "Usage :"
	@echo "    On the server computer, start the server:"
	@echo "       ./server port"
	@echo "    On the client computer, start the client:"
	@echo "       ./client ipserver portserver"
	@echo ""

test : $(SOURCES)
	gcc $(CFLAGS) -o $@ $^