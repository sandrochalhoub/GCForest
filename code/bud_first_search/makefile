include ../boost_home

PYTHON_DIR ?= .
MAINDIR ?= ..

SRC=$(PYTHON_DIR)/wrapper/src
OBJ=$(PYTHON_DIR)/wrapper/obj
INC=$(PYTHON_DIR)/wrapper/include
SWIG=$(PYTHON_DIR)/wrapper/swig
MAININC=$(MAINDIR)/src/include

PWRAPSRC = $(wildcard $(SRC)/*.cpp)
PWRAPSWIG = $(wildcard $(SWIG)/*.i)
PWRAPAUX = $(PWRAPSRC:.cpp=.o)
PWRAPSWIGAUX = $(PWRAPSWIG:.i=_wrap.o)
PWRAPOBJ = $(patsubst $(SRC)/%, $(OBJ)/%, $(PWRAPAUX))
PWRAPSWIGOBJ = $(patsubst $(SWIG)/%, $(OBJ)/%, $(PWRAPSWIGAUX))
PLIBOBJ=$(wildcard $(MAINDIR)/src/obj/*.o)

$(SWIG)/_budFirstSearch.so: $(PWRAPOBJ) $(PWRAPSWIGOBJ)
	c++ -std=c++11 -shared $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(PLIBOBJ) -o $(SWIG)/_budFirstSearch.so

$(OBJ)/budFirstSearch.o: $(SRC)/budFirstSearch.cpp
	c++ -std=c++11 -fPIC -c $(SRC)/budFirstSearch.cpp -I$(INC) -I$(MAININC) -I$(BOOSTDIR) -I../tools -o $(OBJ)/budFirstSearch.o

$(OBJ)/budFirstSearch_wrap.o: $(SWIG)/budFirstSearch_wrap.cxx
	c++ -std=c++11 -fPIC -c $(SWIG)/budFirstSearch_wrap.cxx -I/usr/include/python3.6 -I$(INC) -o $(OBJ)/budFirstSearch_wrap.o

$(SWIG)/budFirstSearch_wrap.cxx: $(SWIG)/budFirstSearch.i
	swig -c++ -python -py3 $(SWIG)/budFirstSearch.i

clean:
	rm $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(SWIG)/*_wrap.cxx $(SWIG)/*.so $(SWIG)/budFirstSearch.py