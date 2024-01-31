all: vm tel

vm: vm.o

tel: tel.cpp
	clang -O2 -o tel tel.cpp -lstdc++

clean:
	rm vm tel *.o
