#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## dconv.py
##
##  Created on: Dec 21, 2017
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
class Converter(object):
    """
        A class for converting a (training) data set.
    """

    def __init__(self, from_csv=None, from_corels=None, separator=None):
        """
            Constructor.
        """

        assert not from_csv or not from_corels, 'At most one format should be chosen.'

        self.names = None
        self.feats = None
        self.samps = None
        self.wghts = None

        if from_csv:
            self.from_csv(filename=from_csv, separator=separator)
        elif from_corels:
            self.from_corels(filenames=from_corels)

    def from_csv(self, filename=None, separator=','):
        """
            Get data from a CSV file.
        """

        if filename:
            # reading data set from file
            with open(filename, 'r') as fp:
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

    def from_corels(self, filenames=None):
        """
            Get data from two files in the CORELS format.
        """

        if filenames:
            with open(filenames[0], 'r') as fp:  # first, read samples
                lines = fp.readlines()

            # getting rid of the newline characters
            lines = map(lambda x: x.strip(), lines)
            lines = filter(lambda x: x != '', lines)

            # number of features
            noff = len(lines)
            self.names = [None for i in xrange(noff + 1)]

            nofs = len(lines[0].split(' ')) - 1
            self.samps = [[None for j in xrange(noff + 1)] for i in xrange(nofs)]
            self.wghts = [1 for i in xrange(nofs)]

            for i, line in enumerate(lines):
                arr = line.split(' ')
                self.names[i] = arr[0].strip('{}')
                for j, val in enumerate(arr[1:]):
                    self.samps[j][i] = val

            with open(filenames[1], 'r') as fp:  # second, read labels
                lines = fp.readlines()

            # getting rid of the newline characters
            lines = map(lambda x: x.strip().split(' '), lines)
            lines = filter(lambda x: x != '', lines)

            self.names[noff] = lines[0][0].strip('{}').split(':')[0]

            labels = []
            for line in lines:
                labels.append(line[0].strip('{}').split(':')[1])

            for j, s in enumerate(self.samps):
                for i, line in enumerate(lines):
                    if line[j + 1] == '1':
                        s[noff] = labels[i]
                        break
                else:
                    assert True, 'No value for sample {0}'.format(j + 1)

    def to_csv(self, filename=None):
        """
            Output data to a CSV file.
        """

        if filename:
            with open(filename, 'w') as fp:
                print(','.join(self.names), file=fp)

                for s, w in zip(self.samps, self.wghts):
                    for i in xrange(w):
                        print(','.join(s), file=fp)

    def to_corels(self, filenames=None):
        """
            Output data to two files in the CORELS format.
        """

        # correct alphabet (binarize)
        self.corelize_alphabet()

        label_id = -len(self.feats[-1])

        with open(filenames[0], 'w') as fp:
            for i, feat in enumerate(self.cnames[:label_id]):
                line = ['{{{0}}}'.format(feat)]

                for s, w in zip(self.csamps, self.wghts):
                    line += [s[i] for j in xrange(w)]

                print(' '.join(line), file=fp)

        with open(filenames[1], 'w') as fp:
            for i, feat in enumerate(self.cnames[label_id:]):
                line = ['{{{0}}}'.format(feat)]

                for s, w in zip(self.csamps, self.wghts):
                    line += [s[label_id + i] for j in xrange(w)]

                print(' '.join(line), file=fp)

    def corelize_alphabet(self):
        """
            Check if data is in the expected binary format and fix if not.
        """

        self.cnames = []
        self.cfeats = []
        self.csamps = []
        self.valmap = {}

        for n, f in zip(self.names, self.feats):
            vals = ['0' for j in xrange(len(f))]
            for i, v in enumerate(f):
                name = '{{{0}:{1}}}'.format(n, v)
                self.cnames.append(name)

                vals[i] = '1'
                self.valmap[(n, v)] = vals[:]
                vals[i] = '0'

        for s in self.samps:
            cs = []
            for n, v in zip(self.names, s):
                cs.extend(self.valmap[(n, v)])

            self.csamps.append(cs)


#
#==============================================================================
def parse_options():
    """
        Parses command-line options.
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'hs:',
                                   ['help',
                                    'sep='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize() + '\n')
        usage()
        sys.exit(1)

    sep = ','

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-s', '--sep'):
            sep = str(arg)
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    return sep, args


#
#==============================================================================
def usage():
    """
        Prints help message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] file')
    print('Options:')
    print('        -h, --help')
    print('        -s, --sep=<string>    Separator used in the input CSV file (default = \',\')')


#
#==============================================================================
if __name__ == '__main__':
    sep, files = parse_options()

    if len(files) == 1:  # from CSV to CORELS
        # read dataset from a CSV file
        converter = Converter(from_csv=files[0], separator=sep)

        # output filename template
        fname = os.path.splitext(files[0])[0]
        names = ['{0}.out'.format(fname), '{0}.label'.format(fname)]

        # save to CORELS files
        converter.to_corels(filenames=names)
    elif len(files) == 2:  # from CORELS to CSV
        # read dataset from a CSV file
        converter = Converter(from_corels=files)

        # output filename template
        fname = os.path.splitext(files[0])[0]

        # save to CORELS files
        converter.to_csv(filename='{0}.csv'.format(fname))
