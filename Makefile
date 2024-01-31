all: vm tel orb

vm: vm.o
orb: orb.o

tel: tel.cpp
	clang -O2 -o tel tel.cpp -lstdc++

clean:
	rm vm tel *.o
