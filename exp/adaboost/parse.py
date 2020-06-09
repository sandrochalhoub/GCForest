#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
class GenericParser(object):

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
    parsers = dict([(m, GenericParser()) for m in e.all_methods])

    # Data needed for the table:
    # - number of examples in dataset
    # - train accuracy & test accuracy

    # What I want to do:
    # Print a table with header:
    header = """
        \\begin{tabular}{|*{6}{c|}}
        \\hline
        \\multirow{2}{*}{Dataset} & \\multirow{2}{*}{\\#s} & \\multicolumn{2}{l|}{BFS acc} & \\multicolumn{2}{l|}{CART acc} \\\\
        \\cline{3-6}
          &  & train & test & train & test \\\\
        \\hline
        """


    o = Observation(e, parsers)
    o.write_summary_table('tex/summary.tex', ['train', 'test'], precisions=[0,2])
    o.write_large_table('tex/all.tex', ['train', 'test'], precisions=[0,2])
    # o.write_cactus('tex/cactus.tex', X='number of conflicts', Y='cpu time', precision=1, epsilon=.01)
    compile_latex()
