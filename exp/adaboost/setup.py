#! /usr/bin/env python

from rocknrun import *

def setup():

    # create a keyfile (somename.key)
    keyfile = open('adaboost.key', 'w')

    methods = []

    split = 0.2
    max_depth = 4
    ada_it = 30

    # Cart methods
    cart_base = "python3 ../../code/examples/scripts/adaboost.py #BENCHMARK --seed #SEED --split %f --max_depth %i --n_estimators %i" % (split, max_depth, ada_it)
    methods.append(("cart", cart_base))

    # Bud methods
    bud_base = "../../code/bin/adaboost #BENCHMARK --seed #SEED --split %f --max_depth %i --ada_it %i --search 100000 --print_par" % (split, max_depth, ada_it)
    methods.append(("bud", bud_base))

    keyfile.write('%d methods\n'%len(methods))

    for name, command in methods:
        keyfile.write(name + "\n")
        keyfile.write(command + "\n")

    # declare the benchmarks (print_benchlist assumes that everything in benchfolder is an instance file)
    benchfolder = '../datasets/'
    print_benchlist(benchfolder, keyfile)

    # declare some seeds
    keyfile.write('123 456\n')

    keyfile.close()


if __name__ == '__main__' :
    setup()
    e = Experiment()
    e.generate_jobs(timeout='00:10:00')
