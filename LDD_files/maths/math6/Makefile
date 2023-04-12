#Build

all: buildlib buildapp

buildapp:
	make -C appdir app 

buildlib:
	make -C libdir lib 


#Clean-up

clean: cleanapp cleanlib

cleanapp:
	make -C appdir clean 

cleanlib:
	make -C libdir clean 
	
