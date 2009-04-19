BABEL_HOME=/opt/openbabel221
BABEL_INCDIR=$(BABEL_HOME)/include/openbabel-2.0
LIBDIR=$(BABEL_HOME)/lib
BOOSTLIB=/opt/boost138/lib
BOOSTINC=/opt/boost138/include/boost-1_38/
GENEVAINC=/opt/geneva/include/
GENEVALIB=/opt/geneva/lib/

LDFLAGS=-L$(LIBDIR) -L$(BOOSTLIB) -L$(GENEVALIB) -lgeneva -lopenbabel -lboost_program_options-gcc43-mt -lboost_date_time-gcc43-mt -lboost_system-gcc43-mt -lboost_serialization-gcc43-mt -lboost_thread-gcc43-mt
CPPFLAGS=-I. -I$(BABEL_INCDIR) -I$(BOOSTINC) -I$(GENEVAINC) -O3

all:    eminim2

eminim2: eminim2.o intcoord.o GParser.o 


clean:
		rm -f *.o *~ eminim2
