lib:
	gcc -c add.c -o add.o 
	gcc -c subtract.c -o subtract.o
	gcc -c multiply.c -o multiply.o
	gcc -c divide.c -o divide.o
	ar rcs libmathop.a add.o subtract.o multiply.o divide.o
	
clean:
	rm add.o subtract.o multiply.o divide.o libmathop.a
