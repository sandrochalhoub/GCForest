#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## d2bin.py
##
##  Created on: Jan 4, 2018
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
import sys

# a dirty hack (xrange does not exist in Python3)
#==============================================================================
try:
    xrange
except NameError:
    xrange = range


#
#==============================================================================
class Binarizer(object):
    """
        A class for binarizing a (training) data set.
    """

    def __init__(self, from_fp=None, separator=None, enc=1):
        """
            Constructor.
        """

        self.names = None
        self.feats = None
        self.samps = None
        self.wghts = None
        self.avmap = None

        self.enc = enc  # default encoding is unary

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
            self.samps, self.wghts = [], []

            # create weighted representation of the samples
            lines = collections.Counter(lines)

            for line in lines:
                w = lines[line]
                sample = line.strip().split(separator)

                for i, f in enumerate(sample):
                    if f:
                        self.feats[i].add(f)

                self.samps.append(sample)
                self.wghts.append(w)

    def binarize(self):
        """
            Do binarization.
        """

        if self.enc == 1:
            self.binarize1()  # unary encoding
        else:
            self.binarize2()  # binary encoding

    def binarize1(self):
        """
            Do unary encoding.
        """

        self.unames = []
        self.usamps = []
        self.valmap = {}

        for n, f in zip(self.names[:-1], self.feats[:-1]):
            if len(f) > 2:
                vals = ['0' for j in xrange(len(f))]
                for i, v in enumerate(f):
                    name = '{0}:{1}'.format(n, v)
                    self.unames.append(name)

                    vals[i] = '1'
                    self.valmap[(n, v)] = vals[:]
                    vals[i] = '0'
            else:
                self.unames.append(n)

        self.unames.append(self.names[-1])

        for s in self.samps:
            cs = []
            for n, v in zip(self.names[:-1], s[:-1]):
                if (n, v) in self.valmap:
                    cs.extend(self.valmap[(n, v)])
                else:
                    cs.append(v)

            cs.append(s[-1])
            self.usamps.append(cs)

        self.names, self.samps = self.unames, self.usamps
        self.avmap = self.valmap

    def binarize2(self):
        """
            Do binary encoding.
        """

        self.unames = []
        self.usamps = []
        self.valmap = {}

        for n, f in zip(self.names[:-1], self.feats[:-1]):
            if len(f) > 2:
                len_ = int(math.ceil(math.log(len(f), 2)))
                for i in xrange(1, len_ + 1):
                    name = '{0}:b{1}'.format(n, i)
                    self.unames.append(name)

                for i, v in enumerate(f):
                # for i, v in enumerate(sorted(f, key=lambda x: int(x))):
                    vals = ['0' for j in xrange(len_)]
                    for j, c in enumerate('{0:b}'.format(i)[::-1]):
                        vals[j] = c

                    self.valmap[(n, v)] = vals
            else:
                self.unames.append(n)

        self.unames.append(self.names[-1])

        for s in self.samps:
            cs = []
            for n, v in zip(self.names[:-1], s[:-1]):
                if (n, v) in self.valmap:
                    cs.extend(self.valmap[(n, v)])
                else:
                    cs.append(v)

            cs.append(s[-1])
            self.usamps.append(cs)

        self.names, self.samps = self.unames, self.usamps
        self.avmap = self.valmap

    def dump(self, fp=sys.stdout):
        """
            Output data to a CSV file.
        """

        if fp:
            print(','.join(self.names), file=fp)

            for s, w in zip(self.samps, self.wghts):
                for i in xrange(w):
                    print(','.join(s), file=fp)

        if self.avmap:
            for key in sorted(self.avmap.keys()):
                print('{0}:{1},{2}'.format(key[0], key[1], ''.join(self.avmap[key])), file=sys.stderr)


#
#==============================================================================
def parse_options():
    """
        Parses command-line options.
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'e:hs:',
                                   ['enc=',
                                    'help',
                                    'sep='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize() + '\n')
        usage()
        sys.exit(1)

    enc = 1
    sep = ','

    for opt, arg in opts:
        if opt in ('-e', '--enc'):
            if str(arg) == 'binary':
                enc = 2
        elif opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-s', '--sep'):
            sep = str(arg)
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    return enc, sep, args


#
#==============================================================================
def usage():
    """
        Prints help message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] file')
    print('Options:')
    print('        -e, --enc=<string>    Separator used in the input CSV file')
    print('                              Available values: binary, unary (default: unary)')
    print('        -h, --help')
    print('        -s, --sep=<string>    Separator used in the input CSV file (default = \',\')')


#
#==============================================================================
if __name__ == '__main__':
    enc, sep, files = parse_options()

    if files:
        with open(files[0]) as fp:
            tool = Binarizer(from_fp=fp, separator=sep, enc=enc)
    else:  # reading from stdin
        tool = Binarizer(from_fp=sys.stdin, separator=sep, enc=enc)

    tool.binarize()
    tool.dump()
