include ../boost_home

WRAPRDIR ?= .
MAINDIR ?= ..

CCC = g++

SRC=$(WRAPRDIR)/wrapper/src
OBJ=$(WRAPRDIR)/wrapper/obj
INC=$(WRAPRDIR)/wrapper/include
SWIG=$(WRAPRDIR)/wrapper/swig
MAININC=$(MAINDIR)/src/include

PYFLAGS = `python3-config --cflags`
CFLAGS = -std=c++11 -fPIC -I$(INC) -I$(MAININC) -I$(BOOSTDIR) -I../tools $(PYFLAGS) # -ffloat-store
LFLAGS = -L$(OBJ) -flto

PWRAPSRC = $(wildcard $(SRC)/*.cpp)
PWRAPSWIG = $(wildcard $(SWIG)/*.i)
PWRAPAUX = $(PWRAPSRC:.cpp=.o)
PWRAPSWIGAUX = $(PWRAPSWIG:.i=_wrap.o)
PWRAPOBJ = $(patsubst $(SRC)/%, $(OBJ)/%, $(PWRAPAUX))
PWRAPSWIGOBJ = $(patsubst $(SWIG)/%, $(OBJ)/%, $(PWRAPSWIGAUX))
PLIBOBJ=$(wildcard $(MAINDIR)/src/obj/*.o)

wrapper/_budFirstSearch.so: $(PWRAPOBJ) $(PWRAPSWIGOBJ)
	$(CCC) -std=c++11 -fPIC -shared $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(PLIBOBJ) -o wrapper/_budFirstSearch.so

$(OBJ)/budFirstSearch.o: $(SRC)/budFirstSearch.cpp
	$(CCC) $(CFLAGS) -c $(SRC)/budFirstSearch.cpp -o $@

$(OBJ)/budFirstSearch_wrap.o: $(SWIG)/budFirstSearch_wrap.cxx
	$(CCC) $(CFLAGS) -c $(SWIG)/budFirstSearch_wrap.cxx -o $@

$(SWIG)/budFirstSearch_wrap.cxx: $(SWIG)/budFirstSearch.i
	swig -c++ -python -py3 -outdir wrapper $(SWIG)/budFirstSearch.i

clean:
	rm -rf $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(SWIG)/*_wrap.cxx wrapper/*.so wrapper/budFirstSearch.py
