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
    parsers = dict([(m, DTParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    # Parsed material: #s, %duplicate, time method 1, time method 2

    o = Observation(e, parsers)

    time = Statistic('time', label= 'time', precision=lambda x:2)
    sample_count = Statistic('sample_count', label='\\#s')
    dratio = Statistic('dratio', label='\\% dupli.', precision=lambda x:3)
    accuracy = Statistic('accuracy', label='acc.', precision=lambda x:3)

    m_no_weights_success = Method('no weights', stats=[time])
    m_weights_success = Method('weights', stats=[time, dratio])

    m_no_weights_fails = Method('no weights', stats=[accuracy])
    m_weights_fails = Method('weights', stats=[accuracy, dratio])

    benches_success = []
    benches_fail = []

    for b in benches:
        m = next(iter( e.all_methods))

        if o.data[m][b.label][e.seeds["weighted"][0]]["optimal"][-1] != 0:
            benches_success.append(b)
        else:
            benches_fail.append(b)


    # o.write_table('tex/weighted.tex', [m_no_weights,m_weights], benches, info=[sample_count])
    o.write_table('tex/weighted_success.tex', [m_no_weights_success,m_weights_success], benches_success)
    o.write_table('tex/weighted_fails.tex', [m_no_weights_fails, m_weights_fails], benches_fail)

    # compile_latex()
