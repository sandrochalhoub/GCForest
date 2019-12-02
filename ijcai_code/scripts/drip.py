#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## drip.py
##
##  Created on: Jan 16, 2018
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

# print function as in Python3
#==============================================================================
from __future__ import print_function

# for importing the correct version of pysat and hitman
#==============================================================================
import atexit
import collections
import getopt
import os
import re
import socket
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
        self.wghts = None

        # register the cleaning function
        self.files = []
        atexit.register(self.at_exit)

        if from_fp:
            self.from_fp(fp=from_fp, separator=separator)

        self.keep = [False for n in self.names[:-1]]
        self.keep.append(True)  # label

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
            self.samps, self.wghts = [], []

            for line, w in collections.Counter(lines).iteritems():
                sample = line.strip().split(separator)

                assert len(sample) == len(self.names), 'Wrong number of columns'
                for i, f in enumerate(sample):
                    if f:
                        self.feats[i].add(f)

                self.samps.append(sample)
                self.wghts.append(w)

    def dump_to_ripperk(self, files):
        """
            Dump the dataset in the RIPPERk format.
        """

        if files:
            fname = os.path.basename(os.path.splitext(files[0])[0])
        else:
            fname = 'ripperk'

        fname = '{0}.{1}@{2}'.format(fname, os.getpid(), socket.gethostname())

        self.files.extend(['{0}-attr.txt'.format(fname),
            '{0}-data.txt'.format(fname), '{0}-model.dat'.format(fname),
            '{0}-rules.txt'.format(fname)])

        # dumping attributes
        with open(self.files[0], 'w') as fp:
            for name, feats in zip(self.names, self.feats):
                print('{0} {1}'.format(name, ' '.join(feats)), file=fp)

        # dumping data
        with open(self.files[1], 'w') as fp:
            for s, w in zip(self.samps, self.wghts):
                for i in xrange(w):
                    print(' '.join(s), file=fp)

    def run_ripperk(self, k, p):
        """
            Run RIPPERk.
        """

        tool = '../ripperk/src/ripperk.py'
        attr = self.files[0]
        data = self.files[1]
        model = self.files[2]
        rules = self.files[3]
        label = self.names[-1]

        st = os.system('python {0} -e learn -a {1} -t {2} -c {3} -m {4} -o {5} -k {6} -p {7}'.format(
            tool, attr, data, label, model, rules, k, p))

        assert st == 0, 'Failed to run RIPPERk'

        # parsing results
        used_attributes = set([])
        for line in open(rules, 'r'):
            if line.startswith('ELSE'):
                break

            line = line[3:]  # dispose of 'IF'
            line = line.split(' THEN')[0]  # dispose of 'THEN'

            for lit in re.split(' \|\| | && ', line):
                used_attributes.add(lit.split(' == ')[0])

        for name in used_attributes:
            self.keep[self.nm2id[name]] = True

    def dump(self, fp=sys.stdout):
        """
            Dump the resulting dataset.
        """

        # stats
        filtered = [i for i, k in enumerate(self.keep) if not k]

        print('c # of feats (totl):', len(self.keep) - 1, file=sys.stderr)
        print('c # of feats (filt):', len(filtered), file=sys.stderr)
        print('c # of feats (left):', len(self.keep) - 1 - len(filtered), file=sys.stderr)

        if filtered:
            print('c filtered: {0}'.format(', '.join([self.names[i] for i in filtered])), file=sys.stderr)

        if fp:
            print(','.join([n for n, k in zip(self.names, self.keep) if k]), file=fp)

            for s, w in zip(self.samps, self.wghts):
                samp = ','.join([v for v, k in zip(s, self.keep) if k])
                for i in xrange(w):
                    print(samp, file=fp)

    def at_exit(self):
        """
            Removing temporary RIPPERk files.
        """

        for fn in self.files:
            if os.path.exists(fn):
                os.remove(fn)

#
#==============================================================================
def parse_options():
    """
        Parses command-line options.
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'hk:ps:',
                                   ['help',
                                    'opts=',
                                    'prune',
                                    'sep='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize() + '\n')
        usage()
        sys.exit(1)

    sep = ','
    optims = 0
    prune = 0

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-k', '--opts'):
            optims = int(arg)
        elif opt in ('-p', '--prune'):
            prune = 1
        elif opt in ('-s', '--sep'):
            sep = str(arg)
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    return sep, optims, prune, args


#
#==============================================================================
def usage():
    """
        Prints help message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] file')
    print('Options:')
    print('        -h, --help')
    print('        -k, --opts=<int>      Number of optimizations')
    print('                              Available vaues: [0 .. INT_MAX] (default: 0)')
    print('        -s, --sep=<string>    Separator used in the input CSV file (default = \',\')')
    print('        -p, --prune           Use pruning')


#
#==============================================================================
if __name__ == '__main__':
    sep, opts, prune, files = parse_options()

    if files:
        with open(files[0]) as fp:
            data = Data(from_fp=fp, separator=sep)
    else:  # reading from stdin
        data = Data(from_fp=sys.stdin, separator=sep)

    # dump, run ripperk and get its result
    data.dump_to_ripperk(files)
    data.run_ripperk(opts, prune)

    # remove reduntant columns keeping only those used by RIPPERk and dump
    data.dump()
