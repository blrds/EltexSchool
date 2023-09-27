all:
	gcc -c Message.pb-c.c -o mes.o
	ar rc libmes.a mes.o
	ranlib libmes.a
	gcc server.c -L. -lmes -o server.o
clean:
	rm -rf *.o
	rm -rf *.a