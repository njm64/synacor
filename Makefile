all: vm tel orb dis

vm: vm.o common.o
dis: dis.o common.o
orb: orb.o

tel: tel.cpp
	clang -O2 -o tel tel.cpp -lstdc++

clean:
	rm vm tel *.o
