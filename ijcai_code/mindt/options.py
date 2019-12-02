#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## options.py
##
##  Created on: Sep 20, 2017
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

#
#==============================================================================
from __future__ import print_function
import getopt
import math
import os
from pysat.card import EncType
import sys

#
#==============================================================================
encmap = {
    "seqc": EncType.seqcounter,
    "cardn": EncType.cardnetwrk,
    "sortn": EncType.sortnetwrk,
    "tot": EncType.totalizer,
    "mtot": EncType.mtotalizer,
    "kmtot": EncType.kmtotalizer
}


#
#==============================================================================
class Options(object):
    """
        Class for representing command-line options.
    """

    def __init__(self, command):
        """
            Constructor.
        """

        self.approach = 'pbased'
        self.approx = 0
        self.enc = 'cardn'
        self.files = None
        self.filter = 0
        self.solver = 'm22'
        self.primer = 'lbx'
        self.indprime = False
        self.mapfile = None
        self.plimit = 0
        self.nooverlap = False
        self.cdump = False
        self.pdump = False
        self.rdump = False
        self.separator = ','
        self.trim = 0
        self.use_cld = False
        self.verb = 0
        self.weighted = False
        self.nbnodes = 15
        self.iti_result_file = None
        self.accmin = 0.95
        self.scilearn = None
        if command:
            self.parse(command)

    def parse(self, command):
        """
            Parser.
        """

        self.command = command

        try:
            opts, args = getopt.getopt(command[1:],
                                    'a:de:f:hil:m:op:s:t:vw',
                                    ['approach=',
                                        'approx=',
                                        'cdump',
                                        'do-cld',
                                        'enc=',
                                        'filter=',
                                        'help',
                                        'indprime'
                                        'map-file=',
                                        'no-overlap',
                                        'pdump',
                                        'rdump',
                                        'plimit=',
                                        'primer=',
                                        'sep=',
                                        'solver=',
                                        'trim=',
                                        'verbose',
                                        'weighted',
                                        'nbnodes=',
                                        'accmin=',
                                        'scilearn=',
                                        'iti=',])
        except err:
            sys.stderr.write(str(err).capitalize())
            self.usage()
            sys.exit(1)

        for opt, arg in opts:
            if opt in ('-a', '--approach'):
                self.approach = str(arg)
            elif opt == '--approx':
                self.approx = int(arg)
            elif opt == '--cdump':
                self.cdump = True
            elif opt in ('-d', '--do-cld'):
                self.use_cld = True
            elif opt in ('-e', '--enc'):
                self.enc = str(arg)
            elif opt in ('-f', '--filter'):
                if arg == 'all':
                    arg = -1
                self.filter = int(arg)
            elif opt in ('-h', '--help'):
                self.usage()
                sys.exit(0)
            elif opt in ('-i', '--indprime'):
                self.indprime = True
                self.filter = -1  # filter all unnecessary primes
            elif opt in ('-l', '--plimit'):
                self.plimit = int(arg)
            elif opt in ('-m', '--map-file'):
                self.mapfile = str(arg)
            elif opt in ('-o', '--no-overlap'):
                self.nooverlap = True
            elif opt in ('-p', '--primer'):
                self.primer = str(arg)
            elif opt == '--pdump':
                self.pdump = True
            elif opt == '--rdump':
                self.rdump = True
            elif opt == '--sep':
                self.separator = str(arg)
            elif opt in ('-s', '--solver'):
                self.solver = str(arg)
            elif opt in ('-t', '--trim'):
                self.trim = int(arg)
            elif opt in ('-v', '--verbose'):
                self.verb += 1
            elif opt in ('-w', '--weighted'):
                self.weighted = True
            elif opt in ('--nbnodes'):
                print(arg)
                self.nbnodes = int(arg)
            elif opt in ('--accmin'):
                print(arg)
                self.accmin = float(arg)
            elif opt in ('--scilearn'):
                self.scilearn = int(arg)
            elif opt in ('--iti'):
                print(arg)
                self.iti_result_file = str(arg)

            else:
                assert False, 'Unhandled option: {0} {1}'.format(opt, arg)

        self.enc = encmap[self.enc]
        self.files = args

    def usage(self):
        """
            Print usage message.
        """

        print('Usage: ' + os.path.basename(self.command[0]) + ' [options] training-data')
        print('Options:')
        print('        -a, --approach=<string>    Approach to use')
        print('                                   Available values: mp92, pbased (default = pbased)')
        print('        --approx=<int>             Approximate set cover with at most k attempts')
        print('                                   Available values: [0 .. INT_MAX] (default = 0)')
        print('        --cdump                    Dump largest consistent subset of input samples')
        print('        -d, --do-cld               Do D clause calls')
        print('        -e, --enc=<string>         Encoding to use')
        print('                                   Available values: cardn, kmtot, mtot, sortn, tot (default = cardn)')
        print('        -f, --filter=<int>         Filter out unnecessary combinations of this size when computing primes')
        print('                                   Available values: [0 .. INT_MAX], all (default = 0)')
        print('        -h, --help')
        print('        -i, --indprime             Compute primes for each sample separately')
        print('        -l, --plimit=<int>         Compute at most this number of primes per sample')
        print('                                   Available values: [0 .. INT_MAX] (default = 0)')
        print('        -m, --map-file=<string>    Path to a file containing a mapping to original feature values. (default: none)')
        print('        -o, --no-overlap           Force no overlap on the training data')
        print('        -p, --primer=<string>      Prime implicant enumerator to use')
        print('                                   Available values: lbx, mcsls, sorted (default = lbx)')
        print('        --pdump                    Dump MaxSAT formula for enumerating primes')
        print('                                   (makes sense only when using an MCS-based primer)')
        print('        --rdump                    Dump resulting CSV table')
        print('        --sep=<string>             Field separator used in input file (default = \' \')')
        print('        -s, --solver=<string>      SAT solver to use')
        print('                                   Available values: g3, lgl, m22, mc, mgh (default = m22)')
        print('        -t, --trim=<int>           Trim unsatisfiable core at most this number of times')
        print('                                   Available values: [0 .. INT_MAX] (default = 0)')
        print('        -v, --verbose              Be more verbose')
        print('        -w, --weighted             Minimize the total number of literals')
        print('        --nbnodes                  Number of nodes in tree is fixed')
