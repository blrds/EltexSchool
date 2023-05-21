static:
	gcc -c operations.c
	ar rc libop.a operations.o
	ranlib libop.a
	gcc main.c -L. -lop -o mains.o
pdynamic:
	gcc -fPIC -c operations.c
	gcc -shared -o liboppd.so operations.o
	gcc -c operations.c
	ar rc liboppd.a operations.o
	ranlib liboppd.a
	gcc main.c -static -L. -loppd -o mainpd.o
dynamic:
	gcc -fPIC -c operations.c
	gcc -shared -o libopd.so operations.o
	gcc main.c -L. -lopd -o maind.o
	LD_LIBRARY_PATH=""
	LD_LIBRARY_PATH=/~/Desktop/EltexSchool/5/4/2
	export LD_LIBRARY_PATH
clean: 
	rm -rf *.o
	rm -rf *.so
	rm -rf *.a
	LD_LIBRARY_PATH=""