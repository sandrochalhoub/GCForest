#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
class GenericParser(object):

    def __call__(self,output):
        res = {}
        read_result = False

        for line in output:
            # time
            vals = line.split()

            for data in [a.split("=") for a in vals]:
                if len(data) != 2:
                    continue
                if data[0] == "dratio":
                    res[data[0]] = [float(data[1])* 100]
                if data[0] == "time" and read_result:
                    res[data[0]] = [float(data[1])]

            if line.find('optimal') >= 0 or line.find('interrupted') >= 0:
                read_result = True
        
        return res


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, GenericParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    # Parsed material: #s, %duplicate, time method 1, time method 2

    o = Observation(e, parsers)
    time = Statistic('time', label= 'time', precision=lambda x:3)
    sample_count = Statistic('sample_count', label='\\#s')
    dratio = Statistic('dratio', label='\\% dupli.', precision=lambda x:1)

    m_no_weights = Method('no weights', stats=[time])
    m_weights = Method('weights', stats=[time, dratio])

    # o.write_table('tex/weighted.tex', [m_no_weights,m_weights], benches, info=[sample_count])
    o.write_table('tex/weighted.tex', [m_no_weights,m_weights], benches)

    # compile_latex()
