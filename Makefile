build:
	gcc server.c common.c -o server -I. -g3 -Wall -Wextra -Werror $(CFLAGS)
	gcc client.c common.c -o client -I. -g3 -Wall -Wextra -Werror $(CFLAGS)
clean:
	rm -f server client
