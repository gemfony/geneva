BABEL_HOME=/opt/openbabel221
INCDIR=$(BABEL_HOME)/include/openbabel-2.0
LIBDIR=$(BABEL_HOME)/lib

LDFLAGS=-L$(LIBDIR) -L/opt/boost138/lib -lopenbabel -lboost_program_options-gcc43-mt
CPPFLAGS=-I. -I$(INCDIR)
CFLAGS=-I$(INCDIR)

all:    eminim2

eminim2: eminim2.o intcoord.o GCommandLineParser.o 


clean:
		rm -f *.o *~ eminim2
