POLY = 1
START = 4000

.SUFFIXES: .asm

CFLAGS = -DPOLY=$(POLY)
ifneq ($(POLY),0)
OBJS = p.o table.o
else
OBJS = m.o table.o
endif

all: a.bin

a.bin: $(OBJS)
	ld68 -b -C 0x$(START) $(OBJS)
	dd if=a.out of=$@ bs=0x$(START) skip=1
	f9dasm -6800 -offset $(START) $@ > a.lst

table.o: mml
	./mml > table.s
	as68 -l table.lst table.s

.asm.o:
	as68 -l $(<:.asm=.lst) $<

clean:
	rm -f a.{bin,out} *.{lst,o,s} mml
