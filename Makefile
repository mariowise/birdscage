INCLUDE = -I src/
OUTPUT = ./bin/birds.run
LINKS = -lm -lGL -lglut -lpthread

CC = mpicc
CCRUN = mpirun
CFLAG = -c -w

main: build/main.o build/getpar.o build/mglut.o build/birds.o
	@ $(CC) -o $(OUTPUT) build/*.o $(LINKS)
	@ printf "Sources compiled into "
	@ printf $(OUTPUT)
	@ printf "\n"

build/main.o: src/main.c
	@ printf "  cc src/main.c "
	@ $(CC) $(CFLAG) -o build/main.o src/main.c $(INCLUDE) $(LINKS)
	@ printf "OK\n"

build/getpar.o: src/getpar.c
	@ printf "  cc src/getpar.c "
	@ $(CC) $(CFLAG) -o build/getpar.o src/getpar.c $(INCLUDE)
	@ printf "OK\n"

build/mglut.o: src/glut/mglut.c
	@ printf "  cc src/glut/mglut.c "
	@ $(CC) $(CFLAG) -o build/mglut.o src/glut/mglut.c $(INCLUDE)
	@ printf "OK\n"

build/birds.o: src/birds/birds.c
	@ printf "  cc src/birds/birds.c "
	@ $(CC) $(CFLAG) -o build/birds.o src/birds/birds.c $(INCLUDE)
	@ printf "OK\n"


clean:
	@ clear
	@ echo "Cleanning"
	@ rm -r build/*.o
	@ rm -r bin/*.run

run:
	@ clear
	@ $(CCRUN) -np 3 $(OUTPUT) -N 10 -I 100 -f aves.txt

me: clean main run