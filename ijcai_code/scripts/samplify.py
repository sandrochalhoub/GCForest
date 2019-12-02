#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## samplify.py
##
##  Created on: Jan 10, 2018
##      Author: Filipe Pereira, Alexey S. Ignatiev
##      E-mail: filipe.pereira.1995@gmail.com, aignatiev@ciencias.ulisboa.pt
##

#
#==============================================================================
from __future__ import print_function
import collections
import getopt
import os
from pysat.formula import CNF
import random
import sys


#
#==============================================================================
class Sampler(object):
    """
        A class for generating samples given a DNF formula in DIMACS.
    """

    def __init__(self, from_fp):
        """
            Constructor.
        """

        self.terms = None
        self.nvars = None
        self.samps = None

        if from_fp:
            formula = CNF()
            formula.from_fp(file_pointer=from_fp)

            self.terms = formula.clauses
            self.nvars = formula.nv

    def samplify(self, nvars, nsamples, no_dupl):
        """
            Do the job.
        """

        if nvars > self.nvars:
            self.nvars = nvars

        if no_dupl == False:
            self.samps = []
        else:
            self.samps = set([])

        self.generate_function()

        random.seed()
        while len(self.samps) < nsamples:
            args = '{0:b}'.format(random.getrandbits(self.nvars))

            if len(args) < self.nvars:
                args = '0' * (self.nvars - len(args)) + args

            exec('res = evaluate({0})'.format(', '.join(args)))

            if no_dupl == False:
                self.samps.append('{0}{1}'.format(args, int(res)))
            else:
                self.samps.add('{0}{1}'.format(args, int(res)))

    def generate_function(self):
        """
            Generate an integer function for sampling.
        """

        terms = []

        for t in self.terms:
            terms.append(' and '.join(map(lambda l: 'x{0}'.format(l) if l > 0 else '(not x{0})'.format(-l), t)))

        expr = ' or '.join(['({0})'.format(t) for t in terms])
        func = 'def evaluate({0}): return {1}'.format(', '.join(['x{0}'.format(i + 1) for i in xrange(self.nvars)]), expr)

        exec(func, globals())

    def dump_samples(self, permute):
        """
            Prints samples as a CSV table into stdout.
        """

        if self.samps:
            shuffle = range(self.nvars)
            if permute:
                random.shuffle(shuffle)

            # preamble
            print(','.join(['x{0}'.format(i + 1) for i in shuffle] + ['y']))

            # samples
            for s in self.samps:
                print(','.join([s[shuffle[i]] for i in xrange(len(s) - 1)] + [s[-1]]))


#
#==============================================================================
def parse_options():
    """
        Parses command-line options.
    """

    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   'dhps:v:',
                                   ['help',
                                    'no-dupl',
                                    'permute',
                                    'samples=',
                                    'vars='])
    except getopt.GetoptError as err:
        sys.stderr.write(str(err).capitalize() + '\n')
        usage()
        sys.exit(1)

    nsamples = 10
    nvars = None
    no_dupl = False
    permute = False

    for opt, arg in opts:
        if opt in ('-d', '--no-dupl'):
            no_dupl = True
        elif opt in ('-h', '--help'):
            usage()
            sys.exit(0)
        elif opt in ('-p', '--permute'):
            permute = True
        elif opt in ('-s', '--samples'):
            nsamples = int(arg)
        elif opt in ('-v', '--vars'):
            nvars = int(arg)
        else:
            assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

    return nvars, nsamples, no_dupl, permute, args


#
#==============================================================================
def usage():
    """
        Prints help message.
    """

    print('Usage:', os.path.basename(sys.argv[0]), '[options] dimacs-file')
    print('Options:')
    print('        -d, --no-dupl          Do not allow duplicate samples')
    print('        -h, --help')
    print('        -p, --permute          Shuffle (permute) columns')
    print('        -s, --samples=<int>    Generate this number of samples')
    print('                               Available values: [1 .. INT_MAX] (default: 10)')
    print('        -v, --vars=<int>       Use at most this number of variables')
    print('                               Available values: [1 .. INT_MAX] (default: unspecified)')

#
#==============================================================================
if __name__ == '__main__':
    nvar, nsamples, no_dupl, permute, files = parse_options()

    assert no_dupl == False or nsamples <= 2 ** nvar, 'Too many samples expected'

    if files:
        with open(files[0]) as fp:
            tool = Sampler(from_fp=fp)
    else:  # reading from stdin
        tool = Sampler(from_fp=sys.stdin)

    tool.samplify(nvar, nsamples, no_dupl)
    tool.dump_samples(permute)
