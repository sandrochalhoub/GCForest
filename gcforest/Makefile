include ./boost_home
SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX programs
# please copy this makefile and set CPLEXDIR and CONCERTDIR to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

CPLEXDIR      = /net/phorcys/data/roc/Logiciels/CPLEX_Studio201/cplex/
CONCERTDIR    = /net/phorcys/data/roc/Logiciels/CPLEX_Studio201/concert/
BLOSSOMDIR    = /home/schalhoub/Documents/Blossom_Git/blossom

# ---------------------------------------------------------------------
# Compiler selection 
# ---------------------------------------------------------------------

CCC = g++
CC  = gcc
JAVAC = javac 

# ---------------------------------------------------------------------
# Compiler options 
# ---------------------------------------------------------------------

CCOPT = -m64 -fPIC -fno-strict-aliasing -fexceptions -DNDEBUG
COPT  = -m64 -fPIC
JOPT  = -classpath $(CPLEXDIR)/lib/cplex.jar -O

# ---------------------------------------------------------------------
# Link options and libraries
# ---------------------------------------------------------------------

CPLEXBINDIR   = $(CPLEXDIR)/bin/$(BINDIST)
CPLEXJARDIR   = $(CPLEXDIR)/lib/cplex.jar
CPLEXLIBDIR   = $(CPLEXDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

# For dynamic linking
CPLEXBINDIR   = $(CPLEXDIR)/bin/$(SYSTEM)
CPLEXLIB      = cplex$(dynamic:yes=2010)
run           = $(dynamic:yes=LD_LIBRARY_PATH=$(CPLEXBINDIR))

CCLNDIRS  = -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) $(dynamic:yes=-L$(CPLEXBINDIR))
CLNDIRS   = -L$(CPLEXLIBDIR) $(dynamic:yes=-L$(CPLEXBINDIR))
CCLNFLAGS = -lconcert -lilocplex -l$(CPLEXLIB) -lm -lpthread -ldl
CLNFLAGS  = -l$(CPLEXLIB) -lm -lpthread -ldl
JAVA      = java   -Djava.library.path=$(CPLEXDIR)/bin/x86-64_linux -classpath $(CPLEXJARDIR):

CONCERTINCDIR = $(CONCERTDIR)/include
CPLEXINCDIR   = $(CPLEXDIR)/include

SRC 	      = $(BLOSSOMDIR)/src
EXDIR         = $(BLOSSOMDIR)/examples
EXSRCCPP      = $(EXDIR)/src
EXINC         = $(SRC)/include
EXDATA        = $(BLOSSOMDIR)/data
BIN	      = $(BLOSSOMDIR)/bin
SRC           = $(SRC)/lib
OBJ	      =	$(SRC)/obj
DOC	      =	$(BLOSSOMDIR)/doc
TCL	      =	$(BLOSSOMDIR)/tools


CFLAGS  = $(COPT)  -I$(CPLEXINCDIR)
CCFLAGS = $(CCOPT) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR) 
JCFLAGS = $(JOPT)


#------------------------------------------------------------
#  make all      : to compile the examples. 
#  make execute  : to compile and execute the examples.
#------------------------------------------------------------

CPP_EX = gcforest

all_cpp: $(CPP_EX)

execute_cpp: $(CPP_EX)
	$(run) ./gcforest

# ------------------------------------------------------------

clean :
	/bin/rm -rf *.o *~ *.class
	/bin/rm -rf $(C_EX) $(CX_EX) $(CPP_EX)
	/bin/rm -rf *.mps *.ord *.sos *.lp *.sav *.net *.msg *.log *.clp

# ------------------------------------------------------------
#
# The examples
#

gcforest: gcforest.o
	$(CCC) $(CCFLAGS) $(CCLNDIRS) -o gcforest gcforest.o $(CCLNFLAGS)
gcforest.o: $(EXSRCCPP)/gcforest.cpp
	$(CCC) -c $(CCFLAGS) $(EXSRCCPP)/gcforest.cpp -o gcforest.o

# Local Variables:
# mode: makefile
# End:
