#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## dsposit.py
##
##  Created on: Feb 22, 2018
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

#
#==============================================================================
from __future__ import print_function
import collections
import getopt
import itertools
import os
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
            self.samps, self.wghts, self.classes = [], [], {}

            for line, w in collections.Counter(lines).iteritems():
                sample = line.strip().split(separator)

                for i, f in enumerate(sample):
                    if f:
                        self.feats[i].add(f)

                self.samps.append(sample)
                self.wghts.append(w)

                # clasterizing the samples
                if sample[-1] not in self.classes:
                    self.classes[sample[-1]] = []

                self.classes[sample[-1]].append(len(self.samps) - 1)

    def compute_domains(self, unary=False):
        """
            Compute domains of each feature.
            Unarized features are mapped back to the original values.
        """

        self.domns = {}

        if unary:
            for name, feats in zip(self.names, self.feats):
                if ':' in name and len(feats) == 2:
                    # this is likely to be a binarized feature

                    # one more check; skip if not Boolean
                    if sorted(list(feats)) != ['0', '1']:
                        self.domns[name] = feats
                        continue

                    name, val = name.split(':')
                    if name not in self.domns:
                        self.domns[name] = set([])

                    self.domns[name].add(val)
                else:
                    self.domns[name] = feats
        else:
            for name, feats in zip(self.names[:-1], self.feats[:-1]):
                self.domns[name] = feats

    def dump_sample(self, sid, fp=sys.stdout):
        """
            Dump the resulting dataset to a file.
        """

        if fp:
            sample = ','.join(self.samps[sid])

            for i in xrange(self.wghts[sid]):
                print(sample, file=fp)


#
#==============================================================================
class DSet(object):
    """
        Class for storing a resulting decision set.
    """

    def __init__(self, from_fp=None):
        """
            Constructor.
        """

        self.rules = []
        self.default = None

        if from_fp:
            self.parse(from_fp)
        else:
            assert 0, 'No decision set file given'

    def parse(self, fp):
        """
            Parsing procedure.
        """

        lines = fp.readlines()
        lines = filter(lambda x: 'cover:' in x, lines)
        lines = map(lambda x: x.split(':', 1)[1].strip(), lines)

        for line in lines:
            body, label = line.split(' => ')
            label = label.strip('\'').split(': ')

            if body == 'true':
                self.default = label
                continue

            rule = []

            for l in body.split(', '):
                if l[0] == '\'':
                    name, val = l.strip('\'').rsplit(': ', 1)
                    lnew = tuple([name, val, True])
                elif l[0] == 'n':  # negative literal
                    name, val = l[4:].strip('\'').rsplit(': ', 1)
                    lnew = tuple([name, val, False])

                rule.append(lnew)
            rule.append(tuple(label + [True]))

            self.rules.append(rule)

    def posit(self, data, unary, verb):
        """
            Try to get a smaller representation by replacing a conjunction
            of negative literals with a disjunction of positive literals.
        """

        if unary:
            nof_ruls, nof_lits = 0, 0

            for rule in self.rules:
                negvals, posvals = {}, []

                for l in rule[:-1]:
                    if l[2] == False and ':' in l[0]:
                        name, val = l[0].split(':')

                        if name not in negvals:
                            negvals[name] = set([])

                        negvals[name].add(val)
                    else:
                        posvals.append('\'{0}: {1}\''.format(l[0], l[1]))

                # decide whether we split or not
                to_split = []
                to_keep = []
                for name, nvals in negvals.iteritems():
                    if len(nvals) > len(data.domns[name].difference(nvals)):
                        negvals[name] = data.domns[name].difference(nvals)
                        to_split.append(['\'{0}: {1}\''.format(name, v) for v in negvals[name]])
                    else:
                        to_keep.append(['not \'{0}: {1}\''.format(name, v) for v in negvals[name]])

                for pr in itertools.product(*to_split):
                    pr = list(pr)
                    for vals in to_keep:
                        pr.extend(vals)

                    print('c* cover: {0} => {1}: {2}'.format(', '.join(pr), rule[-1][0], rule[-1][1]))

                    nof_ruls += 1
                    nof_lits += len(pr)

            print('c* cover size:', nof_ruls)
            print('c* cover wght:', nof_lits)
        else:
            nof_ruls, nof_lits = 0, 0

            for rule in self.rules:
                negvals, posvals = {}, []

                for l in rule[:-1]:
                    if l[2] == False:
                        name, val = l[0], l[1]

                        if name not in negvals:
                            negvals[name] = set([])

                        negvals[name].add(val)
                    else:
                        posvals.append('\'{0}: {1}\''.format(l[0], l[1]))

                # decide whether we split on negative values or not
                to_split, to_keep = [], []
                for name, nvals in negvals.iteritems():
                    if len(nvals) > len(data.domns[name].difference(nvals)):
                        negvals[name] = data.domns[name].difference(nvals)
                        to_split.append(['\'{0}: {1}\''.format(name, v) for v in negvals[name]])
                    else:
                        to_keep.append(['not \'{0}: {1}\''.format(name, v) for v in negvals[name]])

                # producing new rules
                for pr in itertools.product(*to_split):
                    pr = list(pr)
                    for vals in to_keep:
                        pr.extend(vals)

                    pr.extend(posvals)

                    print('c* cover: {0} => {1}: {2}'.format(', '.join(pr), rule[-1][0], rule[-1][1]))

                    nof_ruls += 1
                    nof_lits += len(pr)

            print('c* cover size:', nof_ruls)
            print('c* cover wght:', nof_lits)


#
#==============================================================================
def parse_options():
    """
        Parses command-line options.
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'd:hs:uv',
                                   ['dset=',
                                    'help',
                                    'sep=',
                                    'unary',
                                    'verbose'])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize() + '\n')
        usage()
        sys.exit(1)

    dset = None
    sep = ','
    unary = False
    verb = False

    for opt, arg in opts:
        if opt in ('-d', '--dset'):
            dset = str(arg)
        elif opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-s', '--sep'):
            sep = str(arg)
        elif opt in ('-u', '--unary'):
            unary = True
        elif opt in ('-v', '--verbose'):
            verb = True
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    return dset, sep, unary, verb, args


#
#==============================================================================
def usage():
    """
        Prints help message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] file')
    print('Options:')
    print('        -d, --dset=<string>    File where an optimal decision set is stored (default = none)')
    print('        -h, --help')
    print('        -s, --sep=<string>     Separator used in the input CSV file (default = \',\')')
    print('        -u, --unary            Expect a unary encoded dataset')
    print('        -v, --verbose          Be verbose')


#
#==============================================================================
if __name__ == '__main__':
    dset, sep, unary, verb, files = parse_options()

    if files:
        with open(files[0]) as fp:
            data = Data(from_fp=fp, separator=sep)
            data.compute_domains(unary)

        if dset:
            with open(dset, 'r') as fp:
                dset = DSet(from_fp=fp)
        else:
            dset = DSet(from_fp=sys.stdin)

        dset.posit(data, unary, verb)
