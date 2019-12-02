#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## dsimp.py
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
import os
import sys


#
#==============================================================================
class Simplifier(object):
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

            for line, w in collections.Counter(lines).iteritems():
                sample = line.strip().split(separator)

                for i, f in enumerate(sample):
                    if f:
                        self.feats[i].add(f)

                self.samps.append(sample)
                self.wghts.append(w)

        self.res = [True for n in self.names]

    def do(self, nodm=False, noeq=False):
        """
            Do simplifications.
        """

        if not nodm:
            self.filter_dm()

        if not noeq:
            self.filter_eq()

    def filter_dm(self):
        """
            Filter out 'dominated' features.
        """

        for i in xrange(len(self.res) - 1):
            if not self.res[i]:
                continue

            for j in xrange(i + 1, len(self.res) - 1):
                if not self.res[j]:
                    continue

                status_i, status_j = True, True
                amap = [{}, {}]
                omap = [{}, {}]

                for s in self.samps:
                    if status_i:
                        if s[-1] not in amap[0]:
                            amap[0][s[-1]] = s[i]

                        if s[i] not in omap[0]:
                            omap[0][s[i]] = s[-1]

                        status_i = not (s[i] != amap[0][s[-1]] or s[-1] != omap[0][s[i]])

                    if status_j == True:
                        if s[-1] not in amap[1]:
                            amap[1][s[-1]] = s[j]

                        if s[j] not in omap[1]:
                            omap[1][s[j]] = s[-1]

                        status_j = not (s[j] != amap[1][s[-1]] or s[-1] != omap[1][s[j]])

                    if not status_i and not status_j:
                        break

                if status_i and not status_j:
                    self.res[j] = False
                elif not status_i and status_j:
                    self.res[i] = False

    def filter_eq(self):
        """
            Filter out 'equal' and 'opposite' features.
        """

        for i in xrange(len(self.res) - 1):
            if not self.res[i]:
                continue

            for j in xrange(i + 1, len(self.res) - 1):
                if not self.res[j]:
                    continue

                # alphabet mapping
                amap = {}

                if len(self.feats[i]) != len(self.feats[j]):
                    continue

                for s in self.samps:
                    if s[i] not in amap:
                        amap[s[i]] = s[j]

                    if amap[s[i]] != s[j]:
                        break
                else:
                    self.res[j] = False

    def dump(self, fp=sys.stdout):
        """
            Dump the resulting dataset to a file.
        """

        # stats
        filtered = [i for i, r in enumerate(self.res) if not r]

        print('c # of feats (totl):', len(self.res) - 1, file=sys.stderr)
        print('c # of feats (filt):', len(filtered), file=sys.stderr)
        print('c # of feats (left):', len(self.res) - 1 - len(filtered), file=sys.stderr)

        if filtered:
            print('c filtered: {0}'.format(', '.join([self.names[i] for i in filtered])), file=sys.stderr)

        if fp:
            print(','.join([n for n, r in zip(self.names, self.res) if r]), file=fp)

            for s, w in zip(self.samps, self.wghts):
                samp = ','.join([v for v, r in zip(s, self.res) if r])
                for i in xrange(w):
                    print(samp, file=fp)


#
#==============================================================================
def parse_options():
    """
        Parses command-line options.
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'dehos:',
                                   ['no-dm',
                                    'no-eq',
                                    'help',
                                    'no-op',
                                    'sep='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize() + '\n')
        usage()
        sys.exit(1)

    sep = ','
    nodm = False
    noeq = False

    for opt, arg in opts:
        if opt in ('-d', '--no-dm'):
            nodm = True
        elif opt in ('-e', '--no-eq'):
            noeq = True
        elif opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-s', '--sep'):
            sep = str(arg)
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    return sep, nodm, noeq, args


#
#==============================================================================
def usage():
    """
        Prints help message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] file')
    print('Options:')
    print('        -d, --no-dm           Do no check for \'dominated\' features')
    print('        -e, --no-eq           Do no check for \'equal\' and \'opposite\' features')
    print('        -h, --help')
    print('        -s, --sep=<string>    Separator used in the input CSV file (default = \',\')')


#
#==============================================================================
if __name__ == '__main__':
    sep, nodm, noeq, files = parse_options()

    if files:
        with open(files[0]) as fp:
            simp = Simplifier(from_fp=fp, separator=sep)
    else:  # reading from stdin
        simp = Simplifier(from_fp=sys.stdin, separator=sep)

    simp.do(nodm=nodm, noeq=noeq)
    simp.dump()
