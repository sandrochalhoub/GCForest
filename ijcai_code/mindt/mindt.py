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
import inspect, os, sys
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../pysat-module/'))
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../hitman/py/'))

from dtencoder import DTEncoder

from data import Data, PrepData
from options import Options
import resource

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

    # parsing data
    if options.files:
        data = Data(filename=options.files[0], mapfile=options.mapfile,
                separator=options.separator)
    else:
        data = Data(fpointer=sys.stdin, mapfile=options.mapfile,
                separator=options.separator)
    
    if not(options.prepfile is None):
        print(options.prepfile)
        prepData = PrepData(origdata = data, filename=options.prepfile,
                separator=options.separator)

    
    if options.verb:
        print('c0 # of samps: {0} ({1} weighted)'.format(sum(data.wghts), len(data.samps)))
        print('c0 # of feats: {0} ({1} binary)'.format(len(data.names) - 1, len(filter(lambda x: x > 0, data.fvmap.opp.keys())) - len(data.feats[-1])))
        print('c0 # of labls: {0}'.format(len(data.feats[-1])))

        used_time = resource.getrusage(resource.RUSAGE_SELF).ru_utime
        print('c0 parse time: {0:.4f}'.format(used_time))
        print('')

    print(options.approach)
    if options.approach == 'dtencoding':

        res = True
        N_to_try = options.nbnodes
        total_sat_time = 0
        
        is_solution = 0
        if (options.iti_result_file is not None):
            print(options.iti_result_file)
            with open(options.iti_result_file) as f:
                lines = f.readlines() 
                f.close()
            
            for line in lines:
                if "Nodes" in line:
                    s = line.split()
                    N_to_try = int(s[1])
                    break
                 
        
        print(N_to_try)
        
        while (res):
            if not(options.prepfile is None):
                dtencoder = DTEncoder(prepData, options)
            else:
                dtencoder = DTEncoder(data, options)
            res, sat_time = dtencoder.generate_formula(N = N_to_try)
            total_sat_time += sat_time
            if (res == False):
                break
            is_solution = 1
            print("Tree of size {}, time  {}, total {}".format(N_to_try, sat_time, total_sat_time))
            N_to_try = N_to_try - 2
            #exit()

        if (is_solution > 0):
            print("Optimal tree  {}, total time {}".format( N_to_try +  2, total_sat_time))
        else:    
            print("No tree  with <=  {} nodes".format( N_to_try, total_sat_time))
        
    else:  # mp92 (karmarkar)
        pass

    if options.verb:
        total_time = resource.getrusage(resource.RUSAGE_CHILDREN).ru_utime + resource.getrusage(resource.RUSAGE_SELF).ru_utime
        print('c3 total time: {0:.4f}'.format(total_time))
