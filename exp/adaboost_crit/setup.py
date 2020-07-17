#! /usr/bin/env python

from rocknrun import *

def setup():

    # create a keyfile (somename.key)
    keyfile = open('adaboost.key', 'w')

    methods = []

    split = 0.2
    max_depth = 4
    ada_it = 30

    # error
    error_base = "../../code/bin/adaboost #BENCHMARK --seed #SEED --split %f --max_depth %i --ada_it %i --print_par --erroronly" % (split, max_depth, ada_it)
    methods.append(("error", error_base))

    # error depth
    depth_base = "../../code/bin/adaboost #BENCHMARK --seed #SEED --split %f --max_depth %i --ada_it %i --print_par --depthonly" % (split, max_depth, ada_it)
    methods.append(("depth", depth_base))

    # depth size
    size_base = "../../code/bin/adaboost #BENCHMARK --seed #SEED --split %f --max_depth %i --ada_it %i --print_par" % (split, max_depth, ada_it)
    methods.append(("size", size_base))

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
    e.generate_jobs(timeout='03:00:00')
