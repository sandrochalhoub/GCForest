#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## minds.py
##
##  Created on: Dec 3, 2017
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

# print function as in Python3
#==============================================================================
from __future__ import print_function
import subprocess

# for importing the correct version of pysat and hitman
#==============================================================================
import inspect, os, sys
from numpy.matlib import rand
from numpy.random.mtrand import randint
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../pysat-module/'))
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../hitman/py/'))

from  utils import *
from pathlib import Path
from data import Data
from options import Options
import resource

def run(path_in_str):
    #print(path_in_str)
    if ("-un" not in path_in_str and "schizo" not in path_in_str):
        print(path_in_str)
        os.system("python src/mindt/convertor2binary.py " +  path_in_str)
        #exit()

def convert_all2binary():
 
    path_manu_bench = "/home/nina/workspace/xai/benchs/Datasets/Emmanuel/"
    path_penn_bench = "/home/nina/workspace/xai/benchs/Datasets/PennML/"   
    pathlist1 = Path(path_manu_bench).glob('**/*.csv')
    pathlist2 = Path(path_penn_bench).glob('**/*.csv')
    
    for path in pathlist1:
            # because path is object not string
        path_in_str = str(path)
        #print(path_in_str)
        run(path_in_str)
        
    for path in pathlist2:
            # because path is object not string
        #print(path_in_str)    
        path_in_str = str(path)
        run(path_in_str)
    
    
if __name__ == '__main__':
    convert_all2binary()
 