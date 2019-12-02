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

# for importing the correct version of pysat and hitman
#==============================================================================
import inspect, os, sys, random, copy
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../pysat-module/'))
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../hitman/py/'))


import subprocess

from  utils import *
from data import Data
from options import Options
import resource
import convertor2binary
random.seed(2019)

#iti_all =open("execute_iti", 'w+')

#
#==============================================================================
def show_info():
    """
        Print info message.
    """

    print('c MinDT: miner of decision trees')
    print('c author(s): Alexey Ignatiev    [email:aignatiev@ciencias.ulisboa.pt]')
    print('c            Joao Marques-Silva [email:jpms@ciencias.ulisboa.pt]')
    print('c            Nina Narodytska [email:n.narodytska@gmail.com]')

    print('')


#
#==============================================================================
if __name__ == '__main__':
    # parsing command-line options
    options = Options(sys.argv)

    print(sys.stdout.fileno())
    # making output unbuffered
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w')

    # showing head
    show_info()
    
    #read file
    
    input_file_names = options.files[0]
    with open(input_file_names) as f:
        lines_names = f.readlines()
        f.close()
  
    new_lines = []
    nb_feat = 0
    for name in lines_names:
        nm = name.split()
        nb_feat = (len(nm))
        if (int(nm[-1]) == 1):
            nm[-1] = 0
        if (int(nm[-1]) > 1):
            nm[-1] = 1
            
        #print(name) 
        new_line = ''.join("{},".format(x) for x in nm)
        new_line = new_line[:-1]        
        print(new_line)
        new_lines.append(new_line[:])

    output_file_names = input_file_names + ".csv"
    with open(output_file_names, "w+") as f:        
        for i in range(nb_feat):
            pref = "f" if i < nb_feat - 1 else "res"
            print("{}{}".format(pref, i), end = "," if (i < nb_feat - 1) else "\n", file = f)
        for n in new_lines:
            print("{}".format(n), file = f)
            

