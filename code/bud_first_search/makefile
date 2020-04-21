include ../boost_home

WRAPRDIR ?= .
MAINDIR ?= ..

PYINC = "/usr/include/python3.5"

CCC = g++

SRC=$(WRAPRDIR)/wrapper/src
OBJ=$(WRAPRDIR)/wrapper/obj
INC=$(WRAPRDIR)/wrapper/include
SWIG=$(WRAPRDIR)/wrapper/swig
MAININC=$(MAINDIR)/src/include

PWRAPSRC = $(wildcard $(SRC)/*.cpp)
PWRAPSWIG = $(wildcard $(SWIG)/*.i)
PWRAPAUX = $(PWRAPSRC:.cpp=.o)
PWRAPSWIGAUX = $(PWRAPSWIG:.i=_wrap.o)
PWRAPOBJ = $(patsubst $(SRC)/%, $(OBJ)/%, $(PWRAPAUX))
PWRAPSWIGOBJ = $(patsubst $(SWIG)/%, $(OBJ)/%, $(PWRAPSWIGAUX))
PLIBOBJ=$(wildcard $(MAINDIR)/src/obj/*.o)

wrapper/_budFirstSearch.so: $(PWRAPOBJ) $(PWRAPSWIGOBJ)
	$(CCC) -std=c++11 -shared $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(PLIBOBJ) -o wrapper/_budFirstSearch.so

$(OBJ)/budFirstSearch.o: $(SRC)/budFirstSearch.cpp
	$(CCC) -std=c++11 -fPIC -c $(SRC)/budFirstSearch.cpp -I$(INC) -I$(MAININC) -I$(BOOSTDIR) -I../tools -o $(OBJ)/budFirstSearch.o

$(OBJ)/budFirstSearch_wrap.o: $(SWIG)/budFirstSearch_wrap.cxx
	$(CCC) -std=c++11 -fPIC -c $(SWIG)/budFirstSearch_wrap.cxx -I$(PYINC) -I$(INC) -o $(OBJ)/budFirstSearch_wrap.o

$(SWIG)/budFirstSearch_wrap.cxx: $(SWIG)/budFirstSearch.i
	swig -c++ -python -py3 -outdir wrapper $(SWIG)/budFirstSearch.i

clean:
	rm $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(SWIG)/*_wrap.cxx wrapper/*.so wrapper/budFirstSearch.py
