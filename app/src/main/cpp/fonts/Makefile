obj = load.o set.o write.o ibm_fnt.o

CC = gcc

all: libsimplefonts.a

ibm_fnt.c : ibm.fnt
	xxd -i ibm.fnt > ibm_fnt.c

libsimplefonts.a : $(obj)
	ar cr libsimplefonts.a $(obj)

clean:
	rm -f *.[oa]
