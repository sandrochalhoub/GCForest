#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
class GenericParser(object):

    def __call__(self,output):
        res = {}
        read_result = False

        for line in output:
            # time
            if read_result:
                vals = line.split()

                for data in [a.split("=") for a in vals]:
                    if len(data) != 2:
                        continue
                    if data[0] == "time":
                        res[data[0]] = [float(data[1])]

            if line.find('optimal') >= 0 || line.find('interrupted') >= 0:
                read_result = True

            # other stats
            if line.startswith("r "):
                data = line[2:].strip().split("=")
                res[data[0].strip()] = [float(data[1])]

        return res


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, GenericParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    # Parsed material: #s, %duplicate, time method 1, time method 2

    o = Observation(e, parsers)
    time = Statistic('time', label= 'time', precision=lambda x:3)
    sample_count = Statistic('sample_count', label='\\#s')
    duplicate = Statistic('duplicate', label='\\% dupli.', precision=lambda x:3)

    m_no_weights = Method('no weights', stats=[time])
    m_weights = Method('weights', stats=[time, duplicate])

    o.write_table('tex/weighted.tex', [m_no_weights,m_weights], benches, info=[sample_count])

    # compile_latex()
