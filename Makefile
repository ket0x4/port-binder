build:
	gcc server.c common.c -o server -I. -g3
	gcc client.c common.c -o client -I. -g3