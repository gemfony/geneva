BABEL_HOME=/opt/openbabel221
BABEL_INCDIR=$(BABEL_HOME)/include/openbabel-2.0
LIBDIR=$(BABEL_HOME)/lib
BOOSTLIB=/opt/boost138/lib
BOOSTINC=/opt/boost138/include/boost-1_38/

LDFLAGS=-L$(LIBDIR) -L$(BOOSTLIB) -lopenbabel -lboost_program_options-gcc43-mt -lboost_date_time-gcc43-mt -lboost_system-gcc43-mt
CPPFLAGS=-I. -I$(BABEL_INCDIR) -I$(BOOSTINC)

all:    eminim2

eminim2: eminim2.o intcoord.o GCommandLineParser.o 


clean:
		rm -f *.o *~ eminim2
