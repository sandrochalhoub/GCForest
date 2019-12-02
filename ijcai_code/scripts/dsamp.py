#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## dsamp.py
##
##  Created on: Jan 29, 2018
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

#
#==============================================================================
from __future__ import print_function
import collections
import getopt
import math
import os
import random
import sys


#
#==============================================================================
class Data(object):
    """
        A class for simplifying a (training) data set.
    """

    def __init__(self, from_fp=None, separator=None):
        """
            Constructor.
        """

        self.names = None
        self.feats = None
        self.samps = None

        if from_fp:
            self.from_fp(fp=from_fp, separator=separator)

    def from_fp(self, fp=None, separator=','):
        """
            Get data from a CSV file.
        """

        if fp:
            lines = fp.readlines()

            # reading preamble
            self.names = lines[0].strip().split(separator)
            self.feats = [set([]) for n in self.names]
            del(lines[0])

            # filling name to id mapping
            self.nm2id = {name: i for i, name in enumerate(self.names)}

            # reading training samples
            self.samps, self.classes = [], {}

            for line in lines:
                sample = line.strip().split(separator)

                for i, f in enumerate(sample):
                    if f:
                        self.feats[i].add(f)

                self.samps.append(sample)

                # clasterizing the samples
                if sample[-1] not in self.classes:
                    self.classes[sample[-1]] = []

                self.classes[sample[-1]].append(len(self.samps) - 1)

    def subsample(self, perc):
        """
            Subsample a given dataset.
        """

        print(','.join(self.names))
        nof_samps = int(math.ceil(len(self.samps) * perc / len(self.classes)))

        random.seed()
        for lb in self.classes:
            samps = set([])

            while len(samps) < nof_samps and len(samps) < len(self.classes[lb]):
                choice = random.choice(self.classes[lb])
                samps.add(choice)

            for s in samps:
                print(','.join(self.samps[s]))


#
#==============================================================================
def parse_options():
    """
        Parses command-line options.
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'hp:s:',
                                   ['perc=',
                                    'help',
                                    'sep='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize() + '\n')
        usage()
        sys.exit(1)

    perc = 0.1
    sep = ','

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-p', '--perc'):
            perc = float(arg)
        elif opt in ('-s', '--sep'):
            sep = str(arg)
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    return perc, sep, args


#
#==============================================================================
def usage():
    """
        Prints help message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] file')
    print('Options:')
    print('        -p, --perc=<float>     Percentage of the samples to use')
    print('                               Available values: (0 .. 1] (default = 0.1)')
    print('        -h, --help')
    print('        -s, --sep=<string>     Separator used in the input CSV file (default = \',\')')


#
#==============================================================================
if __name__ == '__main__':
    perc, sep, files = parse_options()

    if files:
        with open(files[0]) as fp:
            data = Data(from_fp=fp, separator=sep)
    else:
        data = Data(from_fp=sys.stdin, separator=sep)

    data.subsample(perc)
