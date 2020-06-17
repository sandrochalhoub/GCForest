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
        res['optimal'] = [0]

        for line in output:

            # if line.find('Assertion') >= 0:
            #     continue

            if line.find('optimal') >= 0:
                self.store('optimal',1,res)

            if not line.startswith(self.dataflag):
                continue

            data = line[len(self.dataflag):].split()
            for st in data:
                stat,val = st.split(self.equal)
                if res['optimal'][-1] == 0:
                    self.store('sol'+stat,val,res)
                else:
                    self.store('proof'+stat,val,res)
                self.store(stat,val,res)

        return res


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, GenericParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    o = Observation(e, parsers)
    count = Statistics("count", label="count")
    features = Statistic("feature", label="\\# feat.")
    duplicates = Statistic('duplicate', label='\\# dupli.')
    incoherent = Statistic("suppressed", label="\\# inc.")
    final_count = Statistic('final_count', label='final count')

    m_dataset = Method('dataset', stats=[count, features, duplicates, incoherent, final_count])

    o.write_table('tex/dataset_props.tex', [m_dataset], benches)
