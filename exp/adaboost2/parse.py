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


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, GenericParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    o = Observation(e, parsers)
    train_acc = Statistic('train acc', label='train acc.', precision=lambda x:3, best=max)
    test_acc = Statistic('test acc', label='test acc.', precision=lambda x:3, best=max)

    l_ada_it = [1, 5, 10, 30, 100]

    # One table per max_depth
    for ada_it in l_ada_it:
        m_cart = Method('cart_%i' % ada_it, stats=[train_acc, test_acc])
        m_bud = Method('bud_%i' % ada_it, stats=[train_acc, test_acc])

        o.write_table('tex/iteration_%i.tex' % ada_it, [m_cart,m_bud], benches)
