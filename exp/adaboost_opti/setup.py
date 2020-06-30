#! /usr/bin/env python

from rocknrun import *

def setup():

    # create a keyfile (somename.key)
    keyfile = open('adaboost.key', 'w')

    methods = []

    split = 0.2
    max_depth = 4 
    search_limit = 100000
    ada_it = 30

    # Interrupted
    int_base = "../../code/bin/adaboost #BENCHMARK --seed #SEED --split %f --max_depth %i --ada_it %i --search %i --print_par" % (split, max_depth, ada_it, search_limit)
    methods.append(("interrupted", int_base))

    # Optimal
    opti_base = "../../code/bin/adaboost #BENCHMARK --seed #SEED --split %f --max_depth %i --ada_it %i --print_par" % (split, max_depth, ada_it)
    methods.append(("optimal", opti_base))

    keyfile.write('%d methods\n'%len(methods))

    for name, command in methods:
        keyfile.write(name + "\n")
        keyfile.write(command + "\n")

    # declare the benchmarks (print_benchlist assumes that everything in benchfolder is an instance file)
    benchfolder = '/net/phorcys/data/roc/eh/dt/simp/'
    print_benchlist(benchfolder, keyfile)

    # declare some seeds
    keyfile.write('123 456\n')

    keyfile.close()


if __name__ == '__main__' :
    setup()
    e = Experiment()
    e.generate_jobs(timeout='02:00:00')
