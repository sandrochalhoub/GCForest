include ./boost_home

COPTIMIZE= -g -fno-omit-frame-pointer -flto  
COMPILFLAGS= -D_UNIX -D_BIT64 -std=c++1z -Wno-sign-compare 

include ./template.mk

DATE := $(shell date '+%y-%m-%d')

clean : 
	rm -rf $(OBJ)/*.o $(OBJ)/*.a $(SRC)/*~ $(MOD)/obj/*.o $(MOD)/src/*~ $(MOD)/src/*/*~ $(INC)/*~ $(UTI)/*~  *~ $(BIN)/* $(DOC)/*~ ./fzn-mistral fz/mistral-fzn

