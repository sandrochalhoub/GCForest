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
CFLAGS = -std=c++11 -fPIC -I$(INC) -I$(MAININC) -I$(BOOSTDIR) -I../tools $(PYFLAGS) -arch x86_64 -Wno-shorten-64-to-32 # -ffloat-store
LFLAGS = -L$(OBJ) -flto -arch x86_64 -Wno-shorten-64-to-32

PWRAPSRC = $(wildcard $(SRC)/*.cpp)
PWRAPSWIG = $(wildcard $(SWIG)/*.i)
PWRAPAUX = $(PWRAPSRC:.cpp=.o)
PWRAPSWIGAUX = $(PWRAPSWIG:.i=_wrap.o)
PWRAPOBJ = $(patsubst $(SRC)/%, $(OBJ)/%, $(PWRAPAUX))
PWRAPSWIGOBJ = $(patsubst $(SWIG)/%, $(OBJ)/%, $(PWRAPSWIGAUX))
PLIBOBJ=$(wildcard $(MAINDIR)/src/obj/*.o)

wrapper/_blossom.so: $(PWRAPOBJ) $(PWRAPSWIGOBJ)
	$(CCC) -std=c++11 -fPIC -shared $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(PLIBOBJ) -arch x86_64 -Wno-shorten-64-to-32 -o wrapper/_blossom.so

$(OBJ)/blossom.o: $(SRC)/blossom.cpp
	$(CCC) $(CFLAGS) -c $(SRC)/blossom.cpp -o $@

$(OBJ)/blossom_wrap.o: $(SWIG)/blossom_wrap.cxx
	$(CCC) $(CFLAGS) -c $(SWIG)/blossom_wrap.cxx -o $@

$(SWIG)/blossom_wrap.cxx: $(SWIG)/blossom.i
	swig -c++ -python -py3 -outdir wrapper $(SWIG)/blossom.i

clean:
	rm -rf $(PWRAPOBJ) $(PWRAPSWIGOBJ) $(SWIG)/*_wrap.cxx wrapper/*.so wrapper/blossom.py
