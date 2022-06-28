include ../boost_home
SYSTEM     = x86-64_linux
LIBFORMAT  = static_pic

#------------------------------------------------------------
#
# When you adapt this makefile to compile your CPLEX programs
# please copy this makefile and set CPLEXDIR and CONCERTDIR to
# the directories where CPLEX and CONCERT are installed.
#
#------------------------------------------------------------

CPLEXDIR      = /net/phorcys/data/roc/Logiciels/CPLEX_Studio201/cplex
CONCERTDIR    = /net/phorcys/data/roc/Logiciels/CPLEX_Studio201/concert
BLOSSOMDIR    = ../.


BIN=$(BLOSSOMDIR)/gcforest/bin
SRC=$(BLOSSOMDIR)/src/lib
OBJ=$(BLOSSOMDIR)/src/obj
INC=$(BLOSSOMDIR)/src/include
TCL=$(BLOSSOMDIR)/tools


PINCSRC = $(wildcard $(INC)/*.hpp)
PLIBSRC = $(wildcard $(SRC)/*.cpp)
PLIBAUX = $(PLIBSRC:.cpp=.o)
PLIBOBJ = $(patsubst $(SRC)/%, $(OBJ)/%, $(PLIBAUX))


# ---------------------------------------------------------------------
# Compiler selection 
# ---------------------------------------------------------------------

CCC = g++
CC  = gcc
JAVAC = javac 

# ---------------------------------------------------------------------
# Compiler options 
# ---------------------------------------------------------------------

CCOPT = -m64 -fPIC -fno-strict-aliasing -fexceptions -Wno-sign-compare -c #-DNDEBUG

COMPILFLAGS= -std=c++1z -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

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

# SRC 	      = $(BLOSSOMDIR)/src
# EXDIR         = $(BLOSSOMDIR)/examples
# EXSRCCPP      = $(EXDIR)/src
# EXINC         = $(SRC)/include
# EXDATA        = $(BLOSSOMDIR)/data
# BIN	      = $(BLOSSOMDIR)/bin
# SRC           = $(SRC)/lib
# OBJ	      =	$(SRC)/obj
# DOC	      =	$(BLOSSOMDIR)/doc
# TCL	      =	$(BLOSSOMDIR)/tools



# CFLAGS = -I$(INC) -I$(TCL) -Wall -I$(BOOSTDIR) -fPIC -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR # -ffloat-store
LFLAGS = -L$(OBJ) -flto #--static

CFLAGS  = -I$(INC) -I$(TCL) -Wall -I$(BOOSTDIR)
CCFLAGS = $(CFLAGS) -I$(CPLEXINCDIR) -I$(CONCERTINCDIR)



%.o:			CFLAGS +=$(CCOPT) $(COMPILFLAGS)

#------------------------------------------------------------
#  make all      : to compile the examples. 
#  make execute  : to compile and execute the examples.
#------------------------------------------------------------

CPP_EX = gcforest #blend

all_cpp: lib $(CPP_EX)

# ------------------------------------------------------------

clean :
	/bin/rm -rf *.o *~ *.class
	/bin/rm -rf $(C_EX) $(CX_EX) $(CPP_EX)
	/bin/rm -rf *.mps *.ord *.sos *.lp *.sav *.net *.msg *.log *.clp
	
	
	
	
lib: $(PLIBOBJ) $(PUTIOBJ)
$(OBJ)/%.o:  $(SRC)/%.cpp $(INC)/%.hpp
	@echo 'compile '$<
	$(CCC) $(CFLAGS) -c $< -o $@
	

# ------------------------------------------------------------
#
# The examples
#

gcforest: gcforest.o
	$(CCC) $(CCFLAGS) $(CCLNDIRS) $(PLIBOBJ) $(LFLAGS) -o gcforest gcforest.o $(CCLNFLAGS)
gcforest.o: gcforest.cpp
	$(CCC) -c $(CCFLAGS) gcforest.cpp -o gcforest.o

# Local Variables:
# modtt
# makefile
