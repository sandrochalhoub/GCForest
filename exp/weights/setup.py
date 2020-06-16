#! /usr/bin/env python

from rocknrun import *

def setup():

    # create a keyfile (somename.key)
    keyfile = open('weighted.key', 'w')

    methods = []

    max_depth = 5
    search_size = 100000000

    # no weights methods
    no_weights = "../../code/bin/bud_first_search #BENCHMARK --seed #SEED --max_depth %i --search %i --print_par" % (max_depth, search_size)
    methods.append(("no weights", no_weights))

    # weights methods
    weights = "../../code/bin/bud_first_search #BENCHMARK --seed #SEED --max_depth %i --use_weights --search %i --print_par" % (max_depth, search_size)
    methods.append(("weights", weights))

    keyfile.write('%d methods\n'%len(methods))

    for name, command in methods:
        keyfile.write(name + "\n")
        keyfile.write(command + "\n")

    # declare the benchmarks (print_benchlist assumes that everything in benchfolder 'is an instance file)
    benchfolder = '/net/phorcys/data/roc/eh/dt/simp/'
    print_benchlist(benchfolder, keyfile)

    # declare some seeds
    keyfile.write('123 456\n')

    keyfile.close()


if __name__ == '__main__' :
    setup()
    e = Experiment()
    e.generate_jobs(timeout='01:00:00')
