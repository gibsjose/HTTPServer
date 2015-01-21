IDIR =
CC = gcc
CFLAGS = -I$(IDIR) -Wall -g -std=c99

ODIR = obj
LDIR = ../lib
LIBS = -lpthread

BIN = httpserver

_DEPS =
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = httpserver.o requesthandler.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

all: setup $(BIN)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

httpserver: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean setup

setup:
	@mkdir -p $(ODIR)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ $(BIN)
