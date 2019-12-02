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
from click.termui import pause
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../pysat-module/'))
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../hitman/py/'))


import subprocess

import numpy as np
from  utils import *
from data import Data
from options import Options
import resource
import convertor2binary
random.seed(2019)

iti_all =open("execute_iti", 'w+')

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

def parse_results2():
    bench_res = {}
    for bench in original_benckmarks:
        values = original_benckmarks[bench]
        head, tail = os.path.split(bench)
        bench = head
        bench = bench  + "/"+ LABEL_SUFFIX_DIR_RESULTS + "/"
        print(bench)
     
        if (True):
            tree_sizes_all = []      
            tree_sizes = []
            nb_smp= 0   
            nb_orig_node = 0
            bench_v = bench  + "/"               
            bench_i = bench_v + tail + LABEL_SUFFIX_SAVE_RESULTS
            bench_i_orig  = bench_i.replace("Results", "Samples").replace("-reduced.csv_save", ".csv.result")
            print(bench_i_orig)
            print(bench_i)
            #exit()

            try:
                with open(bench_i) as f:
                    lines = f.readlines() 
                    f.close()
                for line in lines:
                    #print(line) 
                    if PRINT_TREE_SIZE_RESULTS in line:
                        spl = line.split()
                        #print(spl)
                        tree_sizes.append([int(spl[3].replace(",","")), float(spl[5].replace(",","")), -1])
                    if PRINT_OPT_TREE_SIZE_RESULTS in line:
                        spl = line.split()
                        #print(spl)
                        tree_sizes.append([int(spl[2].replace(",","")),float(spl[5].replace(",","")), 1])
                    if PRINT_COUNTER_RESULTS in line:
                        spl = line.split(":")
                        #print(spl)
                        if (nb_smp == 0):
                            nb_smp = len(spl)-1
            except IOError as e:
                print("Couldn't open or write to file (%s)." % e)

            try:
                with open(bench_i_orig) as f:
                    lines = f.readlines() 
                    f.close()
                for line in lines:
                    #print(line) 
                    if "Nodes" in line:
                        spl = line.split()
                        #print(spl)
                        nb_orig_node = int(spl[1])
                        print(nb_orig_node)
                            
            except IOError as e:
                print("Couldn't open or write to file (%s)." % e)

            
            #print(len(tree_sizes))
            if (len(tree_sizes) > 0):
                #print(tree_sizes_all)    
                tree_sizes_all.append([tree_sizes[0] + tree_sizes[-1]])

            
            tree_sizes_all = np.array(tree_sizes_all)
            #print(tree_sizes_all)
            if (len(tree_sizes_all) > 0):    

                mn = np.mean(tree_sizes_all, axis=0)
                sm  = np.sum(tree_sizes_all, axis=0)
                    
                bench_res[bench_v] = mn[0][[0,3,4]].tolist()
                #bench_res[bench_v].append(-sm[0[2])
                #bench_res[bench_v].append(sm[0][5])
                #bench_res[bench_v].append(nb_smp)
                bench_res[bench_v].append(nb_orig_node)
                print(nb_smp)
               
    print(bench_res)
    print("\\hline")
    print("Name & \#nodes (ITI,unreduced) &  \#nodes(ITI) & \#nodes & avg. t  \\\\")
    print("\\hline")
    print("\\hline")

    for bench in bench_res:
        res = bench_res[bench]
        #print(bench, res )
        spl = bench.replace("//","/").split("/")
        #print(spl)
        print(spl[9] + "& " + str(res[-1]) + " & ", end  = "")
        for i,  r in enumerate(res[:-1]):
            if (i == len(res) -2):
                print("{:0.2f}".format(r) + " ", end  = " \\\\ \n")
            else:
                print("{:0.2f}".format(r) + " & ", end  = "")
    print("\\hline")


def parse_results(nb_samples):
    bench_res = {}
    for bench in original_benckmarks:
        values = original_benckmarks[bench]
        head, tail = os.path.split(bench)
        bench = head
        bench = bench  + "/"+ LABEL_SUFFIX_DIR_RESULTS + "/"

        for v in values:
            bench_v = bench  + "/"+ str(v) + "/"          
            tree_sizes_all = []                                  
            nb_smp= 0   
            for i in range(nb_samples):
                tree_sizes = []   
                bench_i = bench_v + tail.replace(".csv", "_" + str(i) + ".csv") + LABEL_SUFFIX_SAVE_RESULTS
                print(bench_i)                    
                try:
                    with open(bench_i) as f:
                        lines = f.readlines() 
                        f.close()
                    for line in lines:
                        #print(line) 
                        if PRINT_TREE_SIZE_RESULTS in line:
                            spl = line.split()
                            #print(spl)
                            tree_sizes.append([int(spl[3].replace(",","")), float(spl[5].replace(",","")), -1])
                        if PRINT_OPT_TREE_SIZE_RESULTS in line:
                            spl = line.split()
                            #print(spl)
                            tree_sizes.append([int(spl[2].replace(",","")),float(spl[5].replace(",","")), 1])
                        if PRINT_COUNTER_RESULTS in line:
                            spl = line.split(":")
                            if (nb_smp == 0):
                                nb_smp = len(spl)-1
                except IOError as e:
                    print("Couldn't open or write to file (%s)." % e)
                #print(len(tree_sizes))
                if (len(tree_sizes) > 0):
                    #print(tree_sizes_all)    
                    tree_sizes_all.append([tree_sizes[0] + tree_sizes[-1]])
                    #print(tree_sizes)
            tree_sizes_all = np.array(tree_sizes_all)
           # print(tree_sizes_all)
            if (len(tree_sizes_all) > 0):    

                mn = np.mean(tree_sizes_all, axis=0)
                sm  = np.sum(tree_sizes_all, axis=0)
                    
                bench_res[bench_v] = mn[0][[0,3,4]].tolist()
                bench_res[bench_v].append(-sm[0][2])
                bench_res[bench_v].append(sm[0][5])
                bench_res[bench_v].append(nb_smp)
                print(nb_smp)
           


        if (len(values) == 0):
            tree_sizes_all = []      
            tree_sizes = []
            nb_smp= 0   
            bench_v = bench  + "/"               
            bench_i = bench_v + tail + LABEL_SUFFIX_SAVE_RESULTS
            print(bench_i)
            try:
                with open(bench_i) as f:
                    lines = f.readlines() 
                    f.close()
                for line in lines:
                    #print(line) 
                    if PRINT_TREE_SIZE_RESULTS in line:
                        spl = line.split()
                        #print(spl)
                        tree_sizes.append([int(spl[3].replace(",","")), float(spl[5].replace(",","")), -1])
                    if PRINT_OPT_TREE_SIZE_RESULTS in line:
                        spl = line.split()
                        #print(spl)
                        tree_sizes.append([int(spl[2].replace(",","")),float(spl[5].replace(",","")), 1])
                    if PRINT_COUNTER_RESULTS in line:
                        spl = line.split(":")
                        #print(spl)
                        if (nb_smp == 0):
                            nb_smp = len(spl)-1
            except IOError as e:
                print("Couldn't open or write to file (%s)." % e)
            #print(len(tree_sizes))
            if (len(tree_sizes) > 0):
                #print(tree_sizes_all)    
                tree_sizes_all.append([tree_sizes[0] + tree_sizes[-1]])

            
            tree_sizes_all = np.array(tree_sizes_all)
            #print(tree_sizes_all)
            if (len(tree_sizes_all) > 0):    

                mn = np.mean(tree_sizes_all, axis=0)
                sm  = np.sum(tree_sizes_all, axis=0)
                    
                bench_res[bench_v] = mn[0][[0,3,4]].tolist()
                bench_res[bench_v].append(-sm[0][2])
                bench_res[bench_v].append(sm[0][5])
                bench_res[bench_v].append(nb_smp)
                print(nb_smp)
               
    print(bench_res)
    print("\\hline")
    print("Name & ratio & \#nodes(ITI) & \#nodes & avg. t& \#solves & \#opt & \#samples \\\\")
    print("\\hline")
    print("\\hline")

    for bench in bench_res:
        res = bench_res[bench]
        #print(bench, res )
        spl = bench.replace("//","/").split("/")
        #print(spl)
        if (spl[11] == ''):
            print(spl[9] + "& " + str(1) + " & ", end  = "")
        else:    
            print(spl[9] + "& " +spl[11] + " & ", end  = "")
        for i,  r in enumerate(res):
            if (i == len(res) -1):
                print("{:0.2f}".format(r) + " ", end  = " \\\\ \n")
            else:
                print("{:0.2f}".format(r) + " & ", end  = "")
    print("\\hline")
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


    if options.verb:
        print('c0 # of samps: {0} ({1} weighted)'.format(sum(data.wghts), len(data.samps)))
        print('c0 # of feats: {0} ({1} binary)'.format(len(data.names) - 1, len(filter(lambda x: x > 0, data.fvmap.opp.keys())) - len(data.feats[-1])))
        print('c0 # of labls: {0}'.format(len(data.feats[-1])))

        used_time = resource.getrusage(resource.RUSAGE_SELF).ru_utime
        print('c0 parse time: {0:.4f}'.format(used_time))
        print('')


    nb_samples_bench = 20
    gen_benchs = True
    gen_runs = True
    gen_feat_reduce = False
    gen_parse_results = False


    if (options.scilearn > 0):
        gen_reduced_scilearn = True
          
    if (options.scilearn == 0):
        gen_reduced_scilearn = False


    
    if (gen_parse_results == False):
        path_manu_bench = "./benchs/Datasets/Emmanuel/"
        path_penn_bench = "./benchs/Datasets/PennML/"
        path_others_bench = "./benchs/Datasets/Others/"

    else:
        path_manu_bench = "/home/nina/prom_titan_1/workspace/xai/benchs/Datasets/Emmanuel/"
        path_penn_bench = "/home/nina/prom_titan_1/workspace/xai/benchs/Datasets/PennML/"
        path_others_bench = "/home/nina/prom_titan_1/workspace/xai/benchs/Datasets/Others/"

        #path_manu_bench = "./benchs/Datasets/Emmanuel/"
        #path_penn_bench = "./benchs/Datasets/PennML/"
        
    original_benckmarks = {}
    acc = {}

    
    if (gen_reduced_scilearn == True):
        print("scip this now")
         
        #original_benckmarks[path_manu_bench + "Mouse/mouse.csv"] = {}
        #original_benckmarks[path_manu_bench + "Weather/meteo.csv"] = {}
#         original_benckmarks[path_manu_bench + "Car/car.csv"] = {0.05, 0.1}#, 0.2, 0.3,0.5,0.7,0.9}
#         original_benckmarks[path_manu_bench + "Cancer/cancer.csv"] = {0.1, 0.2}#, 0.5, 0.7,0.9} 
#         original_benckmarks[path_penn_bench + "ShuttleM/shuttleM.csv"] = {0.05, 0.1}
#         original_benckmarks[path_penn_bench + "Colic/colic.csv"] = {0.05, 0.1}
#         original_benckmarks[path_penn_bench + "appendicitis/appendicitis.csv"] = {0.3, 0.4}
#         original_benckmarks[path_penn_bench + "australian/australian.csv"] = {0.05, 0.1}
#         original_benckmarks[path_penn_bench + "backache/backache.csv"] =  {0.3, 0.4}
#         original_benckmarks[path_penn_bench + "cleve/cleve.csv"] =  {0.1, 0.2}
        
        d = {0.05, 0.1, 0.2, 0.5}
        s = {"appendicitis", "australian", "auto", "backache", "balance", "biomed", "breast-cancer", "bupa", "cars", "cleve", "cleveland-nominal", "cleveland", "cloud", "colic", "contraceptiveM", "dermatology", "diabetes", "ecoli", "glass", "glass2", "haberman", "hayes-roth", "heart-c", "heart-h", "heart-statlog", "hepatitis", "hungarian", "liver-disorder", "lupus", "lymphography", "molecular-biology_promoters", "new-thyroid", "postoperative-patient-data", "promoters", "schizo", "shuttleM", "soybean", "spect", "tae"}
        for e in s:
            original_benckmarks[path_penn_bench +   e + "/" + e + "-un.csv"] =  d
        print(original_benckmarks)
        #exit()
         
         
         
        
#         
#          name = path_others_bench + "HouseVotes/house-votes-84.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.95}
# 
# 
# 
#         #name = path_others_bench + "Mammography/uci_mammo_data.csv"
#         #original_benckmarks[name] =  {}
#         #acc[name] =  {0.9}
# 
#          name = path_others_bench + "Titanic/titanic.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {-1}
        
                
#          name = path_penn_bench + "flags/flags.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {-1}
#        
#          name = path_penn_bench + "haberman/haberman.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.77}
#            
#          name  = path_penn_bench + "heart-c/heart-c.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.87}
#            
#          name = path_penn_bench + "heart-h/heart-h.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.85}
#    
#          name = path_penn_bench + "heart-statlog/heart-statlog.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.87}
#    
#          name = path_penn_bench + "hepatitis/hepatitis.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.87}
#    
#          name = path_penn_bench + "hungarian/hungarian.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.85}
#    
#          name = path_penn_bench + "irish/irish.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {-1}
#    
#          #name  = path_penn_bench + "liver-disorder/liver-disorder.csv"
#          #original_benckmarks[name] =  {}
#          #acc[name] =  {0.85}
#    
#          #name = path_penn_bench + "lupus/lupus.csv"
#          #original_benckmarks[name] =  {}
#          #acc[name] =  {0.70}
#    
#          name = path_penn_bench + "spect/spect.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.87}
#    
#          #name = path_penn_bench + "molecular-biology_promoters/molecular-biology_promoters.csv"
#          #original_benckmarks[name] =  {}
#          #acc[name] =  {0.85}
#    
#          name = path_penn_bench + "mux6/mux6.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {-1}
#    
#          #name  = path_penn_bench + "postoperative-patient-data/postoperative-patient-data.csv"
#          #original_benckmarks[name] =  {}            
#          #acc[name] =  {0.95}
#            
#          name = path_penn_bench + "promoters/promoters.csv"
#          original_benckmarks[name] =  {}            
#          acc[name] =  {0.97}
#            
#          #name =  path_penn_bench + "glass2/glass2.csv"
#          #original_benckmarks[name] =  {}            
#          #acc[name] =  {0.85}
#    
#    
#          name = path_manu_bench + "Mouse/mouse.csv"
#          original_benckmarks[name] = {}
#          acc[name] =  {-1}
#     
#          name = path_manu_bench + "Weather/meteo.csv"
#          original_benckmarks[name] = {}
#          acc[name] =  {-1}
#    
#          name = path_manu_bench + "Car/car.csv"
#          original_benckmarks[name] = {}#, 0.2, 0.3,0.5,0.7,0.9}
#          acc[name] =  {0.90}
#    
#          name = path_manu_bench + "Cancer/cancer.csv"
#          original_benckmarks[name] = {}#, 0.5, 0.7,0.9} 
#          acc[name] =  {0.95}
#    
#          name = path_penn_bench + "ShuttleM/shuttleM.csv"
#          original_benckmarks[name] = {}
#          acc[name] =  {0.99}
#    
#         #name  = path_penn_bench + "Balance/balance.csv"
#         #original_benckmarks[name] = {}
#         #acc[name] =  {0.97}
#    
#          name = path_penn_bench + "Colic/colic.csv"
#          original_benckmarks[name] = {}
#          acc[name] =  {0.86}
#    
#          name = path_penn_bench + "appendicitis/appendicitis.csv"
#          original_benckmarks[name] = {}
#          acc[name] =  {0.9}
#    
#          name  = path_penn_bench + "australian/australian.csv"
#          original_benckmarks[name] = {}
#          acc[name] =  {0.87}
#            
#          name = path_penn_bench + "backache/backache.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.9}
#            
#          #name = path_penn_bench + "biomed/biomed.csv"
#          #original_benckmarks[name] =  {}
#          #acc[name] =  {0.85}
#    
#          #name = path_penn_bench + "bupa/bupa.csv"
#          #original_benckmarks[name] =  {}
#          #acc[name] =  {0.85}
#    
#          name  = path_penn_bench + "cleve/cleve.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.85}
#            
#          #name = path_penn_bench + "diabetes/diabetes.csv"
#          #original_benckmarks[name] =  {}
#          #acc[name] =  {0.85}
#    
#          name  = path_penn_bench + "new-thyroid/new-thyroid.csv"
#          original_benckmarks[name] =  {}
#          acc[name] =  {0.99}
#            
#          name = path_penn_bench + "Corral/corral.csv"
#          original_benckmarks[name] = {}
#          acc[name] =  {-1}
#  
# #         #for bench_name, ratios in original_benckmarks.items():
# #            #print(" bench name ", bench_name)
# #            #un_file =  bench_name.replace(".csv", LABEL_SUFFIX_UNARY_CSV)
# #            #print(un_file)
# #            #if ("Emmanuel" not in bench_name):             
# #            #    exe = "python src/scripts/d2bin.py " + bench_name  + " > dummy 2>" + bench_name.replace(".csv", LABEL_SUFFIX_UNARY_EXT_CSV)
# #            #else:
# #            #    exe = "python src/scripts/d2bin.py " + bench_name  + " > " + un_file +  " 2>" + bench_name.replace(".csv", LABEL_SUFFIX_UNARY_EXT_CSV)
# #            #print(exe)
# #            #os.system(exe)
             
            #exit()

    #exit()

    if (gen_reduced_scilearn != True):

#         original_benckmarks[path_penn_bench + "flags/flags-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "haberman/haberman-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "heart-c/heart-c-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "heart-h/heart-h-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "heart-statlog/heart-statlog-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "hepatitis/hepatitis-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "hungarian/hungarian-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "irish/irish-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         #original_benckmarks[path_penn_bench + "liver-disorder/liver-disorder-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         #original_benckmarks[path_penn_bench + "lupus/lupus-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "spect/spect-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         #original_benckmarks[path_penn_bench + "molecular-biology_promoters/molecular-biology_promoters-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "mux6/mux6-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         #original_benckmarks[path_penn_bench + "postoperative-patient-data/postoperative-patient-data-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}            
#         original_benckmarks[path_penn_bench + "promoters/promoters-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}     
#         #original_benckmarks[path_penn_bench + "glass2/glass2-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}            
#                    
#         original_benckmarks[path_manu_bench + "Mouse/mouse-un" + LABEL_SUFFIX_REDUCED_CSV ] = {}
#         original_benckmarks[path_manu_bench + "Weather/meteo-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         original_benckmarks[path_manu_bench + "Car/car-un" + LABEL_SUFFIX_REDUCED_CSV] = {}#, 0.2, 0.3,0.5,0.7,0.9}
#         original_benckmarks[path_manu_bench + "Cancer/cancer-un" + LABEL_SUFFIX_REDUCED_CSV] = {}#, 0.5, 0.7,0.9} 
#         original_benckmarks[path_penn_bench + "ShuttleM/shuttleM-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         #original_benckmarks[path_penn_bench + "Balance/balance-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         original_benckmarks[path_penn_bench + "Colic/colic-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         original_benckmarks[path_penn_bench + "appendicitis/appendicitis-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         original_benckmarks[path_penn_bench + "australian/australian-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         original_benckmarks[path_penn_bench + "backache/backache-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         #original_benckmarks[path_penn_bench + "biomed/biomed-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         #original_benckmarks[path_penn_bench + "bupa/bupa-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "cleve/cleve-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         #original_benckmarks[path_penn_bench + "diabetes/diabetes-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "new-thyroid/new-thyroid-un" + LABEL_SUFFIX_REDUCED_CSV] =  {}
#         original_benckmarks[path_penn_bench + "Corral/corral-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#  
#         original_benckmarks[path_others_bench + "HouseVotes/house-votes-84-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         #original_benckmarks[path_others_bench + "Mammography/uci_mammo_data-un" + LABEL_SUFFIX_REDUCED_CSV] = {}
#         original_benckmarks[path_others_bench + "Titanic/titanic-un" + LABEL_SUFFIX_REDUCED_CSV] = {}

#         original_benckmarks[path_penn_bench + "flags/flags-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "haberman/haberman-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "heart-c/heart-c-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "heart-h/heart-h-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "heart-statlog/heart-statlog-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "hepatitis/hepatitis-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "hungarian/hungarian-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "irish/irish-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "liver-disorder/liver-disorder-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "lupus/lupus-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "spect/spect-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "molecular-biology_promoters/molecular-biology_promoters-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "mux6/mux6-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "postoperative-patient-data/postoperative-patient-data-un" + ".csv"] =  {}            
#         original_benckmarks[path_penn_bench + "promoters/promoters-un" + ".csv"] =  {}     
#         #original_benckmarks[path_penn_bench + "glass2/glass2-un" + ".csv"] =  {}            
#                  
#         original_benckmarks[path_manu_bench + "Mouse/mouse-un" + ".csv" ] = {}
#         original_benckmarks[path_manu_bench + "Weather/meteo-un" + ".csv"] = {}
#         original_benckmarks[path_manu_bench + "Car/car-un" + ".csv"] = {}#, 0.2, 0.3,0.5,0.7,0.9}
#         original_benckmarks[path_manu_bench + "Cancer/cancer-un" + ".csv"] = {}#, 0.5, 0.7,0.9} 
#         original_benckmarks[path_penn_bench + "ShuttleM/shuttleM-un" + ".csv"] = {}
#         original_benckmarks[path_penn_bench + "Balance/balance-un" + ".csv"] = {}
#         original_benckmarks[path_penn_bench + "Colic/colic-un" + ".csv"] = {}
#         original_benckmarks[path_penn_bench + "appendicitis/appendicitis-un" + ".csv"] = {}
#         original_benckmarks[path_penn_bench + "australian/australian-un" + ".csv"] = {}
#         original_benckmarks[path_penn_bench + "backache/backache-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "biomed/biomed-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "bupa/bupa-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "cleve/cleve-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "diabetes/diabetes-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "new-thyroid/new-thyroid-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "Corral/corral-un" + ".csv"] = {}
#   
#         original_benckmarks[path_penn_bench + "flags/flags-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "haberman/haberman-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "heart-c/heart-c-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "heart-h/heart-h-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "heart-statlog/heart-statlog-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "hepatitis/hepatitis-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "hungarian/hungarian-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "irish/irish-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "liver-disorder/liver-disorder-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "lupus/lupus-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "spect/spect-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "molecular-biology_promoters/molecular-biology_promoters-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "mux6/mux6-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "postoperative-patient-data/postoperative-patient-data-un" + ".csv"] =  {}            
#         original_benckmarks[path_penn_bench + "promoters/promoters-un" + ".csv"] =  {}     
#         #original_benckmarks[path_penn_bench + "glass2/glass2-un" + ".csv"] =  {}            
#                    
#         original_benckmarks[path_manu_bench + "Mouse/mouse-un"  + ".csv"] =  {}
#         original_benckmarks[path_manu_bench + "Weather/meteo-un"  + ".csv"] =  {}
#         original_benckmarks[path_manu_bench + "Car/car-un"  + ".csv"] =  {}#, 0.2, 0.3,0.5,0.7,0.9}
#         original_benckmarks[path_manu_bench + "Cancer/cancer-un"  + ".csv"] =  {}#, 0.5, 0.7,0.9} 
#         original_benckmarks[path_penn_bench + "ShuttleM/shuttleM-un"  + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "Balance/balance-un"  + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "Colic/colic-un"  + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "appendicitis/appendicitis-un"  + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "australian/australian-un"  + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "backache/backache-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "biomed/biomed-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "bupa/bupa-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "cleve/cleve-un" + ".csv"] =  {}
#         #original_benckmarks[path_penn_bench + "diabetes/diabetes-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "new-thyroid/new-thyroid-un" + ".csv"] =  {}
#         original_benckmarks[path_penn_bench + "Corral/corral-un"  + ".csv"] =  {}
#  
#         original_benckmarks[path_others_bench + "HouseVotes/house-votes-84-un"  + ".csv"] =  {}
#         #original_benckmarks[path_others_bench + "Mammography/uci_mammo_data-un"  + ".csv"] =  {}
#         original_benckmarks[path_others_bench + "Titanic/titanic-un"  + ".csv"] =  {}

        original_benckmarks[path_manu_bench + "Mouse/mouse" + ".csv"] = {}
        original_benckmarks[path_manu_bench + "Weather/meteo" + ".csv"] = {}
        original_benckmarks[path_manu_bench + "Car/car" + ".csv"] = {0.05, 0.1}#, 0.2, 0.3,0.5,0.7,0.9}
        original_benckmarks[path_manu_bench + "Cancer/cancer" + ".csv"] = {0.1, 0.2}#, 0.5, 0.7,0.9} 
        original_benckmarks[path_penn_bench + "ShuttleM/shuttleM" + ".csv"] = {0.05, 0.1}
        original_benckmarks[path_penn_bench + "Colic/colic" + ".csv"] = {0.05, 0.1}
        original_benckmarks[path_penn_bench + "appendicitis/appendicitis" + ".csv"] = {0.3, 0.4}
        original_benckmarks[path_penn_bench + "australian/australian" + ".csv"] = {0.05, 0.1}
        original_benckmarks[path_penn_bench + "backache/backache" + ".csv"] =  {0.3, 0.4}
        original_benckmarks[path_penn_bench + "cleve/cleve" + ".csv"] =  {0.1, 0.2}
    
    #original_benckmarks[path_penn_bench + "new-thyroid/new-thyroid.csv"] =  {0.1, 0.2}
    #original_benckmarks[path_penn_bench + "ContraceptiveM/contraceptiveM.csv"] = {0.1, 0.3, 0.5}
    #original_benckmarks[path_penn_bench + "Corral/corral.csv"] = {0.7, 0.9}
    
    if (gen_parse_results == True):
        #parse_results2()
        parse_results(nb_samples_bench)
        exit()
    
    
    #original_benckmarks[path_manu_bench + "Income/income.csv"] =  {0.01, 0.015, 0.005}

#     
    
    
    gen_runs_all =open("list_dt", 'w+')
    commands_all = []


    for bench_name, ratios in original_benckmarks.items():
        print(bench_name)
    
        head, tail = os.path.split(bench_name)
        output_file =  os.path.basename(bench_name)
        output_dir = head  +  "/Samples/"   
        try:
            os.stat(output_dir)
        except:
            os.mkdir(output_dir)       
        print("dir", output_dir)    
        output_path = output_dir + output_file
        
        
        print(output_path)
        data = Data(filename=bench_name, mapfile=options.mapfile,
                separator=options.separator)
        
        l = len(ratios)
        print(l)
        if (l == 0):
            # we do nto sample
            print(output_path)
            if (gen_benchs == True):
                #bin_file = convertor2binary.convertor2binary(data = data, output_file = output_path)
                bin_file =  output_path.replace(".csv","-un.csv")
                bin_file_copy_from =  bin_file.replace("Samples/", "")
                os.system(" cp " + bin_file_copy_from + " " + bin_file)
                if (gen_reduced_scilearn == True):
                    reduced_bin_file = convertor2binary.convertor2reduced(data = data, bin_file = bin_file, acc = acc[bench_name])
                
                convertor2binary.convertor2iti(data = data, output_file = output_path, file_to_run = iti_all)
            if (gen_runs == True):                
                results_dir = output_dir.replace("Samples", LABEL_SUFFIX_DIR_RESULTS)
                commands_all.insert(0,"mkdir " + results_dir)
                
                
                s = "/home/nina/workspace/xai/timeout -t 1000 -m 20000000 "
                s += "python src/mindt/mindt.py -a dtencoding "
                s+= "--iti " + output_path + "." + LABEL_SUFFIX_RESULTS + " "
                s += output_path.replace("/Samples","")  
                s += "  >  " + results_dir + output_file + LABEL_SUFFIX_SAVE_RESULTS
                commands_all.append(s)
                
            if (gen_feat_reduce == True):    
            # collect stats
                convertor2binary.convertor2iti_collect_stats(data = data, output_file = output_path)    
               
                    
        for r in ratios:
            print (r)
            output_dir_r = output_dir  +  "/" + str(r)  + "/"   
            try:
                os.stat(output_dir_r)
            except:
                os.mkdir(output_dir_r)       
            print("dir", output_dir_r)    

            rp = r#/(r+1)
            nb_samples = int(len(data.samps)*(rp))
            print(nb_samples)
            for i in range(nb_samples_bench):
                output_file =  os.path.basename(bench_name).replace(".csv", "_" + str(i) + ".csv")
                output_path = output_dir_r + output_file
                if (gen_benchs == True):
                    data_r = copy.deepcopy(data)
                    data_r.samps = random.sample(data.samps, nb_samples)
                    print(output_path)
                    #print(nb_samples, len(data.samps))
                    convertor2binary.convertor2binary(data = data_r, output_file = output_path)
                    #iti_all =open("execute_iti", 'w+')
                    #convertor2binary.convertor2iti(data = data_r, output_file = output_path, file_to_run = iti_all)            
                    #iti_all.close()
                    #subprocess.call(["python2.7", "rq_test2_1.py"])
                    #pause(3)  
                           
                if (gen_runs == True):
                    results_dir_up = output_dir.replace("Samples", "Results")
                    results_dir = output_dir_r.replace("Samples", "Results")
                    commands_all.insert(0,"mkdir  "+ results_dir_up)
                    commands_all.insert(0,"mkdir  "+ results_dir)
                    
                    
                    s = "/home/nina/workspace/xai/timeout -t 1000 -m 20000000 "
                    s += "python src/mindt/mindt.py -a dtencoding "
                    s+= "--iti " + output_path + "." + LABEL_SUFFIX_RESULTS + " "
                    s += output_path.replace(".csv","-un.csv") 
                    s += "  >  " + results_dir + output_file + "_save"
                    commands_all.append(s)
                if (gen_feat_reduce == True):    
                    # collect stats
                    convertor2binary.convertor2iti_collect_stats(data = data, output_file = output_path)    

    print(commands_all)
    for s in commands_all:
        print(s, file = gen_runs_all)
    gen_runs_all.close()