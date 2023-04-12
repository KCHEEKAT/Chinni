#Build

all: buildapp
	gcc appdir/math.o -Llibdir -lmathop -o math

buildapp: buildlib
	make -C appdir app 

buildlib:
	make -C libdir lib 


#Clean-up

clean: cleanapp cleanlib
	rm -f math

cleanapp:
	make -C appdir clean 

cleanlib:
	make -C libdir clean 
	
