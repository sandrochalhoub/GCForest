#! /usr/bin/env python

from rocknrun import *

def setup():

    # create a keyfile (somename.key)
    keyfile = open('dataset.key', 'w')

    methods = []


    # dataset methods
    dataset_base = "../../code/bin/bud_first_search #BENCHMARK --print_par --max_depth 2 --use_weights --filter_inconsistent"
    methods.append(("dataset", dataset_base))

    keyfile.write('%d methods\n'%len(methods))

    for name, command in methods:
        keyfile.write(name + "\n")
        keyfile.write(command + "\n")

    # declare the benchmarks (print_benchlist assumes that everything in benchfolder is an instance file)
    benchfolder = '/net/phorcys/data/roc/eh/dt/simp/'
    print_benchlist(benchfolder, keyfile)

    keyfile.close()


if __name__ == '__main__' :
    setup()
    e = Experiment()
    e.generate_jobs(timeout='00:30:00')
