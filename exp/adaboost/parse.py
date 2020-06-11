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
    train_acc = Statistic('train acc', label='train acc.', precision=lambda x:3)
    test_acc = Statistic('test acc', label='test acc.', precision=lambda x:3)

    m_cart = Method('cart', stats=[train_acc, test_acc])
    m_bud = Method('bud', stats=[train_acc, test_acc])

    o.write_table('tex/tabfile.tex', [m_cart,m_bud], benches)
