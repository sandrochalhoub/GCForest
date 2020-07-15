#! /usr/bin/env python

from rocknrun import *

def print_benchmarks(folder, names, keyfile):
    files, paths = get_benchlist(folder)

    keyfile.write("%i benchmarks\n" % len(names))

    for f,p in zip(files,paths):
        if f in names:
            keyfile.write("%s\n%s\n" % (f, p))

"""
Runs adaboost over a handful of different datasets with variable max_depth to
see if there is a real benefit over CART
"""

def setup():

    # create a keyfile (somename.key)
    keyfile = open('adaboost1.key', 'w')

    methods = []

    split = 0.2
    l_max_depth = [3, 4, 5, 7, 10, 15]
    ada_it = 30
    search_size = 3000000

    for max_depth in l_max_depth:
        # Cart methods
        cart_base = "python3 ../../code/examples/scripts/adaboost.py #BENCHMARK --seed #SEED --split %f --max_depth %i --n_estimators %i" % (split, max_depth, ada_it)
        methods.append(("cart_%i" % max_depth, cart_base))

        # Bud methods
        # bud_base = "../../code/bin/adaboost #BENCHMARK --seed #SEED --split %f --max_depth %i --ada_it %i --search %i --print_par" % (split, max_depth, ada_it, search_size)
        bud_base = "python3 ../../code/examples/scripts/adaboost.py #BENCHMARK --seed #SEED --split %f --max_depth %i --n_estimators %i --search %i" % (split, max_depth, ada_it, search_size)
        methods.append(("bud_%i" % max_depth, bud_base))

    keyfile.write('%d methods\n'%len(methods))

    for name, command in methods:
        keyfile.write(name + "\n")
        keyfile.write(command + "\n")

    # declare the benchmarks (print_benchlist assumes that everything in benchfolder 'is an instance file)
    benchfolder = '/net/phorcys/data/roc/eh/dt/simp/'
    # Use this if you want to only test some datasets and not every dataset in the folder
    """
    benchnames = [
        "taiwan_binarised"
    ]
    print_benchmarks(benchfolder, benchnames, keyfile)
    """
    print_benchlist(benchfolder, keyfile)

    # declare some seeds
    keyfile.write('123 456 4511\n')

    keyfile.close()


if __name__ == '__main__' :
    setup()
    e = Experiment()
    e.generate_jobs(timeout='02:00:00')
