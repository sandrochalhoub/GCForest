
wrapper/swig/_budFirstSearch.so: wrapper/obj/budFirstSearch.o wrapper/obj/budFirstSearch_wrap.o
	c++ -std=c++11 -shared wrapper/obj/budFirstSearch.o wrapper/obj/budFirstSearch_wrap.o -o wrapper/swig/_budFirstSearch.so

wrapper/obj/budFirstSearch.o: wrapper/src/budFirstSearch.cpp
	c++ -std=c++11 -fPIC -c wrapper/src/budFirstSearch.cpp -Iwrapper/include -o wrapper/obj/budFirstSearch.o

wrapper/obj/budFirstSearch_wrap.o: wrapper/swig/budFirstSearch_wrap.cxx
	c++ -std=c++11 -fPIC -c wrapper/swig/budFirstSearch_wrap.cxx -I/usr/include/python3.6 -Iwrapper/include -o wrapper/obj/budFirstSearch_wrap.o

wrapper/swig/budFirstSearch_wrap.cxx: wrapper/swig/budFirstSearch.i
	swig -c++ -python -py3 wrapper/swig/budFirstSearch.i

clean:
	rm wrapper/obj/*.o wrapper/swig/*_wrap.cxx wrapper/swig/*.so wrapper/swig/budFirstSearch.py