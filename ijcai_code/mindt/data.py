#!/usr/bin/env python
#-*- coding:utf-8 -*-
##
## data.py
##
##  Created on: Sep 20, 2017
##      Author: Alexey S. Ignatiev
##      E-mail: aignatiev@ciencias.ulisboa.pt
##

#
#==============================================================================
from __future__ import print_function
import collections
import itertools
import os
from  utils import *


#
#==============================================================================
class Data(object):
    """
        Class for representing data (transactions).
    """

    def __init__(self, filename=None, fpointer=None, mapfile=None, separator=' '):
        """
            Constructor and parser.
        """

        self.names = None
        self.nm2id = None
        self.samps = None
        self.wghts = None
        self.feats = None
        self.fvmap = None
        self.ovmap = {}
        self.fvars = None
        self.fname = filename
        self.mname = mapfile
        self.deleted = set([])

        if filename:
            with open(filename, 'r') as fp:
                self.parse(fp, separator)
        elif fpointer:
            self.parse(fpointer, separator)

        if self.mname:
            self.read_orig_values()

    def parse(self, fp, separator):
        """
            Parse input file.
        """

        # reading data set from file
        lines = fp.readlines()

        # reading preamble
        self.names = lines[0].strip().split(separator)
        self.feats = [set([]) for n in self.names]
        del(lines[0])

        # filling name to id mapping
        self.nm2id = {name: i for i, name in enumerate(self.names)}
        print(self.nm2id)

        self.nonbin2bin = {}
        for name in self.nm2id:
            spl = name.rsplit(':',1)
            if (spl[0] not in self.nonbin2bin):
                self.nonbin2bin[spl[0]] = [name]
            else:
                self.nonbin2bin[spl[0]].append(name)
        #print(self.nonbin2bin)
        
        # reading training samples
        self.samps, self.wghts = [], []

        print(collections.Counter(lines))
        for line, w in collections.Counter(lines).items():
            sample = line.strip().split(separator)
            #print(sample)
            for i, f in enumerate(sample):
                if f:
                    self.feats[i].add(f)
            self.samps.append(sample)
            self.wghts.append(w)


        # direct and opposite mappings for items
        idpool = itertools.count(start=1)
        FVMap = collections.namedtuple('FVMap', ['dir', 'opp'])
        self.fvmap = FVMap(dir={}, opp={})

        # mapping features to ids
        for i in range(len(self.names) - 1):
            feats = sorted(list(self.feats[i]), reverse=True)
            #print(feats)
            if len(feats) > 2:
                for l in feats:
                    self.fvmap.dir[(self.names[i], l)] = l
            else:
                #print(idpool)
                var = next(idpool)
                self.fvmap.dir[(self.names[i], feats[0])] = 1
                if len(feats) == 2:
                    self.fvmap.dir[(self.names[i], feats[1])] = 0


        # all labels are marked with distinct ids
        for l in sorted(self.feats[-1], reverse=True):
            self.fvmap.dir[(self.names[-1], l)] = next(idpool)

        # opposite mapping
        for key, val in self.fvmap.dir.items():
            self.fvmap.opp[val] = key

        # encoding samples
        for i in range(len(self.samps)):
            #print(self.samps[i])
            self.samps[i] = [self.fvmap.dir[(self.names[j], self.samps[i][j])] for j in range(len(self.samps[i])) if self.samps[i][j]]
            #print(self.samps[i])

        # determining feature variables (excluding class variables)
        for v, pair in self.fvmap.opp.items():
            if pair[0] == self.names[-1]:
                self.fvars = v - 1
                break
        print(self.fvmap)
    def read_orig_values(self):
        """
            Read original values for all the features.
            (from a separate CSV file)
        """

        self.ovmap = {}

        for line in open(self.mname, 'r'):
            featval, bits = line.strip().split(',')
            feat, val = featval.split(':')

            for i, b in enumerate(bits):
                f = '{0}:b{1}'.format(feat, i + 1)
                v = self.fvmap.dir[(f, '1')]

                if v not in self.ovmap:
                    self.ovmap[v] = [feat]

                if -v not in self.ovmap:
                    self.ovmap[-v] = [feat]

                self.ovmap[v if b == '1' else -v].append(val)

    def dump_result(self, primes, covers):
        """
            Save result to a CSV file.
        """

        fname = '{0}-result.csv'.format(os.path.splitext(self.fname)[0])

        for f in self.feats:
            if len(f) > 2:
                print('c2 non-binary features detected; not dumping the result')
                return

        with open(fname, 'w') as fp:
            print(','.join(self.names), file=fp)

            for cid in covers:
                for pid in covers[cid]:
                    feats = ['' for n in xrange(len(self.names))]

                    for l in primes[cid][pid - 1]:
                        name, val = self.fvmap.opp[l]
                        feats[self.nm2id[name]] = val

                    # label
                    name, val = self.fvmap.opp[cid]
                    feats[self.nm2id[name]] = val

                    print(','.join(feats), file=fp)


class PrepData(object):
    """
        Class for representing data (transactions).
    """

    def __init__(self, origdata, filename=None, separator=' '):
        """
            Constructor and parser.
        """

        self.origdata = origdata
        self.names = []
        self.nm2id = None
        self.samps = None
        self.wghts = None
        self.feats = None
        self.fvmap = None
        self.ovmap = {}
        self.fvars = None
        self.fname = filename

        self.deleted = set([])

        if filename:
            with open(filename, 'r') as fp:
                self.parse(fp, separator)
        else:
            print("No prep file is given")
            assert(False)

    def parse(self, fp, separator):
        """
            Parse input file.
        """

        # reading data set from file
        lines = fp.readlines()

        # reading preamble
        prep_names = lines[0].strip().split(separator)
        print(self.origdata.names)
        for p in prep_names[:-1]:
            self.names.append(self.origdata.names[int(p)])
        self.names.append(self.origdata.names[-1])
        print(self.names)    
        
        self.feats = [set([]) for n in self.names]
        del(lines[0])

        # filling name to id mapping
        self.nm2id = {name: i for i, name in enumerate(self.names)}
        

        self.nonbin2bin = {}
        for name in self.nm2id:
            spl = name.rsplit(':',1)
            if (spl[0] not in self.nonbin2bin):
                self.nonbin2bin[spl[0]] = [name]
            else:
                self.nonbin2bin[spl[0]].append(name)
        print(self.nonbin2bin)
        
        # reading training samples
        self.samps, self.wghts = [], []

        print(collections.Counter(lines))
        for line, w in collections.Counter(lines).items():
            sample = line.strip().split(separator)
            print(sample)
            for i, f in enumerate(sample):
                if not("*" in f):
                    sample[i] = int(sample[i])
                else:
                    if (i < len(sample) - 1):
                        sample[i] = DONOTCARE
                if f and  sample[i] != DONOTCARE:
                    self.feats[i].add(sample[i])                        

            self.samps.append(sample)
            self.wghts.append(w)
        print("samples",  self.samps, self.feats)

        # direct and opposite mappings for items
        idpool = itertools.count(start=1)
        FVMap = collections.namedtuple('FVMap', ['dir', 'opp'])
        self.fvmap = FVMap(dir={}, opp={})

        # mapping features to ids
        for i in range(len(self.names) - 1):
            feats = sorted(list(self.feats[i]), reverse=True)
            print(feats)
            if len(feats) > 2:
                for l in feats:
                    self.fvmap.dir[(self.names[i], l)] = l
            else:
                #print(idpool)
                var = next(idpool)
                self.fvmap.dir[(self.names[i], feats[0])] = 1
                if len(feats) == 2:
                    self.fvmap.dir[(self.names[i], feats[1])] = 0


        # all labels are marked with distinct ids
        for l in sorted(self.feats[-1], reverse=True):
            self.fvmap.dir[(self.names[-1], l)] = next(idpool)

        # opposite mapping
        for key, val in self.fvmap.dir.items():
            self.fvmap.opp[val] = key

        # encoding samples
        for i in range(len(self.samps)):
            #print(self.samps[i])
            for j in range(len(self.samps[i])):
                print("looking --->", self.samps[i])
                if self.samps[i][j] !=  DONOTCARE:
                    print(self.names[j], self.samps[i][j],  self.fvmap.dir[(self.names[j], self.samps[i][j])])
                    self.samps[i][j] = self.fvmap.dir[(self.names[j], self.samps[i][j])]
                print("~~~~~~~", self.samps[i])

        # determining feature variables (excluding class variables)
        for v, pair in self.fvmap.opp.items():
            if pair[0] == self.names[-1]:
                self.fvars = v - 1
                break
        print(self.fvmap)