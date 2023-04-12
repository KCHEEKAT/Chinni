lib:
	gcc -c add.c -o add.o 
	gcc -c subtract.c -o subtract.o
	gcc -c multiply.c -o multiply.o
	gcc -c divide.c -o divide.o
	gcc -shared add.o subtract.o multiply.o divide.o -o libmathop.so
	
clean:
	rm -f add.o subtract.o multiply.o divide.o libmathop.so
