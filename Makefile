all:
	gcc phoenix.c http_parser.c -o phoenix -llthread -lpthread
