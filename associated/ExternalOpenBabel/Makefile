BABEL_HOME=/opt/openbabel221
INCDIR=$(BABEL_HOME)/include/openbabel-2.0
LIBDIR=$(BABEL_HOME)/lib
BOOSTDIR=/opt/boost138/lib

LDFLAGS=-L$(LIBDIR) -L$(BOOSTDIR) -lopenbabel -lboost_program_options-gcc43-mt -lboost_date_time-gcc43-mt -lboost_system-gcc43-mt
CPPFLAGS=-I. -I$(INCDIR)
CFLAGS=-I$(INCDIR)

all:    eminim2

eminim2: eminim2.o intcoord.o GCommandLineParser.o 


clean:
		rm -f *.o *~ eminim2
