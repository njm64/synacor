all: vm tel orb dis decrypt dumpvice

vm: vm.o common.o
dis: dis.o common.o
decrypt: decrypt.o common.o
orb: orb.o
dumpvice: dumpvice.o

tel: tel.cpp
	clang -O2 -o tel tel.cpp -lstdc++

clean:
	rm vm tel *.o
