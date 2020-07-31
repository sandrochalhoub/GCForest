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

        # Compute average tree size:
        if "ada_size" in res:
            ada_size = res["ada_size"]
            self.store("avg_size", sum(ada_size) / len(ada_size), res)

        return res


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, DTParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    o = Observation(e, parsers)
    train_acc = Statistic('ada_train_acc', label='train acc.', precision=lambda x:3, best=max)
    test_acc = Statistic('ada_test_acc', label='test acc.', precision=lambda x:3, best=max)
    tree_size = Statistic('avg_size', label='tree size', precision=lambda x:2, best=min)

    l_ada_it = [1, 5, 10, 30, 100]

    # One table per max_depth
    for ada_it in l_ada_it:
        m_cart = Method('cart_%i' % ada_it, stats=[train_acc, test_acc, tree_size])
        m_bud = Method('bud_%i' % ada_it, stats=[train_acc, test_acc, tree_size])

        o.write_table('tex/iteration_%i.tex' % ada_it, [m_cart,m_bud], benches)
