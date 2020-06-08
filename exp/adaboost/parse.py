#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
class AdaboostParser(object):

    def __call__(self,respath):
        res = {}
        for line in open(respath, 'r'):
            if line.startswith("r "):
                stats_line = line[2:]
                data = line.split("=")

                res[data[0]] = data[1]
        return res


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, AdaboostParser()) for m in e.all_methods])

    # Data needed for the table:
    # - number of examples in dataset
    # - train accuracy & test accuracy

    o = Observation(e, parsers)
    o.write_summary_table('tex/summary.tex', ['number of conflicts', 'cpu time'], precisions=[0,2])
    o.write_large_table('tex/all.tex', ['number of conflicts', 'cpu time'], precisions=[0,2])
    o.write_cactus('tex/cactus.tex', X='number of conflicts', Y='cpu time', precision=1, epsilon=.01)
    compile_latex()
