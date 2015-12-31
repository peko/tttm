IDIR =./inc
CC=clang
CFLAGS=-I$(IDIR) -O3

SDIR=src
ODIR=obj
LDIR =./lib

LIBS=-lm -lproj -lshp

_DEPS = mesh.h shapes.h types.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = tttm.o mesh.o shapes.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tttm: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 