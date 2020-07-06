#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
class GenericParser(object):

    def __call__(self,respath):
        res = {}
        for line in respath:
            if line.startswith("r "):
                data = line[2:].strip().split("=")
                res[data[0].strip()] = [float(data[1])]
        return res

def write_methods_table(o, tabname, methods):
    with open(tabname, 'r') as tabfile:
        # header
        tabfile.write("")
        # columns
        tabfile.write("")

        for ... :
            tabfile.write("\\texttt{")

            # values
            for ...:
                tabfile.write("")

            tabfile.write("}")


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, GenericParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    o = Observation(e, parsers)
    train_acc = Statistic('train acc', label='train acc.', precision=lambda x:3, best=)
    test_acc = Statistic('test acc', label='test acc.', precision=lambda x:3)

    l_max_depth = [3, 4, 5, 7, 10, 15]

    for max_depth in l_max_depth:
        m_cart = Method('cart_%i' % max_depth, stats=[train_acc, test_acc])
        m_bud = Method('bud_%i' % max_depth, stats=[train_acc, test_acc])

        o.write_table('tex/max_depth_%i.tex' % max_depth, [m_cart,m_bud], benches)

    # Summary table
    for b in benches:
        methods = []

        for max_depth in l_max_depth:
            methods += [
                Method('cart_%i' % max_depth, stats=[train_acc, test_acc]),
                Method('bud_%i' % max_depth, stats=[train_acc, test_acc])
            ]

        # o.write_summary_table('tex/%s.tex' % b.label, )
