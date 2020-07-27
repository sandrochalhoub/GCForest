#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
class DTParser(object):
    def __init__(self, separator=' ', equal='=', dataflag='d'):
        self.equal = equal
        self.separator = separator
        self.dataflag = dataflag

    def store(self, stat, vstr, res):
        stat = stat.strip()
        if not res.has_key(stat):
            res[stat] = []
        val = None
        try:
            val = int(vstr)
        except:
            val = float(vstr)
        res[stat].append(val)

    def __call__(self,output):

        res = {}

        for line in output:
            if not line.startswith(self.dataflag):
                continue

            data = line[len(self.dataflag):].split()
            for st in data:
                stat,val = st.split(self.equal)
                self.store(stat,val,res)

        return res


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, DTParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    o = Observation(e, parsers)
    train_acc = Statistic('ada_train_acc', label='train acc.', precision=lambda x:3, best=max)
    test_acc = Statistic('ada_test_acc', label='test acc.', precision=lambda x:3, best=max)
    time = Statistic('ada_time', label='time', precision=lambda x:3, best=min)

    m_error = Method('error', stats=[train_acc, test_acc, time])
    m_depth = Method('depth', stats=[train_acc, test_acc, time])
    m_size = Method('size', stats=[train_acc, test_acc, time])

    o.write_table('tex/crit.tex', [m_error, m_size], benches)

    # compile_latex()
