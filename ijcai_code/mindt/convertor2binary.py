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
import feature_selection


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

def convertor2iti_collect_stats(data = None, output_file = None):
    out_file = output_file + "." + LABEL_SUFFIX_DATA
    out_file_names = output_file + "." + LABEL_SUFFIX_NAMES
    out_file_res = output_file + "." + LABEL_SUFFIX_RESULTS
    out_file_reduced_csv = output_file.replace(".csv", "") + "." + LABEL_SUFFIX_REDUCED_CSV 
    out_file_reduced_csv_data = out_file_reduced_csv + "." + LABEL_SUFFIX_DATA
    out_file_reduced_csv_names =out_file_reduced_csv + "." + LABEL_SUFFIX_NAMES
    print(out_file_res)
    
    with open(out_file_names) as f:
        lines_names = f.readlines()
        f.close()
    
    names =  []
    map_name_value = {}
    map_data_value = {}

    pref_dummy = LABEL_PREFIX_DUMMY
    r  = randint(100)
    pref_dummy  = pref_dummy # + str(int(r))
    for name in lines_names[1:]:
        nm = name.strip().split(":")[0]
        if (len(nm) != 0):
            names.append(nm) 
            map_name_value[nm] = [pref_dummy + nm]
            map_data_value[nm] = []
    
    print(names)     
    with open(out_file_res) as f:
        lines = f.readlines() 
        f.close()
    for line in lines:
        print(line) 
        for feat in names:
            if feat in line:
                sp = line.split("=")
                f = feat
                a = sp[1].strip()
                map_name_value[f].append(a)
    print(map_name_value)
        
    with open(out_file) as f:
        lines_data = f.readlines()
        f.close()
        
    new_samples = []
    for data in lines_data:
        #print(data)
        sample = data.split(",")
        new_sample = []
        for i, ft in enumerate(sample):
            ft = ft.strip().replace(".", '')
            if (i == len(sample) - 1):
                res = ft#ft.replace(LABEL_PREFIX_RES,'')
                new_sample.append(res)
                continue
            #print(i,ft)
            nm = names[i]
            if ft in map_name_value[nm]:
                #print(ft)
                #ft = ft.replace(LABEL_PREFIX_FEATS,'')
                new_sample.append(ft)
            else:
                ft= pref_dummy + nm
                new_sample.append(ft)
            if (ft not in map_data_value[nm]):
                map_data_value[nm].append(ft)
        print(new_sample)
        new_samples.append(new_sample)        
    print(map_data_value)
    
    removed_features = []
    for i, data_value in enumerate(map_data_value):
        if (len(map_data_value[data_value]) == 1):
           removed_features.append(i) 
    
    
    

    print(removed_features)    
    with open(out_file_reduced_csv_names, "w+") as f_names:        
        # result first 
        s_nm = lines_names[0].strip().replace("\n","")
        print(s_nm, file = f_names)
        for i, name in enumerate(names):
            values = map_data_value[name]
            s_nm = "{}:".format(name)
            if i not in removed_features:
                for v in values:
                    s_nm = s_nm + "{},".format(v)
                s_nm = s_nm[:-1]
                s_nm += "."
                print(s_nm, file = f_names)
        f_names.close()

    with open(out_file_reduced_csv_data, "w+") as f:
        with open(out_file_reduced_csv, "w+") as f_csv:
            for i, name in enumerate(names):
                if i not in removed_features:
                    print("{},".format(name), end = "", file = f_csv)
                
            print(LABEL_PREFIX_RES, file = f_csv)
            for new_sample in new_samples:
                s = ''.join('{},'.format(str(k).strip()) for i, k in enumerate(new_sample) if i not in removed_features)   
                s = s[:-1]
                print(s, file = f)
                print(s, file = f_csv)

            f.close()
            f_csv.close()



def convertor2iti(data = None, output_file = None, file_to_run = None):  

    print("Data is given")
    out_file = output_file + "." + LABEL_SUFFIX_DATA
    out_file_names = output_file + "." + LABEL_SUFFIX_NAMES
    out_file_res = output_file + "." + LABEL_SUFFIX_RESULTS
    
    print(out_file)
    print("Output " + out_file)
    bin_encoding =open(out_file, 'w+')
    bin_encoding_names =open(out_file_names, 'w+')

    print(data.samps)    
    values_list = [[] for x in range(len(data.names))]
    print(values_list)
    for sample in data.samps:
        for i,s in enumerate(sample):
            if (s not in values_list[i]):
                values_list[i].append(s)
                values_list[i] = sorted(values_list[i])

    print(values_list)


    
    str_names = ""
    #-,+.
    #outlook:sunny,overcast,rain.
    #temperature:hot,mild,cool.
    #humidity:high,normal.
    #windy:true,false.    
    
    str_names = ''.join('{}{},'.format(LABEL_PREFIX_RES, str(k).strip()) for i, k in enumerate(values_list[-1]))
    str_names = str_names[:-1] + ".\n"
    for i, name in enumerate(values_list):
        if (i == len(values_list) - 1):
            continue
        str_names = str_names + data.names[i] + ":"
        #print(''.join('a{},'.format(str(k[1]).strip()) for k in enumerate(values_list[i])))
        str_names = str_names + ''.join('{}{},'.format(LABEL_PREFIX_FEATS, str(k).strip()) for i, k in enumerate(values_list[i]))       
        str_names = str_names[:-1] + ".\n"
    print(str_names, file = bin_encoding_names) 
    
    for i, sample in enumerate(data.samps):
        print(i)
        #print(sample)
        str1 = ""           
        for i,s in enumerate(sample):
            pref = LABEL_PREFIX_FEATS
            if (i == len(sample) - 1):
                 pref = LABEL_PREFIX_RES
                
            v = pref + str(s).strip()
            str1 = str1 + v +  ","
        str1 =str1[:-1]
        str1  = str1 + "."
        #print(str1)
        print(str1, file = bin_encoding) 
        #exit()
    

    bin_encoding.close()

    head, tail = os.path.split(out_file)
    tail = tail.replace("." + LABEL_SUFFIX_DATA ,"")
    s =  "/home/nina/workspace/xai/src/ITI/bin/iti " + head + " -l" + tail  + " -f -u  -w  > " + out_file_res
    print(s,  file = file_to_run)
   

def convertor2reduced(data = None, bin_file = None, acc = None):    
    f = bin_file.replace("Samples/", "")
    os.system("cp " + bin_file + " " + f)
    #print("cp " + bin_file + " " + f)

    if (list(acc)[0] < 0):
        st = "cp " + bin_file  + " " +  f.replace(".csv",LABEL_SUFFIX_REDUCED_CSV)
    else:
        st = "python src/mindt/feature_selection.py  --accmin=" + str(list(acc)[0]) + " " +  f
    print(st)
    os.system(st)
    #exit()
    return  f.replace(".csv",LABEL_SUFFIX_REDUCED_CSV)
   
def convertor2binary(data = None, output_file = None):    
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
        out_file = options.files[0].replace(".csv","-un.csv") #options.files[0] + ".un.csv"
    
    else:
        print("Data is given")
        output_file = output_file.replace(".csv","-un.csv")
        out_file = output_file
    
    #print(" python src/scripts/d2bin.py " + output_file + " >  " + out_file)
    #exit()
    #return
    print(out_file)
    print("Output " + out_file)
    bin_encoding =open(out_file, 'w+')

    print(data.samps)    
    values_list = [[] for x in range(len(data.names))]
    print(values_list)
    for sample in data.samps:
        for i,s in enumerate(sample):
            if (s not in values_list[i]):
                values_list[i].append(s)
                values_list[i] = sorted(values_list[i])

    print(values_list)
    str1 = ""
    for i, name in enumerate(data.names):
        l = len(values_list[i])
        if l == 2: 
            str1 = str1 + name  + ","
        else:
            for j in range(l):
                str1 = str1 + name + ":" +str(j) + ","
    str1 =str1[:-1]
    print(str1)
    print(str1, file = bin_encoding) 
    
    for i, sample in enumerate(data.samps):
        print(i)
        #print(sample)
        str1 = ""           
        for i,s in enumerate(sample):
            idx = values_list[i].index(s)
            l = len(values_list[i])
            
            if l == 2:
                str1 = str1 + str(idx) + ","            
            else:     
                for j in range(l):
                    if(j == idx):
                        str1 = str1 + "1,"
                    else:
                        str1 = str1 + "0,"
        str1 =str1[:-1]
        #print(str1)
        print(str1, file = bin_encoding) 

    bin_encoding.close()
    return out_file
    
if __name__ == '__main__':
    convertor2binary()
 