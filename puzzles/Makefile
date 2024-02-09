all: orb tel
orb: orb.o
tel: tel.cpp
	clang -O2 -o tel tel.cpp -lstdc++

clean:
	rm -f orb tel *.o
