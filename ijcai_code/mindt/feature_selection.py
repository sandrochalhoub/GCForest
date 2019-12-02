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

from data import Data
from options import Options
import resource
from collections import Counter

import numpy as np

from sklearn.datasets import fetch_20newsgroups
from sklearn.feature_selection import mutual_info_classif
from sklearn.feature_selection import VarianceThreshold
from sklearn.feature_selection import SelectFromModel
from sklearn.datasets import load_iris
from sklearn.svm import LinearSVC
from sklearn.feature_selection import SelectKBest
from sklearn.feature_selection import chi2, mutual_info_classif
from sklearn.ensemble import ExtraTreesClassifier
from sklearn import tree
#
#==============================================================================
def show_info():
    """
        Print info message.
    """

    print('c featire selection ')
    print('c author(s): Alexey Ignatiev    [email:aignatiev@ciencias.ulisboa.pt]')
    print('c            Joao Marques-Silva [email:jpms@ciencias.ulisboa.pt]')
    print('c            Nina Narodytska [email:n.narodytska@gmail.com]')

    print('')


#
#==============================================================================
   
def feature_exactor(data = None, output_file = None):    
    # parsing command-line options
    options = Options(sys.argv)


    # making output unbuffered
    #sys.stdout = os.fdopen(sys.stdout.fileno(), 'w')

    # showing head
    #show_info()

    if(data is None):
         # parsing data
        if options.files:
            data = Data(filename=options.files[0], mapfile=options.mapfile,
                    separator=options.separator)
        else:
            data = Data(fpointer=sys.stdin, mapfile=options.mapfile,
                    separator=options.separator)
    
        if options.verb:
            print('c0 # of samps: {0} ({1} weighted)'.format(sum(data.wghts), len(data.samps)))
            print('c0 # of feats: {0} ({1} binary)'.format(len(data.names) - 1, len(filter(lambda x: x > 0, data.fvmap.opp.keys())) - len(data.feats[-1])))
            print('c0 # of labls: {0}'.format(len(data.feats[-1])))
    
            used_time = resource.getrusage(resource.RUSAGE_SELF).ru_utime
            print('c0 parse time: {0:.4f}'.format(used_time))
            print('')


        #output_file = options.files[0].replace(".csv","-un.csv")
        out_file = options.files[0].replace(".csv",LABEL_SUFFIX_REDUCED_CSV) #options.files[0] + ".un.csv"
    
    else:
        print("Data is given")
        output_file = output_file.replace(".csv",LABEL_SUFFIX_REDUCED_CSV)
        out_file = output_file

    for k in range(1,100):
        x = []
        t = []
        if ( k== len(data.samps[0]) - 1):
            print("Failed to generates")
            return
        for sample in data.samps:
            x.append(sample[:-1])
            t.append(sample[-1])
        
        x = np.array(x)
        t = np.array(t) 
        #print(x,t)
        
        iris = load_iris()
        #X, y = iris.data, iris.target
        #print(type(X), type(y))
        #print(X,y)        
        results  = SelectKBest(mutual_info_classif, k = k)
        x_new = results.fit_transform(x, t)
        #print(x_new)
        #print(results.scores_)
        best_index = results.scores_.argsort()[-k:][::-1]
        #print(best_index)
        
#         results = ExtraTreesClassifier(n_estimators = 1)
#         results.fit(x, t)
# 
#         print(results.feature_importances_)
#         best_index = indices = np.argsort(results.feature_importances_)[::-1][-k:]
#         print(best_index)        
# 
#         x_new = []
#         for r in x:
#             new_r = []
#             for i,c in enumerate(r):
#                 if i in best_index:
#                     new_r.append(c)
#             x_new.append(new_r)


        print(len(x_new))
        clf = tree.DecisionTreeClassifier()
        clf = clf.fit(x_new, t)
        y_predict = clf.predict(x_new)
        from sklearn.metrics import accuracy_score
        acc = accuracy_score(t, y_predict)
        print(acc)
        if (acc >= options.accmin):
            break


    if (acc >= options.accmin):        
        examples = {}
        for i, sample in enumerate(data.samps):
            print(i)
            #print(sample)
            str1 = ""           
            for i,s in enumerate(sample):
                if ((i == len(data.names)-1)):
                    if (str1 in examples):
                        examples[str1].append(s)
                    else:  
                        examples[str1] = [s]
                if (i in best_index or (i == len(data.names)-1)):
                    str1 = str1 + str(s) + ","            
            str1 =str1[:-1]
        
        print(out_file)
        print("Output " + out_file)
        bin_encoding =open(out_file, 'w+')
        
        str1 = ""
        for i, name in enumerate(data.names):
            if (i in best_index or (i == len(data.names)-1)):
                str1 = str1 + name  + ","
        str1 =str1[:-1]
        #print(str1)
        print(str1, file = bin_encoding) 
        
        
        for i, sample in enumerate(data.samps):
            print(i)
            #print(sample)
            str1 = ""           
            for i,s in enumerate(sample):
                if ((i == len(data.names)-1)):
                    c = Counter(examples[str1])
                    value, count = c.most_common()[0]
                    str1 = str1 + str(value)
                if (i in best_index):
                    str1 = str1 + str(s) + ","            
            #str1 =str1[:-1]
            #print(str1)
            print(str1, file = bin_encoding) 
    
        bin_encoding.close()
    print(examples)
    
if __name__ == '__main__':
    feature_exactor()
 