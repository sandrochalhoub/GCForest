#! /usr/bin/env python

import subprocess
import sys
import os
import cPickle
import math

class KirtaniaException(Exception):
    pass
    
class AssertException(Exception):
    pass
    
class SegfaultException(Exception):
    pass


def compile_latex():
    cp = subprocess.Popen(['pdflatex', 'results.tex'], cwd='./tex')
    cp.wait()
    read = subprocess.Popen(['open', 'tex/results.pdf'])

class Wheel:
    step = ['|','/','-','\\']
    
    def __init__(self,period):
        self.period = period
        self.tick = 0
        self.x = 0

    def turn(self):
        if self.tick == 0 :
            sys.stdout.write(' ')
        if (self.tick % self.period) == 0 :
            sys.stdout.write('\b%s'%Wheel.step[self.x % len(Wheel.step)])
            sys.stdout.flush()
            # print('\b%s'%Wheel.step[self.x % len(Wheel.step)], end='', flush=True)
            self.x += 1
        self.tick += 1
        
    def close(self):
        sys.stdout.write('\b')
        sys.stdout.flush()
        
# default printing method 'prec' below-unit digits, if n>=0 it is the cardinality of the subset of instances used in the average 
def stprint(n, prec, val):
    if n<0:
        return '%.*f'%(prec, val)
    else:
        return '{\\tiny ${%.2f}$~} %.*f'%(n, prec, val)
        
# print large integers in human readable format        
def printlargeint(n, prec, val):
    for unit in ['','{\\bf K}','{\\bf M}','{\\bf B}','T','?','?','?','?']:
        if abs(val) < 10000:
            return "%.*f%s" % (prec, val, unit)
        val /= 1000.0
    return '?'
    
# format the curve
def plot(curve, methods, X, Y, names):
  styles = [' [a] \n', ' [b] \n', ' [c] \n', ' [d] \n', ' [e] \n', ' [f] \n', ' [g] \n', ' [h] \n', ' [i] \n', ' [j] \n', ' [k] \n', ' [l] \n', ' [m] \n', ' [n] \n', ' [o] \n']
  return '\cactus{%s}{%s}{%s}{{%s}}'%(X, Y, ', '.join([names[m].replace('_','\_') for m in methods]),','.join(['{%s%s}'%(style.join([str((1-x,y)) for x,y in curve[method]]),style) for method,style in zip(methods, styles)]))

        
def integrate(data, precision, epsilon=0, Xrange=(0,1)):
    w = Wheel(50)
    
    curve = []
    x=0
    
    Xmin,Xmax = Xrange
    
    lasty = 0
    lastx = 0
    maxx = float(sum(data))
    maxy = float(len(data))

    if maxx > 0 :
        for y in range(len(data)):
            x+=data[y]
    
            ny = float(y)/maxy
            nx = float(x)/maxx

            delta = math.sqrt((nx-lastx)*(nx-lastx) + (ny-lasty)*(ny-lasty))
    
    
            if epsilon<delta and data[y]>0 :
                
                if x>=Xmin and x<=Xmax:
                    curve.append((x,float(y)/(10.0**precision)))
                lastx = nx
                lasty = ny
            
            w.turn()
    else:
        curve = [(0,0)]
      
    return curve
    

def avg(L):
    if len(L) == 0 :
        return 0.0
    return float(sum(L))/float(len(L))
    
def gavg(L):
    if len(L) == 0 :
        return 1.0
    logt = 0.0
    for l in L:
        if l > 0:
            logt += math.log(l)
    return math.exp(logt/float(len(L)))

# utils for setup
def get_benchlist(folder, patterns=None, count=[]):
    files = []
    paths = []
    
    # print 'explore %s (%s)'%(folder, str(count))
    
    c = 0
    for stuff in os.listdir(folder):
        fullpath = os.path.join(folder, stuff)
        if os.path.isfile(fullpath) :
            bname = os.path.splitext(stuff)[0]
            if len(count) > 0 :
                files.append('%s~%s'%(bname, '~'.join([str(x) for x in count])))
            else:
                files.append(bname)
            paths.append(fullpath)
            # print 'add %s'%files[-1]
        else:             
            c += 1
            nfiles,npaths = get_benchlist(fullpath, count=count+[c])
            files.extend(nfiles)
            paths.extend(npaths)
            # print 'union with %s'%('\n'.join([str(p) for p in npaths]))
    return files,paths;
    
def print_benchlist(folders, outfile=None, patterns=None, annotations=None):
    if type(folders) is not list:
        folders = [folders]
    files,paths = [],[]
    for folder in folders:
        f,p = get_benchlist(folder, patterns) 
        files.extend(f)
        paths.extend(p)
           
    if outfile is None:
        outfile = sys.stdout
    outfile.write('%i benchmarks\n'%len(paths))
    for f,p in zip(files,paths) :
        hint = ''
        if annotations is not None:
            hint = annotations[f]
        outfile.write('%s\n%s %s\n'%(f, p, hint))
          
class Observation(object):
    
    def __init__(self, exp, parser):
        self.experiment = exp
        self.parser = parser
        
        try:
            stamp = open('results/store_stamp', 'r').read().strip()
            final = ''.join(exp.keys)+str(exp.num_runs())
            if stamp != final :
                self.parse()
            else:
                storefile = open('results/store', 'r')
                self.load(storefile)
        except:
            self.parse()
            
        print '%8i total runs\n%8i missing runs\n%8i parse errors\n%8i kirtania errors\n%8i Seg faults\n%8i Failed asserts\n%8i solve errors\n%8i time outs\n%8i memory outs\n'%(exp.num_runs(), sum([len(self.missing_runs[k]) for k in exp.keys]), sum([len(self.parse_errors[k]) for k in exp.keys]), sum([len(self.kirtania_errors[k]) for k in exp.keys]), sum([len(self.segfaults[k]) for k in exp.keys]), sum([len(self.asserts[k]) for k in exp.keys]), sum([len(self.solve_errors[k]) for k in exp.keys]), sum([len(self.timeouts[k]) for k in exp.keys]), sum([len(self.memoryouts[k]) for k in exp.keys]))
                 
        
        
    def parse(self):
  
        keys = self.experiment.keys
  
        # set of runs killed because of the time limit
        self.timeouts = {}.fromkeys(keys)
        # set of runs killed because of the memory limit
        self.memoryouts = {}.fromkeys(keys)
        # set of runs that raise an exception during parse
        self.parse_errors = {}.fromkeys(keys)
        # set of runs that raise an KirtaniaException during parse
        self.kirtania_errors = {}.fromkeys(keys)
        # set of runs that raise a seg fault
        self.segfaults = {}.fromkeys(keys)
        # set of runs that failed an assert
        self.asserts = {}.fromkeys(keys)
        # set of runs that did not produce any meaningful output
        self.solve_errors = {}.fromkeys(keys)
        # set of runs for which there is no output file (yet)
        self.missing_runs = {}.fromkeys(keys)
        # the data!!!
        self.data = {}.fromkeys(self.experiment.all_methods)
        
        for k in keys:
            self.timeouts[k] = []
            self.memoryouts[k] = []
            self.parse_errors[k] = []
            self.kirtania_errors[k] = []
            self.segfaults[k] = []
            self.asserts[k] = []
            self.missing_runs[k] = []
            self.solve_errors[k] = []

        # the keys that can be used for the run by order of priority
        self.filekey = {}.fromkeys(self.experiment.all_methods)    
        for method in self.experiment.all_methods:
            self.filekey[method] = {}.fromkeys(self.experiment.all_benchmarks)
            self.data[method] = {}.fromkeys(self.experiment.all_benchmarks)
            for benchmark in self.experiment.all_benchmarks:
                self.filekey[method][benchmark] = {}.fromkeys(self.experiment.all_seeds)
                self.data[method][benchmark] = {}
                for seed in self.experiment.all_seeds:
                    self.filekey[method][benchmark][seed] = []
                    
        
        self.keycount = {}.fromkeys(self.experiment.keys)
        for key in self.experiment.keys:
            self.keycount[key] = 0
            for method in self.experiment.methods[key]:
                for benchmark in self.experiment.benchmarks[key]:
                    for seed in self.experiment.seeds[key]:
                        self.filekey[method][benchmark][seed].append(key)
        

        resdir = '%s/results'%os.getcwd()
        
        sys.stdout.write('[parse results ')
        w = Wheel(50)
        
        for method in self.experiment.all_methods:
            for benchmark in self.experiment.all_benchmarks:
                for seed in self.experiment.all_seeds:
                    for key in reversed(self.filekey[method][benchmark][seed]):
                        
                        jobid = self.experiment.get_jobid(key, method, benchmark, seed)
                        
                        outfilename = '%s/%s_%i.out'%(resdir, key, jobid)
                        
                        try:
                            w.turn()
                            output = open(outfilename, 'r').readlines()

                            for line in reversed(output):
                                if line.startswith('slurmstepd: error: Exceeded job memory limit'):
                                    self.memoryouts[key].append(jobid)
                                    break
                                elif line.startswith('srun: Force Terminated job'):
                                    self.timeouts[key].append(jobid)
                                    break
                            try:
                                self.data[method][benchmark][seed] = self.parser[method](os.path.join(output))
                                if len(self.data[method][benchmark][seed].keys()) == 0 :
                                    self.solve_errors[key].append(jobid)
                                elif sum([len(self.data[method][benchmark][seed][st]) for st in self.data[method][benchmark][seed].keys()]) == 0 :
                                    self.solve_errors[key].append(jobid)
                            except KirtaniaException as e:
                                self.kirtania_errors[key].append(jobid)
                            except AssertException as e:
                                self.asserts[key].append(jobid)
                            except SegfaultException as e:
                                self.segfaults[key].append(jobid)
                            except:
                                self.parse_errors[key].append(jobid)

                            self.keycount[key] += 1

                            break

                        except IOError:
                            if key == self.filekey[method][benchmark][seed][0]:
                                self.missing_runs[key].append(jobid)
                        
                        # output = open(outfilename, 'r').readlines()
                        # print outfilename
                        #
                        # for line in reversed(output):
                        #     if line.startswith('slurmstepd: error: Exceeded job memory limit'):
                        #         self.memoryouts[key].append(jobid)
                        #         break
                        #     elif line.startswith('srun: Force Terminated job'):
                        #         self.timeouts[key].append(jobid)
                        #         break
                        #
                        # self.data[method][benchmark][seed] = self.parser[method](os.path.join(output))
                        #
                        # if len(self.data[method][benchmark][seed].keys()) == 0 :
                        #     self.solve_errors[key].append(jobid)
                        # elif sum([len(self.data[method][benchmark][seed][st]) for st in self.data[method][benchmark][seed].keys()]) == 0 :
                        #     self.solve_errors[key].append(jobid)
                        #
                        # self.keycount[key] += 1
                        #
                        # break

                                

        w.close()
        sys.stdout.write('%i files]\n'%(w.tick))
        
        self.save()
        
        
    def get_missin_runs(self):
        return []

        
    def save(self):

        sys.stdout.write('[save results')
        sys.stdout.flush()
        
        stamp = ''.join(self.experiment.keys)+'%i\n'%(self.experiment.num_runs()-sum([len(self.missing_runs[k]) for k in self.experiment.keys]))
        
        print 'save stamp =', stamp
        
        
        stampfile = open('results/store_stamp', 'w')
        stampfile.write(stamp)
        stampfile.close()
        
        
        
        storefile = open('results/store', 'w')
        
        print 'save data'
        cPickle.dump(self.data, storefile)
        
        print 'save time outs:', sum([len(self.timeouts[k]) for k in self.timeouts.keys()])
        cPickle.dump(self.timeouts, storefile)
        
        print 'save memory outs:', sum([len(self.memoryouts[k]) for k in self.memoryouts.keys()])
        cPickle.dump(self.memoryouts, storefile)
        
        print 'save parse errors:', sum([len(self.parse_errors[k]) for k in self.parse_errors.keys()])
        cPickle.dump(self.parse_errors, storefile)
        
        print 'save kirtania errors:', sum([len(self.kirtania_errors[k]) for k in self.kirtania_errors.keys()])
        cPickle.dump(self.kirtania_errors, storefile)
        
        print 'save segmentation faults:', sum([len(self.segfaults[k]) for k in self.segfaults.keys()])
        cPickle.dump(self.segfaults, storefile)
        
        print 'save failed asserts:', sum([len(self.asserts[k]) for k in self.asserts.keys()])
        cPickle.dump(self.asserts, storefile)
        
        print 'save solve errors', sum([len(self.solve_errors[k]) for k in self.solve_errors.keys()])
        cPickle.dump(self.solve_errors, storefile)
        
        print 'save missing runs', sum([len(self.missing_runs[k]) for k in self.missing_runs.keys()])
        cPickle.dump(self.missing_runs, storefile)
        
        storefile.close()
        
        sys.stdout.write(']\n')
        
    def load(self,storefile):
        
        sys.stdout.write('[load results')
        sys.stdout.flush()
        self.data = cPickle.load(storefile)
        self.timeouts = cPickle.load(storefile)
        self.memoryouts = cPickle.load(storefile)
        self.parse_errors = cPickle.load(storefile)
        self.kirtania_errors = cPickle.load(storefile)
        self.segfaults = cPickle.load(storefile)
        self.asserts = cPickle.load(storefile)
        self.solve_errors = cPickle.load(storefile)
        self.missing_runs = cPickle.load(storefile)
        
        sys.stdout.write(']\n')
        

    
    def get_ok_benchmarks(self, seeds=None, methods=None, verbose=False):
        benchmarks = self.experiment.all_benchmarks
        if methods is None:
            methods = self.experiment.all_methods
        if seeds is None:
            seeds = self.experiment.all_seeds
            
        ok = []
        for bench in benchmarks:
            is_ok = True
            for method in methods:
                for seed in seeds:
                    if not self.data[method][bench].has_key(seed):
                        is_ok = False
                    else:
                        firststat = self.data[method][bench][seed].keys()[0]
                        if len(self.data[method][bench][seed][firststat]) == 0:
                            is_ok = False
                    if not is_ok:
                        break
                if not is_ok:
                    break
            if is_ok:
                ok.append(bench)
            elif verbose:
                print 'problem with %s'%bench
                for method in methods:
                    if not self.data[method][bench].has_key(seed):
                        print '%s has not run on %s [%s]?'%(method, bench, self.experiment.get_outfiles(method, bench, ''))
                        print self.data[method][bench]
                    else:
                        for seed in seeds:
                            firststat = self.data[method][bench][seed].keys()[0]
                            if len(self.data[method][bench][seed][firststat]) == 0:
                                print '%s dif not report stat %s for %s?'%(method, firststat, bench)
                                print method, bench, self.experiment.get_outfiles(method, bench, seed)
                                print self.data[method][bench][seed][firststat]
                                
        return ok
        
    
    def check_closure(self, stat, seeds=None, methods=None, benchmarks=None):
        if benchmarks is None:
            benchmarks = self.experiment.all_benchmarks
        if methods is None:
            methods = self.experiment.all_methods
        if seeds is None:
            seeds = self.experiment.all_seeds
            
        for method in methods:
            
            # print method
            
            if method not in self.data.keys():
                print method, 'was not run'
                sys.exit(1)
            
            for bench in benchmarks:
                
                # print ' ', bench
                
                if bench not in self.data[method].keys():
                    print method, 'was not run on', bench
                    sys.exit(1)                
                    
                for seed in seeds:
                    
                    # print ' :', seed
                    
                    if seed not in self.data[method][bench].keys():
                        print method, 'was not run on', bench, 'with seed', seed
                        print self.data[method][bench]
                        sys.exit(1)
                        
                    else :
                        if stat not in self.data[method][bench][seed]:
                            print method, 'does not print stat', stat
                            sys.exit(1)
                        
                        elif len(self.data[method][bench][seed][stat]) == 0:
                            print method, 'did not print stat', stat, 'when run on', bench, 'with seed', seed, '(adding 0)'
                            self.data[method][bench][seed][stat].append(0)
            
        

    
    # add a new stat for every solver/benchmark: the normalized gap to the min/max
    def add_gap_to_best(self, stat, best=min, methods=None, benchmarks=None, seeds=None, precision=6):
        if benchmarks is None:
            benchmarks = self.experiment.all_benchmarks
        if methods is None:
            methods = self.experiment.all_methods
        if seeds is None:
            seeds = self.experiment.all_seeds
        worst = max
        if best is max:
            worst = min
            
        self.check_closure(stat, methods=methods, benchmarks=benchmarks, seeds=seeds)
                
        # here we know that everything is fine
        for bench in benchmarks:
            for seed in seeds:
                result = [self.data[method][bench][seed][stat][-1] for method in methods if len(self.data[method][bench][seed][stat])>0]
                best_res  = best(result)
                worst_res = worst(result)
                
                gap = (float(worst_res) - float(best_res))
                
                # for those methods which did not even printed the stat, to give them a score worse than worst_res
                if len(result) < len(methods):
                    gap += 1            
        
                if gap == 0 :
                    for method in methods:
                        self.data[method][bench][seed]['gapto%s'%stat] = [0]
                else :
                    for method in methods:
                        if len(self.data[method][bench][seed][stat])>0:
                            val = self.data[method][bench][seed][stat][-1]
                            if abs(best_res - val) >= (10.0**-precision) :
                                self.data[method][bench][seed]['gapto%s'%stat] = [float(val - best_res) / gap]
                            else :
                                self.data[method][bench][seed]['gapto%s'%stat] = [0]
                        else:
                            self.data[method][bench][seed]['gapto%s'%stat] = [1]
            
    
    def add_expression(self, stat_name, expression, methods=None, benchmarks=None, seeds=None):
        if benchmarks is None:
            benchmarks = self.experiment.all_benchmarks
        if methods is None:
            methods = self.experiment.all_methods
        if seeds is None:
            seeds = self.experiment.all_seeds
            
        # self.check_closure(stat, methods=methods, benchmarks=benchmarks, seeds=seeds)
        
        for bench in benchmarks:
            for method in methods:
                for seed in seeds:
                    
                    # print bench, method, seed, stat_name, self.data[method].has_key(bench), self.data[method][bench]
                    
                    if self.data[method][bench].has_key(seed):
                    
                        self.data[method][bench][seed][stat_name] = [expression(self.data[method][bench][seed])]
                    
    
        
    def write_large_table(self, tabname, statistics, methods=None, benchmarks=None, norms=None, bests=None, precisions=None, names=None, short=False, normalized=None, check=lambda x : True, transform=lambda x:x, pretty_print=None, labels=None):
        # Returns a latex table rows=benchmark, columns=methods
        
        # print tabname
        
        # print methods
        
        tabfile = open(tabname, 'w')

        if precisions is None :
            precisions = [2]*len(statistics)
        else :
            while len(precisions) < len(statistics) :
                precisions.append(2)
                
        if pretty_print is None :
            pretty_print = [stprint]*len(statistics)
        else :
            while len(pretty_print) < len(statistics) :
                pretty_print.append(stprint)
        
        if normalized is None :
            normalized = [False]*len(statistics)
        else :
            while len(normalized) < len(statistics) :
                normalized.append(False)
                
        if labels is None :
            labels = statistics
        elif len(labels) < len(statistics) :
            labels.extends(statistics[len(labels):])
        
        if norms is None :
            norms = [avg]*len(statistics)
        else :
            while len(norms) < len(statistics) :
                norms.append(avg)
            
        if methods is None :
            methods = [m for m in self.experiment.all_methods]
                        
        partitioned = True
        if benchmarks is None :
            partitioned = False
            benchmarks = sorted([(bench, [bench]) for bench in self.experiment.all_benchmarks])
            
        if bests is None :
            bests = [min]*len(statistics)
        worsts = []
        for f in bests:
            if f == min:
                worsts.append(max)
            else:
                worsts.append(min)
            
        if names is None :
            names = dict([(m,m) for m in methods])
            
        ttable = 'longtable'
        if short:
            ttable = 'tabular'
            
        numrow = 0
        mstats = {}.fromkeys(methods)
        mlabs = {}.fromkeys(methods)
        mnorms = {}.fromkeys(methods)
        
        lstats = []
        llabs = []
        lnorms = []
        
        # mbests = {}.fromkeys(methods)
        # mworsts = {}.fromkeys(methods)
        for m in methods:
            mstats[m] = []
            mlabs[m] = []
            mnorms[m] = []
            # mbests[m] = []
            # mworsts[m] = []
            # for st,n in zip(statistics, norms):
            for i in range(len(statistics)):
                la = labels[i]
                st = statistics[i]
                n = norms[i]
                # bs = bests[i]
                # ws = worsts[i]
                has_stat = False
                for bn,lb in benchmarks:
                    for b in lb: 
                        if self.data[m][b] is not None:
                            for s in self.data[m][b].keys():
                                if self.data[m][b][s].has_key(st):
                                    has_stat = True
                                    break
                    if has_stat:
                        mlabs[m].append(la)
                        mstats[m].append(st)
                        mnorms[m].append(n)
                        # mbests[m].append(bs)
                        # mworsts[m].append(ws)
                        break
            numrow += len(mstats[m])
            
        # print numrow
            
        # print methods
        # print mstats
        # print mstats['cdclmp']
        
            
        
        if partitioned :
            tabfile.write('\\begin{%s}{cl%s}\n\\toprule\n'%(ttable,''.join(['%s'%(''.join(['r' for s in mstats[x]])) for x in methods])))
            tabfile.write('&& %s\\\\\n'%' & '.join(['\multicolumn{%i}{c}{%s}'%(len(mstats[x]),names[x].replace('_','\_')) for x in methods]))
            r1 = 3
            for m in methods:
                r2 = r1+len(mstats[m])-1
                tabfile.write('\cmidrule(lr){%i-%i}'%(r1,r2))
                r1 = r2+1   
            # tabfile.write(' '.join(['\cmidrule(lr){%i-%i}'%(i+3, i+2+len(mstats[m])) for i in range(0, numrows, len(statistics))]))
            tabfile.write('\n&& %s \\\\\n\\midrule'%(' & '.join([' & '.join(['\multicolumn{1}{c}{%s}'%s for s in mlabs[x]]) for x in methods])))
        else :     
            tabfile.write('\\begin{%s}{c%s}\n\\toprule\n'%(ttable, ''.join(['%s'%(''.join(['r' for s in statistics])) for x in methods])))
            tabfile.write('& %s\\\\\n'%' & '.join(['\multicolumn{%i}{c}{%s}'%(len(statistics),names[x].replace('_','\_')) for x in methods]))      
            tabfile.write(' '.join(['\cmidrule(lr){%i-%i}'%(i+2, i+1+len(statistics)) for i in range(0, len(methods)*len(statistics), len(statistics))]))
            tabfile.write('\n& %s \\\\\n\\midrule'%(' & '.join([' & '.join(labels) for x in methods])))
 
 
        # print methods
 
        
        for y,ys in benchmarks :
            
            # print y
            
            if y == 'avg' :
                tabfile.write('\\hline\n')
            
            
            res = {}.fromkeys(methods)
            maxsolved = {}.fromkeys(statistics)
            bestres = {}.fromkeys(statistics)
            worstres = {}.fromkeys(statistics)
            span = {}.fromkeys(statistics)
            
            atleastone = False
            succsol = {}.fromkeys(statistics)
            for stat in statistics:
                succsol[stat] = set([])
            
            for x in methods :
                res[x] = {}.fromkeys(mstats[x])
                
                oks = [z for z in ys if self.data[x][z] is not None and len(self.data[x][z].keys())>0]
                
                # if y == 'KIDNEY' and x == 'prune':
                #     print [z for z in ys if self.data[x][z] is None or len(self.data[x][z].keys())==0]
                
                if len(oks) > 0 :
                
                    nbench = sum([len(self.data[x][z]) for z in oks])    
                    # succsol.add(x)
                    
                    for stat, norm in zip(mstats[x], mnorms[x]) :
                        
                        succsol[stat].add(x)
                        atleastone = True
                        
                        vals = [self.data[x][z][s][stat][-1] for s in self.experiment.all_seeds for z in oks if len(self.data[x][z][s][stat]) > 0]
                        if len(vals) == 0 :
                            res[x][stat] = (0,None)
                        else :                            
                            res[x][stat] = (float(len(vals))/float(nbench), norm(vals))
                            
                            
                 
            consistent = check(res)
                       
            # print res
            # print succsol
            
            # print [res[k]['lb'][1] for k in res.keys()]
                          
            if atleastone :  
                
                tabfile.write('\\texttt{%s} '%(y.replace('_','\_')))
                
                if partitioned :
                    tabfile.write('& (%i)'%len(ys))
                
                # print y, mstats[x], res[x]
                
                for stat, (best,worst) in zip(statistics, zip(bests, worsts)) :
                  
                    # print y, [res[x][stat][0] for x in succsol[stat]]
                  
                    maxsolved[stat] = max([res[x][stat][0] for x in succsol[stat]])
                    ms = [res[x][stat][1] for x in succsol[stat] if res[x][stat][0]==maxsolved[stat]]
                
                    if maxsolved[stat] > 0:
                        bestres[stat] = best(ms)
                        worstres[stat] = worst(ms)
                        span[stat] = bestres[stat] - worstres[stat]

                    
                for x in methods :
                
                    for stat, norm, prec, stnormalized, stprinter in zip(mstats[x], mnorms[x], precisions, normalized, pretty_print) :
                        
                        if x in succsol[stat] :    
                        
                            nsolved, val = res[x][stat]
                        
                            if nsolved == 0 :  
                                # print y, x, stat, res[x][stat]               
                                tabfile.write(' & -')
                            elif nsolved == len(self.experiment.all_seeds) :
                                if not consistent:
                                    tabfile.write(' & \\cellcolor{OrangeRed!30}{%.*f}'%(prec, val))
                                else:
                                    ratio = transform(val)
                                    # if stnormalized:
                                    #     if span[stat] != 0 and abs(bestres[stat] - val) >= (10.0**-prec):
                                    #         # if abs(bestres[stat] - val) < 0.001:
                                    #         #     print bestres[stat], val
                                    #         ratio = float(bestres[stat] - val) / float(span[stat])
                                    #     else:
                                    #         ratio = 0

                                    if val == bestres[stat] :
                                        tabfile.write(' & \\cellcolor{TealBlue!30}{%s}'%stprinter(-1, prec, ratio))
                                    else :   
                                        tabfile.write(' & %s'%stprinter(-1, prec, ratio))

                            else :
                                if not consistent:
                                    tabfile.write(' & \\cellcolor{OrangeRed!30}{{\\tiny ${%.2f}$~} %.*f}'%(nsolved, prec, val))
                                else:
                                    ratio = transform(val)
                                    # if stnormalized:
                                    #     if span[stat] == 0 or abs(bestres[stat] - val) < (10.0**-prec):
                                    #         ratio = 0
                                    #     else:
                                    #         ratio = float(bestres[stat] - val) / float(span[stat])
                                    
                                    if nsolved == maxsolved[stat] and val == bestres[stat] :
                                        tabfile.write(' & \\cellcolor{TealBlue!30}{{\\tiny ${%.2f}$~} %.*f}'%(nsolved, prec, ratio))
                                    else :
                                        tabfile.write(' & {\\tiny ${%.2f}$~} %.*f'%(nsolved, prec, ratio))
                                        # tabfile.write(' & %.*f'%(prec, ratio))

                        
                        else :
                            # print 'there', ys, x
                            tabfile.write(' & -')
                            # tabfile.write(' & \multicolumn{%i}{c}{}'%len(statistics))
            
                tabfile.write(' \\\\\n')
                        
        tabfile.write('\\bottomrule\n\\end{%s}\n'%ttable)
        
        
        
    def write_summary_table(self, tabname, statistics, labels=None, methods=None, benchmarks=None, norms=None, bests=None, precisions=None, names=None, short=False):
        # Returns a latex table rows=benchmark, columns=methods

        tabfile = open(tabname, 'w')

        if precisions is None :
            precisions = [[2]]*len(statistics)
            
        if norms is None :
            norms = [[avg]]*len(statistics)
            
        if methods is None :
            methods = [m for m in self.experiment.all_methods]
         
        if benchmarks is None :
            benchmarks = self.experiment.all_benchmarks
 
        if bests is None :
            bests = [min]*len(statistics)
            
        if names is None :
            names = dict([(m,m) for m in methods])
            
        if labels is None:
            labels = statistics
            
        pref = 'tabular'
        if not short:
            pref='longtable'
            
            
        tabfile.write('\\begin{%s}{cr%s}\n\\toprule\n'%(pref,''.join(['r']*sum([len(n) for n in norms]))))
        tabfile.write('\\multirow{2}{*}{method} & \\multirow{2}{*}{\#sol} & %s\\\\\n'%' & '.join(['\multicolumn{%i}{c}{%s}'%(len(norm), sta) for sta,norm in zip(labels,norms)]))
        col = 3
        for i in range(len(statistics)):
            tabfile.write('\cmidrule(lr){%i-%i}'%(col, col+len(norms[i])-1))
            col += len(norms[i])
        # tabfile.write(' '.join(['\cmidrule(lr){%i-%i}'%(i+2, i+4) for i in range(1, len(statistics)*3, 3)]))
        tabfile.write('\n && %s \\\\\n\\midrule\n'%(' & '.join([' & '.join([n.__name__ for n in norms[i]]) for i in range(len(statistics))])))
        tackled_by = {}.fromkeys(methods)
        results = {}.fromkeys(methods)
        cells = {}.fromkeys(methods)
        
        for y in methods :
            results[y] = {}.fromkeys(statistics) 
            cells[y] = {}.fromkeys(statistics) 
        
        for y in methods :
            
            tackled_by[y] = [(x, s) for x in benchmarks if self.data[y][x] is not None for s in self.data[y][x].keys()]
            
            # print y, tackled_by[y]
            
            if len(tackled_by[y]) > 0 :
                for stat, norm in zip(statistics,norms):
                    
                    # # if stat == 'time':
                    # print y, x, self.data[y][x][s]
                    # print self.data[y][x][s]['time']
                    # # print results[y]
                    
                    for x,s in tackled_by[y]:
                        if not self.data[y][x][s].has_key(stat):
                            print x, s, self.data[y][x][s]
                        
                    
                    
                    results[y][stat] = [self.data[y][x][s][stat][-1] for x,s in tackled_by[y] if len(self.data[y][x][s][stat])>0]
                    
                    # print results[y][stat]
                    
                    if len(results[y][stat]) > 0 :
                        # cells[y][stat] = (avg(results[y][stat]), min(results[y][stat]), max(results[y][stat]))
                        cells[y][stat] = [n(results[y][stat]) for n in norm]
                        
        # multipliers = [1 if best is min else -1 for best in bests]
        # #
        # # print multipliers
        #
        #
        # for y in methods:
        #     print [map(lambda x: (-x if best is max else x), cells[y][stat]) for stat,best in zip(statistics,bests)]
        #
                                
                                
        ordering = sorted([(-float(max([len(results[y][stat]) for stat in statistics]))/float(len(tackled_by[y])), [map(lambda x: (-x if best is max else x), cells[y][stat]) for stat,best in zip(statistics,bests)], y, [cells[y][stat] for stat in statistics]) for y in methods if len(tackled_by[y])>0])
        
        
        
        
        for sols, ords, y, vals in ordering :
            tabfile.write('%s & %.2f'%(names[y].replace('_','\_'), -sols))
            for val,prec in zip(vals,precisions):
                for v,p in zip(val,prec):
                    tabfile.write(' & %.*f'%(p,v))
            tabfile.write(' \\\\\n')

            
            # tabfile.write('%s & %.2f & %s \\\\\n'%(names[y].replace('_','\_'), -sols, ' & '.join([' %.*f & %.*f & %.*f'%(prec,mi,0,av,0,ma) for (mi,av,ma),prec in zip(vals,precisions)])))
            
        tabfile.write('\\bottomrule\n\\end{%s}'%pref)
        
        
    def write_clique_table(self, tabname, statistics, methods=None, benchmarks=None, norms=None, bests=None, precisions=None, names=None):
        # Returns a latex table rows=benchmark, columns=methods

        tabfile = open(tabname, 'w')

        if precisions is None :
            precisions = [2]*len(statistics)
            
        if norms is None :
            norms = [avg]*len(statistics)
            
        if methods is None :
            methods = [m for m in self.experiment.all_methods]
         
        if benchmarks is None :
            benchmarks = self.experiment.all_benchmarks
 
        if bests is None :
            bests = [min]*len(statistics)
            
        if names is None :
            names = dict([(m,m) for m in methods])
            
            
        mstats = {}.fromkeys(methods)
        for m in methods:
            mstats[m] = set([])
            for st in statistics:
                has_stat = False
                for b in benchmarks:
                    if self.data[m][b] is not None:
                        for s in self.data[m][b].keys():
                            if self.data[m][b][s].has_key(st):
                                has_stat = True
                                break
                    if has_stat:
                        mstats[m].add(st)
                        break
                
            
            
        tabfile.write('\\begin{tabular}{cr%s}\n\\toprule\n'%(''.join(['rrr' for s in statistics])))
        tabfile.write('\\multirow{2}{*}{method} & \\multirow{2}{*}{\#opt} & %s\\\\\n'%' & '.join(['\multicolumn{2}{c}{%s}'%s for s in statistics]))      
        tabfile.write(' '.join(['\cmidrule(lr){%i-%i}'%(i+2, i+3) for i in range(1, len(statistics)*2, 2)]))
        tabfile.write('\n & %s \\\\\n\\midrule\n'%(' '.join(['& avg (G) & avg (A)' for s in statistics])))
        
        tackled_by = {}.fromkeys(methods)
        solved_by = {}.fromkeys(methods)
        results = {}.fromkeys(methods)
        cells = {}.fromkeys(methods)
        opt = {}.fromkeys(methods)
        
        for y in methods :
            results[y] = {}.fromkeys(statistics) 
            cells[y] = {}.fromkeys(statistics) 
        
        for y in methods :
            
            # print y
            
            tackled_by[y] = [(x, s) for x in benchmarks if self.data[y][x] is not None for s in self.data[y][x].keys()]
            solved_by[y] = [(x, s) for x,s in tackled_by[y] if sum([len(self.data[y][x][s][st]) for st in mstats[y]])>0]
            
            
            if len(tackled_by[y]) > 0 :
                
                
                if y=='cliquer':
                    opt[y] = float(len([(x,s) for x,s in solved_by[y] if self.data[y][x][s]['opt'] == 1])) / float(len(tackled_by[y]))
                else :
                    opt[y] = float(len([(x,s) for x,s in solved_by[y] if self.data[y][x][s]['lb'][-1] == self.data[y][x][s]['ub'][-1]])) / float(len(tackled_by[y]))
                for stat in statistics:
                    if stat in mstats[y] :
                    
                        results[y][stat] = [self.data[y][x][s][stat][-1] for x,s in solved_by[y] if len(self.data[y][x][s][stat])>0]
                        if len(results[y][stat]) == len(tackled_by[y]):
                            cells[y][stat] = (gavg(results[y][stat]), avg(results[y][stat]))
                        else :
                            cells[y][stat] = (None,None)
                    else :
                        cells[y][stat] = ('-','-')
                                
        ordering = sorted([(opt[y], [cells[y][stat] for stat in statistics], y) for y in methods if len(tackled_by[y])>0], reverse=True)
        
        # print ordering
        
        for sols, vals, y in ordering :
            
            row = []
            
            # print vals
            
            for (ga,av),prec in zip(vals,precisions) :
                if av is None :
                    row.append(' \multicolumn{2}{c}{-} ')
                elif av == '-' :
                    row.append(' - & - ')
                else :
                    row.append(' %.*f & %.*f'%(prec,ga,prec,av))
            
            
            # [' %.*f & %.*f'%(prec,ga,prec,av) for (ga,av),prec in zip(vals,precisions)]
            
            tabfile.write('%s & %.3f & %s \\\\\n'%(names[y].replace('_','\_'), opt[y], ' & '.join(row)))
            
        tabfile.write('\\bottomrule\n\\end{tabular}')
     


    def write_cactus(self, cactusname, precision=0, methods=None, benchmarks=None, X='obj', Y='step', epsilon=0, minimize=True, names=None, Xlegend=None, Ylegend=None, Xrange=(0,1)):
        
        # print precision
        # print benchmarks
        
        # for b in benchmarks:
            
        
        cactusfile = open(cactusname, 'w')
        
        if methods is None :
            methods = sorted([m for m in self.experiment.all_methods])
            
        if benchmarks is None :
            benchmarks = sorted([m for m in self.experiment.all_benchmarks])
            
        if names is None :
            names = dict([(m,m) for m in methods])
            
        if Xlegend is None :
            Xlegend = X
            
        if Ylegend is None :
            Ylegend = Y        

        horizon = 0
        for m in self.experiment.all_methods :
            # print m
            for b in benchmarks :
                if self.data[m][b] is not None :
                    for s in self.data[m][b].keys() :
                        if len(self.data[m][b][s][Y]) > 0 :
                            h = self.data[m][b][s][Y][-1]
                            if h > horizon :
                                horizon = h
                    # print b, self.data[m][b][''][Y], horizon
      
        horizon = int(horizon * (10**precision))+1
        sys.stdout.write('[compute curves X=%s, Y=%s, [0,..,%i] '%(X,Y,horizon))
        sys.stdout.flush()
        
        curve = {}.fromkeys(methods)
        for m in methods :
            curve[m] = [0]*horizon
            
            
        numbench = {}.fromkeys(methods)
        numruns = {}.fromkeys(methods)
        
        for method in methods:
            numbench[method] = len([b for b in benchmarks if self.data[method][b] is not None])
                
        w = Wheel(50)
        for benchmark in benchmarks :
            l,u = sys.maxint, -sys.maxint
            
            # print benchmark
        
            for method in methods :
                
                # print method, self.data[method][benchmark][''][X]
                
                numruns[method] = 0
                if self.data[method][benchmark] is not None :
                    for seed in self.data[method][benchmark].keys() :
                        # print
                        # print self.experiment.get_outfiles(method, benchmark, seed),
                        numruns[method] += 1
                        if len(self.data[method][benchmark][seed][X]) > 0 :
                            # print seed, self.data[method][benchmark][seed][X][0], self.data[method][benchmark][seed][X][-1], l, u
                            if self.data[method][benchmark][seed][X][-1] < self.data[method][benchmark][seed][X][0] :
                                lx = self.data[method][benchmark][seed][X][-1]
                                ux = self.data[method][benchmark][seed][X][0]
                            else :
                                lx = self.data[method][benchmark][seed][X][0]
                                ux = self.data[method][benchmark][seed][X][-1]                               
                            
                            
                            if l > lx :
                                l = lx
                            if u < ux :
                                u = ux
                            w.turn()

                # print 'bounds = [%i...%i], %i runs, %i benchs'%(l,u,numruns[method],numbench[method])
                
            

            for method in methods :
                if self.data[method][benchmark] is not None :
                    for seed in self.data[method][benchmark].keys() :
                        lx = 0
                        for x,y in zip(self.data[method][benchmark][seed][X], self.data[method][benchmark][seed][Y]) :
                                           
                            # print x, y,
                
                            vy = int(y * (10**precision))     
                            
                            if minimize :      
                                vx = float(u - x + 1)/float(u - l + 1)
                            else :
                                vx = float(x - l + 1)/float(u - l + 1)
                                                        #
                            # print vx, '['+str(vy)+']', (vx - lx)/float(numbench[method] * numruns[method])

                            curve[method][vy] += (vx - lx)/float(numbench[method] * numruns[method])
                            lx = vx
                            
                            w.turn()
            # sys.exit(1)

        w.close()
        sys.stdout.write(']\n')
        
        
        
        for method in methods :
                        
            sys.stdout.write('[integrate %s '%method)
            curve[method] = integrate(curve[method],precision,epsilon,Xrange)
            sys.stdout.write('%.4f]\n'%(curve[method][-1][0]))
            
        cactusfile.write(plot(curve, methods, Xlegend, Ylegend, names))
        
            
        

class Experiment(object):
    
    def __init__(self, keys=None):
        
        if keys is None :
            self.keys = [os.path.splitext(f)[0] for f in os.listdir(os.getcwd()) if os.path.splitext(f)[1] == '.key']
        else :
            self.keys = keys
        self.seeds = {}.fromkeys(self.keys)
        self.methods = {}.fromkeys(self.keys)
        self.benchmarks = {}.fromkeys(self.keys)
        self.cmdline = {}
        self.path = {}
        self.annotations = {}
        
        self.all_benchmarks = set([])
        self.all_methods = set([])
        self.all_seeds = set([])
        
        self.index = {}.fromkeys(self.keys)
        self.keys_of = {}
        
        
        for k in self.keys:
            self.read_keyfile(k)
                
        for k in self.keys :
            self.init_keystruct(k)
                    
    def read_keyfile(self, k):
        
        # print 'read', k
        
        keyin = open('%s.key'%k, 'r')
        
        try :
            methodheader = keyin.readline()[:-1].split()
            self.methods[k] = []
            for i in range(int(methodheader[0])):
                x = keyin.readline()[:-1]
                v = keyin.readline()[:-1]
                self.cmdline[x] = v
                self.methods[k].append(x)
            benchmarkheader = keyin.readline()[:-1].split()
            self.benchmarks[k] = []
            for i in range(int(benchmarkheader[0])):
                x = keyin.readline()[:-1]
                v = keyin.readline()[:-1]
                annoted_benchmark = v.split()
                if len(annoted_benchmark) > 1 :
                    # print annoted_benchmark
                    self.path[x] = annoted_benchmark[0]
                    self.annotations[x] = ' '.join(annoted_benchmark[1:])
                else:
                    self.path[x] = v
                    self.annotations[x] = ''
                self.benchmarks[k].append(x)
            self.seeds[k] = keyin.readline().split()
            if len(self.seeds[k]) == 0 :
                self.seeds[k] = ['']

            self.init_keystruct(k);
                
        except :
            print('Format error in keyfile\n')
            sys.exit(0)
            
    def write_keyfile(self, k, name, methods=None, benchmarks=None):
        keyout = open('%s.key'%name, 'w')

        if methods is None:
            methods = self.methods[k]
        if benchmarks is None:
            benchmarks = self.benchmarks[k]    
            
        keyout.write('%i methods\n'%len(methods))
        for m in methods:
            keyout.write('%s\n%s\n'%(m, self.cmdline[m]))
        keyout.write('%i benchmarks\n'%len(benchmarks))
        for b in benchmarks:
            keyout.write('%s\n%s\n'%(b, self.path[b]))            
        keyout.write(' '.join(self.seeds[k]))
            
    def init_keystruct(self, k):
        for seed in self.seeds[k] :
            self.all_seeds.add(seed)
        for method in self.methods[k] :
            self.all_methods.add(method)
        for benchmark in self.benchmarks[k] :
            self.all_benchmarks.add(benchmark)
        
        self.index[k] = {}
        for i in range(len(self.methods[k])) :
            m = self.methods[k][i]
            self.index[k][m] = i
            if self.keys_of.has_key(m) :
                self.keys_of[m].add(k)
            else :
                self.keys_of[m] = set([k])
            
        for i in range(len(self.benchmarks[k])) :
            b = self.benchmarks[k][i]
            self.index[k][b] = i
            if self.keys_of.has_key(b) :
                self.keys_of[b].add(k)
            else :
                self.keys_of[b] = set([k])
                
        for i in range(len(self.seeds[k])) :
            s = self.seeds[k][i]
            self.index[k][s] = i
            if self.keys_of.has_key(s) :
                self.keys_of[s].add(k)
            else :
                self.keys_of[s] = set([k])
        
                    
    def get_outfiles(self, method, benchmark, seed):
        f = []
        for k in (self.keys_of[seed] & self.keys_of[benchmark] & self.keys_of[method]) :
            f.append(self.get_outfile(k, method, benchmark, seed))
    
        return f

    def get_outfile(self, key, method, benchmark, seed):
        return '%s_%i'%(key,self.get_jobid(key, method, benchmark, seed))

    def get_jobid(self, key, method, benchmark, seed):
        mid = self.index[key][method]
        bid = self.index[key][benchmark]
        sid = self.index[key][seed]
    
        nbench = len(self.benchmarks[key])
        nmeth = len(self.methods[key])
    
        return sid*(nbench*nmeth)+bid*(nmeth)+mid+1
    
        return '%s_%i'%(key,job_id)
            
        
    def num_runs(self) :
        # return sum([len(self.seeds[k]) * len(self.benchmarks[k]) * len(self.methods[k]) for k in self.keys])
        return sum([self.num_runs_in(k) for k in self.keys])
        
    def num_runs_in(self, k) :
        return len(self.seeds[k]) * len(self.benchmarks[k]) * len(self.methods[k])
                   
    def get_fromjob(self,k,jid):
        if self.seeds.has_key(k) :
            
            nseed = len(self.seeds[k])
            nbench = len(self.benchmarks[k])
            nmethod = len(self.methods[k])
            
            if jid <= (nseed * nbench * nmethod) :
                return self.methods[k][(jid-1) % nmethod], self.benchmarks[k][int((jid-1) % (nbench*nmethod) / nmethod)], self.seeds[k][int((jid-1) / (nbench*nmethod))]
        
        return None,None,None
        
        
    def generate_jobs(self, keys=None, benchmarks=None, methods=None, rerun=None, timeout='00:01:00', full=False) :
        
        sys.stdout.write('[generate ')
        w = Wheel(1000)
        
        if keys is None:
            keys = self.keys
            
        if rerun is not None:
            for k in keys:
                mset = set(self.methods[k])
                bset = set(self.benchmarks[k])
                
                kbench = None
                if benchmarks is not None :
                    kbench = [b for b in benchmarks if b in bset]
                    
                kmeth = None
                if methods is not None :
                    kmeth = [m for m in methods if m in mset]
                
                self.write_keyfile(k, rerun+k, kmeth, kbench)
                self.read_keyfile(rerun+k)
                # self.init_keystruct(rerun+k)
        
        njobs = 0
        for k in keys:
            key = k
            if rerun is not None:
                key = rerun+k
            
            job_file = open('jobs/jobs_%s'%(key), 'w')
            job_id = 0
            
            for seed in self.seeds[key] :
                for benchmark in self.benchmarks[key] :
                    for method in self.methods[key] :
                        job_id += 1
                        
                        job = self.cmdline[method].replace('#BENCHMARK',self.path[benchmark]).replace('#SEED',seed)
                        if self.annotations is not None:
                            job = job.replace('#HINTS', self.annotations[benchmark])
                            
                        job_file.write(job+'\n')
                        
                        w.turn()                    
                        
            njobs += job_id;
            job_file.close()
            slurmfile = open('slurm_%s'%key, 'w')
            if full :
                slurmfile.write("#!/bin/sh\n#SBATCH --ntasks=1 --array=1-%i --time=%s --output results/%s_%%a.out\nsrun -u `sed ${SLURM_ARRAY_TASK_ID}'q;d' jobs/jobs_%s`\n"%(job_id,timeout,key,key))
                # slurmfile.write("#!/bin/sh\n#SBATCH --ntasks=1 --nice=200 --array=1-%i --time=%s --output results/%s_%%a.out\n`sed ${SLURM_ARRAY_TASK_ID}'q;d' jobs/jobs_%s`\n"%(job_id,timeout,key,key))
            else:
                # slurmfile.write("#!/bin/sh\n#SBATCH --ntasks=1 --exclude=trencavel.10g,balibaste.10g,nestorius.10g --nice=200 --array=1-%i --time=%s --output results/%s_%%a.out\nsrun -u `sed ${SLURM_ARRAY_TASK_ID}'q;d' jobs/jobs_%s`\n"%(job_id,timeout,key,key))
                slurmfile.write("#!/bin/sh\n#SBATCH --ntasks=1 --exclude=spinoza.10g,esclarmonde.10g,giordano.10g,galilee.10g  --array=1-%i --time=%s --output results/%s_%%a.out\nsrun -u `sed ${SLURM_ARRAY_TASK_ID}'q;d' jobs/jobs_%s`\n"%(job_id,timeout,key,key))
            slurmfile.close()
        
        w.close()
        sys.stdout.write('%i jobs]\n'%njobs)
        
    def get_file_map(self, motifs) :
        for k in self.keys :
            for seed in self.seeds[k] :
                for bench in self.benchmarks[k] :
                    for method in self.methods[k] :
                        relevant = True
                        for motif in motifs :
                            if bench.find(motif) < 0 and method.find(motif) < 0 :
                                relevant = False
                                break
                        if relevant :
                            print k, seed, bench, method, ' '.join(self.get_outfiles(method, bench, seed))
        
        
if __name__ == '__main__':
    
    fname = os.path.basename(__file__)
    mname = os.path.splitext(fname)[0]
    
    if len(sys.argv) != 2 :
        print 'Calling %s directly build a new empty experiment folder\nUsage: ./%s <expname>'%(fname, fname)
        sys.exit(0)

        
    path = os.path.abspath(sys.argv[1])
    
    if os.path.exists(path) :
        print 'folder', path, 'already exists, aborting'
        sys.exit(0)
    
    exp = os.path.basename(path)
    
    print 'create', path
    mkdir = subprocess.Popen(['mkdir', path])
    mkdir.wait()
    
    print 'create', path+'/results'
    mkdir = subprocess.Popen(['mkdir', path+'/results'])
    mkdir.wait()
    
    print 'create', path+'/jobs'
    mkdir = subprocess.Popen(['mkdir', path+'/jobs'])
    mkdir.wait()
    
    print 'create', path+'/tex'
    mkdir = subprocess.Popen(['mkdir', path+'/tex'])
    mkdir.wait()
    
    print 'create %s/tex/results.tex'%(path)
    cp = subprocess.Popen(['cp', 'examples/results.tex', path+'/tex/'])
    cp.wait()
    
    print 'create %s/parse.py'%(path)
    cp = subprocess.Popen(['cp', 'examples/parse.py', path+'/'])
    cp.wait()
    
    print 'create %s/tex/setup.py'%(path)
    cp = subprocess.Popen(['cp', 'examples/setup.py', path+'/'])
    cp.wait()
    
    print 'create %s/%s'%(path,fname)
    cp = subprocess.Popen(['cp', fname, path+'/'])
    cp.wait()
    
    
    
