all: dis decrypt dumpvice

dis: dis.o ../vm/common.o
decrypt: decrypt.o ../vm/common.o
dumpvice: dumpvice.o

clean:
	rm -f dis decrypt dumpvice *.o
