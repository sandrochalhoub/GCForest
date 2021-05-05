#! /usr/bin/env python

from rocknrun import *

import pickle

import copy

# import string


# themurtree = 'murtree_cart'
themurtree = 'murtree_infogain'
# themurtree = 'murtree_full'
# themurtree = 'murtree_default'
thebfs = 'dt_default'


def timeprinter(n, prec, val):
    if val >= 3600:
        return '$\\mathsmaller{\geq}1$h'
    elif val >= 10:
        prec = 0
    elif val >= 1:
        prec = 1
    return '%.*f'%(prec, val)

def prodprinter(n, prec, val):
    if val >= 10:
        prec = 0
    if n<0:
        return '%s%.*f'%('$\\mathsmaller{\\times}$', prec, val)
    else:
        return '{\\tiny ${%.2f}$~} %s%.*f'%(n, '$\\mathsmaller{\\times}$', prec, val)
        
def incprinter(n, prec, val):
    if abs(val) >= 10:
        prec = 0
    elif abs(val) >= 1:
        prec = 1
    if n<0:
        return '%s%.*f'%('$\\mathsmaller{+}$' if val>0 else '', prec, val)
    else:
        return '{\\tiny ${%.2f}$~} %s%.*f'%(n, '$\\mathsmaller{+}$' if val>0 else '', prec, val)
        
        
def percentprinter(n, prec, val):
    if val >= 10:
        prec = 0
    if val >= 1:
        prec = 1
    if n<0:
        return '%s%.*f\\%%'%('$\\mathsmaller{+}$' if val>0 else '', prec, val)
    else:
        return '{\\tiny ${%.2f}$~} %s%.*f\\%%'%(n, '$\\mathsmaller{+}$' if val>0 else '', prec, val)


def longtablestring(title, fname, sep=6, font='normalsize'):
    return "{\\%s\n\\tabcolsep=%ipt\n\\begin{center}\n\\captionof{figure}{%s}\n\\input{%s.tex}\n\\addtocounter{table}{-1}\n\\end{center}\n\\clearpage\n}\n"%(font,sep,title,fname)

def tabularstring(title, fname, sep=6, font='normalsize'):
	return "\\begin{table}[htbp]\n\\begin{center}\n\\begin{%s}\n\\tabcolsep=%ipt\n\\input{%s.tex}\n\\end{%s}\n\\end{center}\n\\caption{\\label{tab:%s} %s}\n\\end{table}"%(font,sep,fname,font,fname,title)

def sidewaystablestring(title, fname, sep=6, font='normalsize'):
	return "\\begin{sidewaystable}[htbp]\n\\begin{center}\n\\begin{%s}\n\\tabcolsep=%ipt\n\\input{%s.tex}\n\\end{%s}\n\\end{center}\n\\caption{\\label{tab:%s} %s}\n\\end{sidewaystable}"%(font,sep,fname,font,fname,title)
     
def prettyint(val):
    if val < 1000:
        return '%s'%val
    else:
        log10 = 0
        val = float(val)
        while val >= 10.0:
            val /= 10.0
            log10 += 1

        if val*10 == int(val)*10:
            return '\\expnumber{%i}{%i}'%(int(val),log10)
        else:
            return '\\expnumber{%.1f}{%i}'%(val,log10)
    

class ITIParser(object):
    
    def __call__(self,output):
        data = {}
        error = 0
        depth = 0
        for l in output:
            
            # print l,
            
            if l.startswith('Leaves:'):
                row = l[:-1].split(',')
                for dt in row:
                    if dt.find(": ") >= 0 :
                        p = dt.strip().split(': ')
                        r = p[1].find('(iti-fast)')
                        if r >=0 :
                            p[1] = p[1][:r-1]
                        r = p[1].find('(dmti-by-leaves)')
                        if r >=0 :
                            p[1] = p[1][:r-1]
                        data[p[0]] = [float(p[1])]
            elif len(data) > 0:
            
                lp1 = l.find('(');
                lp2 = l.rfind('(');
                rp1 = l.find(')');
                rp2 = l.rfind(')');
            
                d = (lp1-2)/3
            
                if d > depth :
                    depth = d
            
                # print (lp1-2)/3
            
                if(lp1 != lp2):
                    pos = int(l[lp1+1:rp1])
                    neg = int(l[lp2+1:rp2])
                    # print pos, neg
                    error += min(pos,neg)

        data['error'] = [error]
        data['depth'] = [depth]
        
        if len(data['Leaves']) > 0 :
            data['size'] = [2 * data['Leaves'][-1] - 1]
        
        
        if len(data['cpu']) > 0 :
            data['time'] = [data['cpu'][-1]]
        
        if len(data['instances']) > 0 :
            data['accuracy'] = [(data['instances'][-1] - data['error'][-1]) / data['instances'][-1]]
        
        # print data
    
        return data
        

class IParser(object):
    
    def __init__(self):
        pass
    
    def __call__(self,output):
        res = {}        
        for line in output:
            if line.startswith('iti-tree'):
                row = line.split()
                res['size'] = [int(row[2])]
                res['time'] = [float(row[4])]
                break
        return res
        
        
class CplexParser(object):
    
    def __call__(self,output):

        stats = ['time', 'conflicts', 'lb', 'error', 'optimal', 'prooftime', 'solution']
        res = {}.fromkeys(stats)
        for stat in stats:
            res[stat] = []
            
        res['solution'].append(0)
        res['optimal'].append(0)
        res['prooftime'].append(3600)
            
        stat_set = set(stats)
            
        in_search = False    
            
        # numsol = 0
        t = 0
        conf = 0
        lb = 0
        ub = sys.maxint
        
        pt = t
        pconf = conf
        plb = lb
        pub = ub
        ok = False
        for line in output:
            
            # print line
            
            if not ok:
                if not line.startswith('num_vars'):
                    continue
                else:
                    res['num_vars'] = [int(line.split()[1])]
                    ok = True
            
            if line.startswith('num_rows'):
                res['num_rows'] = [int(line.split()[1])]
                
            elif line.startswith('Solution status'):
                if line.split()[5] == 'MIP_optimal':
                    res['optimal'] = [1]
                    res['prooftime'] = [res['time'][-1]]

            elif line.startswith('Presolve time'):
                
                t += float(line.split()[3])
            
            elif line.startswith('Found incumbent of value'):
                
                ub = int(float(line.split()[4]))
                # numsol += 1
                
            elif line.startswith('Objective value:'):
                
                ub = int(float(line.split()[2]))
                lb = ub
                
            elif line.startswith('Total (root+branch&cut)'):
                
                t = float(line.split()[3])
                
            elif line.startswith('*'):
                
                plus_index = line.find('+')
                if plus_index >= 0:
                
                    rdata = line[plus_index+1:].split()
                
                    offset = 2 if line.find('integral') >= 0 else 0
                
                    try:
                        u = float(rdata[1+offset])
                    except:
                        pass
                
                    try:
                        l = float(rdata[2+offset])
                    except:
                        pass
                
                    try:
                        c = int(line[1:plus_index].strip())
                    except:
                        pass
                
                    if c < conf or u < l or u > ub or l < lb:
                        raise ERROR
                
                    lb = l
                    ub = u
                    conf = c
                    
                else:
                    
                    rdata = line[1:].split()
                
                    offset = 2 if line.find('integral') >= 0 else 0
                    
                    try:
                        u = float(rdata[2+offset])
                    except:
                        pass
                
                    try:
                        l = float(rdata[3+offset])
                    except:
                        pass
                
                    try:
                        c = int(line[0].strip())
                    except:
                        pass
                        
                    # print c, l, u
                
                    if c < conf or u < l or u > ub or l < lb:
                        raise ERROR
                
                    lb = l
                    ub = u
                    conf = c
            
            elif line.startswith('Elapsed time ='):
                t = float(line.split()[3])
                
                # n = int(line.split()[13][:-1])
                # if n != numsol:
                #     raise ERROR
                    
            elif line.startswith('   Node  Left     Objective  IInf  Best Integer    Best Bound    ItCnt     Gap'):
                in_search = True
                    
            elif line.startswith('slurm'):
                break
                
            elif in_search and line.startswith('  Real time             =') or line.startswith('Repeating presolve.'):
                in_search = False
 
            elif in_search:
                data = line.split()
                if len(data) == 8:
                    l = int(float(data[5]))
                    if l < lb:
                        raise ERROR
                    lb = l
                    
                    if data[0].endswith('+'):
                        c = int(data[0][:-1])
                    else:
                        c = int(data[0])
                    if c < conf:
                        raise ERROR
                    conf = c
                    
            
            if plb != lb or pub != ub: #or pconf != conf or pt != t:
                
                # print res
                
                res['lb'].append(lb)
                res['error'].append(ub)
                res['conflicts'].append(conf)
                res['time'].append(t)
                pt = t
                pconf = conf
                plb = lb
                pub = ub
 
 
        # if res['lb'][-1] == res['error'][-1]:
        #     res['optimal'] = [1]
        #     res['prooftime'] = [res['time'][-1]]
        # # print res
 
        if len(res['error']) > 0  and res['error'][-1] != sys.maxint:
            res['solution'] = [1]
 
        return res
        
  
class DL8Parser(object):
         
    def __call__(self,output):
        
        res = {}
        res['optimal'] = [0]
        # res['prooftime'] = [3600]
        # res['prooftime'] = []
        
        
        for line in output:
            
            # print line,
            
            if line.startswith("Size:"):
                res['size'] = [int(line.split()[1])]
                
            elif line.startswith("Depth:"):
                res['depth'] = [int(line.split()[1])]
            
            elif line.startswith("Error:"):
                res['error'] = [float(line.split()[1])]
                
            elif line.startswith("Accuracy:"):
                res['accuracy'] = [float(line.split()[1])] 
                
            elif line.startswith("LatticeSize:"):
                res['choices'] = [int(line.split()[1])] 
                
            elif line.startswith("Runtime:"):
                res['time'] = [float(line.split()[1])] 
                res['prooftime'] = res['time']
                
            elif line.startswith("Timeout:"):
                if line.find('False') >= 0:
                    res['optimal'] = [1]
                    
        # print res

        return res
        
class CPParser(object):
         
    def __call__(self,output):
        
        res = {}
        res['optimal'] = [0]
        res['prooftime'] = [3600]
        
        
        for line in output:
            
            if line.startswith("Cost:"):
                res['error'] = [int(line.split()[1])]
                
            elif line.startswith("Search time:"):
                res['time'] = [float(line.split()[2])/1000] 
                
            elif line.startswith("Completed:"):
                if line.find("true") >= 0:
                    res['optimal'] = [1]
                    res['prooftime'] = res['time']

        return res
        
        
class CARTParser(object):
         
    def __call__(self,output):
        
        res = {}
        
        for line in output:
                
            if line.startswith("depth"):
                res['depth'] = [int(line.split()[1])]
            
            elif line.startswith("error"):
                res['error'] = [float(line.split()[1])]
                
            elif line.startswith("accuracy"):
                res['accuracy'] = [float(line.split()[1])] 
                
            elif line.startswith("time"):
                res['time'] = [float(line.split()[1])] 

        return res


class DTParser(object):
    def __init__(self, separator=' ', equal='=', dataflag='d'):
        self.equal = equal
        self.separator = separator
        self.dataflag = dataflag
        
    def store(self, stat, vstr, res):
        stat = stat.strip()
        if not res.has_key(stat):
            res[stat] = []
        val = None
        try:
            val = int(vstr)
        except:
            val = float(vstr)
            
        # if stat == 'error' and len(res['error'])>0 and val == res['error'][-1]:
        #     res['optimal'][-1] = 1
        res[stat].append(val)            
    
    def __call__(self,output):
        
        res = {}
        res['optimal'] = [0]
        
        for line in output:
            
            if line.find('optimal') >= 0:
                self.store('optimal',1,res)
                  
            if line.find('done!') >= 0:
                self.store('optimal',1,res)
                continue
                
            # break
            if line.find('wallclock time') >= 0:
                self.store('prooftime',str(res['time'][-1]),res)
                
            if not line.startswith(self.dataflag):
                continue
                
            data = line[len(self.dataflag):].split()
            for st in data:
                stat,val = st.split(self.equal)
                self.store(stat,val,res)
                
                if res['optimal'][-1] == 0:
                    if res.has_key('error'):
                        if len(res['error'])>1:
                            if res['error'][-1] == res['error'][-2]:
                                continue
                    self.store('sol'+stat,val,res)
                else:
                    self.store('proof'+stat,val,res)
                

        # if res['optimal'][-1] == 0:
        #     res['prooftime'] = [3600]
        return res




    

def quotient(x,y):
    if len(x) == 0 or len(y)==0:
        return 0.0
    elif y[-1]==0:
        return 10.0*float(x[-1])
    else:
        return float(x[-1])/float(y[-1])
        
def stop_iter():
    """An easy way to break out of a generator"""
    raise StopIteration

def common_start(sa, sb):
    return ''.join(a if a == b else stop_iter() for a, b in zip(sa, sb))



where = 'paper'



def catch_errors(e,o):
    for elist, etype in zip([o.parse_errors, o.solve_errors, o.kirtania_errors, o.segfaults, o.timeouts, o.asserts, o.memoryouts],['parse', 'solve', 'kirtania', 'segfaults', 'timeouts', 'asserts', 'memoryouts']):  
        for k in e.keys:
            if len(elist[k]):
                print
                print etype
            
                # subprocess.Popen(['rm', '-rf', etype])
                # subprocess.Popen(['mkdir', etype])
            
                kfile = open('slurm_%s'%k, 'r')
                rfile = open('rerun_%s_%s'%(etype,k), 'w')
                ifile = open('list_%s_%s'%(etype,k), 'w')
        
                for jobid in elist[k] :
                    alg, bench, seed = e.get_fromjob(k, jobid)
                    outfile = 'results/%s.out'%e.get_outfile( k, alg, bench, seed )
                
                    # subprocess.Popen(['cp', outfile, outfile.replace('results', etype)])
                    # print o.data[alg][bench]
                    ifile.write('%s, %s, %s, %s\n'%(k, jobid, outfile, e.cmdline[alg].replace('#BENCHNAME', bench).replace('#BENCHMARK', e.path[bench]).replace('#HINTS', e.annotations[bench])))
        
                print len(elist[k]), '/', e.num_runs_in(k), 'see list_%s_%s'%(etype,k)
                rfile.write(kfile.read().replace('1-%i'%e.num_runs_in(k), ','.join([str(jid) for jid in sorted(elist[k])])).replace('--mem=10000','--mem=50000').replace('--mem=3500','--mem=50000'))
                rfile.close()
                kfile.close()
                ifile.close()



if __name__ == '__main__':
    
    
    # # e = Experiment(['binoct3'])
    # # # parsers = {}
    # # # for s in e.all_methods:
    # # #     parsers[s] = CplexParser()
    # # #
    # # # o = Observation(e, parsers)
    # # #
    # # # print o.data
    # # #
    # # # sys.exit(1)
    # #
    # #
    # p = ITIParser();
    #
    # outp = open('results/itiexp_76.out', 'r').readlines()
    #
    # # print outp
    #
    # res = p(outp)
    #
    # print res
    #
    # sys.exit(1)
    # #
    
    
    # e_iti = Experiment(['itiexp']);
    #
    #
    # iti_solvers = sorted([s for s in e_iti.all_methods])
    #
    # # print iti_solvers
    #
    parsers = {}
    # for s in iti_solvers:
    #     parsers[s] = ITIParser()
    #
    #
    # o_iti = Observation(e_iti, parsers)
    #
    # catch_errors(e_iti, o_iti);
    
    # print o_iti.data['iti_default']
    #
    
    # print e_iti.all_seeds
    #
    # of = e_iti.get_outfiles('iti_default', 'surgical-deepnet-un', '1')
    #
    # print of
    #
    # sys.exit(1)
    
    
    depths = [3,4,5,7,10]
    e = {}.fromkeys(depths)
    o = {}.fromkeys(depths)    
    for d in depths:
        ks = None
        if len(sys.argv) > 1 :
            ks = [k+str(d) for k in sys.argv[1:]]
        e[d] = Experiment(ks)
        
    
    
        

    all_solvers = [s for s in e[3].all_methods]
    solvers = sorted(all_solvers)
    # optimization_solvers = [thebfs, themurtree, 'dl8.5']
    
    


    num_examples = pickle.load(open("num_examples.info", 'r'))
    num_features = pickle.load(open("num_features.info", 'r'))
    
    # for k in num_examples.keys():
    #     if k.endswith('-un'):
    #         num_examples[k[:-3]] = num_examples[k]
    #         num_features[k[:-3]] = num_features[k]
    #     if k.endswith('-bin'):
    #         num_examples[k[:-4]] = num_examples[k]
    #         num_features[k[:-4]] = num_features[k]
    #
    #
    # print num_examples
    # print num_features
    #
    # pickle.dump(num_examples, open("num_examples.info", 'w'))
    # pickle.dump(num_features, open("num_features.info", 'w'))
    #
    # sys.exit(1)


    for s in solvers:
        if s.startswith('dl'):
            parsers[s] = DL8Parser()
        elif s.startswith('cart'):
            parsers[s] = CARTParser()
        elif s.startswith('cp'):
            parsers[s] = CPParser()
        elif s.startswith('binoct'):
            parsers[s] = CplexParser()
        else: #s.startswith('dt') or s.startswith('gr') or s.startswith('probe'):
            parsers[s] = DTParser()


    for d in depths:
        o[d] = Observation(e[d], parsers)      
        catch_errors(e[d], o[d]);
        
        
        # print o[d].data['binoct']
        #
        # sys.exit(1)
        
        for k in e[d].keys:
            

            
            memr = []
            for jobid in o[d].memoryouts[k] :
                alg, bench, seed = e[d].get_fromjob(k, jobid)
                outfilename = 'results/%s.out'%e[d].get_outfile( k, alg, bench, seed )
                # print outfilename
                
                outfile = open(outfilename, 'r')
                for line in outfile:
                    if line.find('exceeded memory limit') >= 0:
                        i = line.find('>')+2
                        j = line.find(')')
                    
                        # print line,
                        mlimit = int(line[i:j])
                        
                        if mlimit < 51200000:
                            memr.append(jobid)
                            # print outfilename
                            
            if len(memr) > 0:
                rfile = open('rerun_%s_%s'%('xmem',k), 'w')
                kfile = open('slurm_%s'%k, 'r')
                rfile.write(kfile.read().replace('1-%i'%e[d].num_runs_in(k), ','.join([str(jid) for jid in sorted(memr)])).replace('--mem=10000','--mem=50000').replace('--mem=3500','--mem=50000'))
                rfile.close()
                kfile.close()
            

                # sys.exit(1)
                # 
        
        
        # num_examples = {}
        # num_features = {}
        # for b in e[3].all_benchmarks:
        #     num_examples[b] = o[3].data[thebfs][b]['1']['count'][-1]
        #     num_features[b] = o[3].data[thebfs][b]['1']['features'][-1]
        #
        # print num_examples
        # print num_features
        #
        #
        # pickle.dump(num_examples, open("num_examples.info", 'w'))
        # pickle.dump(num_features, open("num_features.info", 'w'))
        #
        # sys.exit(1)
        

                
        def time_increase_mul(x,y,z,L):
            if L['optimal'][-1] == 1:
                
                # print x,y,z,L
                # print o[d].data['dt'][y]
                
                if o[d].data[thebfs][y][z]['optimal'][-1] != 1:
                    print 'ODD'
                else:
                    tdl8 = L['time'][-1]
                    if tdl8 < .1:
                        tdl8=.1
                    
                    tdt = o[d].data[thebfs][y][z]['time'][-1]
                    if tdt < .1:
                        tdt = .1
                    # if tdt < .01:
                    #     tdt = .01
                    # return (.01 + tdl8 - tdt) / (.01 + tdt)
                    return (tdl8) / (tdt)
            return None
            
        def time_increase_add(x,y,z,L):
            if L['optimal'][-1] == 1:
                if o[d].data[thebfs][y][z]['optimal'][-1] != 1:
                    print 'ODD'
                else:
                    tdl8 = L['time'][-1]
                    tdt = o[d].data[thebfs][y][z]['time'][-1]
                    return (tdl8) - (tdt)
            return None
            
        def error_increase(x,y,z,L):
            if L.has_key('error') and len(L['error']) > 0:
                tdl8 = L['error'][-1]
                tdt = o[d].data[thebfs][y][z]['error'][-1]
                # return (1 + tdl8 - tdt) / (tdt + 1)
                # return (1 + tdl8 - tdt) * 100 / (tdt + 1)
                return tdl8 - tdt
            return None
            
        def accuracy_decrease(x,y,z,L):
            if L.has_key('accuracy') and len(L['accuracy']) > 0:
                tdl8 = L['accuracy'][-1]
                tdt = o[d].data[thebfs][y][z]['accuracy'][-1]
                return (tdl8 - tdt) * 100 / tdt
            return None
            
            
        # print solvers
        #
        # for b in e[d].all_benchmarks:
        #     print b
        #     for s in solvers:
        #         print s if o[d].data[s].has_key(b) else 'not-%s'%s,
        #     print
        #
        #
        # print o[d].data[themurtree]['Statlog_shuttle-bin']
        
        
        
        # sys.exit(1)
            

        o[d].add_formula('accuracy', lambda x,y,z,L: None if not L.has_key('error') else [float(num_examples[y]-e)/float(num_examples[y]) for e in L['error']], methods = [themurtree, 'cp', 'binoct'])
        # o[d].add_expression('timeincrease', time_increase , methods = ['dl8.5',themurtree, 'cp', 'binoct'])#, 'dt_minerror', 'dt_nopreprocessing', 'dt_nopreprocessingatall', 'dt_nolb']) #, 'binoct'])
        o[d].add_expression('timeincrease', time_increase_add , methods = ['dl8.5',themurtree, 'cp', 'binoct', 'dt_minerror', 'dt_nopreprocessing', 'dt_nopreprocessingatall', 'dt_nolb']) #, 'binoct'])
        o[d].add_expression('errorincrease', error_increase , methods = ['dl8.5', 'cp', themurtree, 'binoct'])
        o[d].add_expression('accuracydecrease', accuracy_decrease , methods = ['dl8.5', 'cp', themurtree, 'binoct'])
        
        o[d].add_expression('solution', lambda x,y,z,L: 1 if L.has_key('error') else 0, methods = ['dl8.5', 'cp'])
        
        
        t = 2
        o[d].add_expression('error%i'%t, lambda x,y,z,L: min([L['error'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        o[d].add_expression('accuracy%i'%t, lambda x,y,z,L: max([L['accuracy'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])

    
        t = 3
        o[d].add_expression('error%i'%t, lambda x,y,z,L: min([L['error'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        o[d].add_expression('accuracy%i'%t, lambda x,y,z,L: max([L['accuracy'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        

        t = 10
        o[d].add_expression('error%i'%t, lambda x,y,z,L: min([L['error'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        o[d].add_expression('accuracy%i'%t, lambda x,y,z,L: max([L['accuracy'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        
    
        t = 60
        o[d].add_expression('error%i'%t, lambda x,y,z,L: min([L['error'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        o[d].add_expression('accuracy%i'%t, lambda x,y,z,L: max([L['accuracy'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        
    
        t = 300
        o[d].add_expression('error%i'%t, lambda x,y,z,L: min([L['error'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        o[d].add_expression('accuracy%i'%t, lambda x,y,z,L: max([L['accuracy'][i] for i in range(len(L['time'])) if L['time'][i] <= t]) if len(L['time'])>0 else None, methods=[thebfs, themurtree])
        
        
        # for s in [thebfs, themurtree, 'cart']:
        #     print s
        #     for b in e[d].all_benchmarks:
        #         if not o[d].data[s][b]['1'].has_key('error'):
        #             print b,
        #     print
            
        
        o[d].add_expression('firsterror', lambda x,y,z,L: L['error'][0] if L.has_key('error') else num_examples[y], methods=[thebfs, themurtree, 'cart'])
        o[d].add_expression('firstaccuracy', lambda x,y,z,L: L['accuracy'][0] if L.has_key('accuracy') else num_examples[y], methods=[thebfs, themurtree, 'cart'])
        
        
        o[d].add_expression('firsttime', lambda x,y,z,L: L['time'][0] if L.has_key('time') else 3600, methods=[thebfs, themurtree, 'cart'])
        


    

    
    t = 2
    error2 = Statistic('error%i'%t, label='$\\leq$3s', best=min, norms=[avg], precision=lambda x:0)
    accuracy2 = Statistic('accuracy%i'%t, label='$\\leq$3s', best=min, norms=[avg], precision=lambda x:0)
    t = 3
    error3 = Statistic('error%i'%t, label='$\\leq$3s', best=min, norms=[avg], precision=lambda x:0)
    accuracy3 = Statistic('accuracy%i'%t, label='$\\leq$3s', best=min, norms=[avg], precision=lambda x:0)
    t = 10
    error10 = Statistic('error%i'%t, label='$\\leq$10s', best=min, norms=[avg], precision=lambda x:0)
    accuracy10 = Statistic('accuracy%i'%t, label='$\\leq$10s', best=min, norms=[avg], precision=lambda x:0)
    t = 60
    error60 = Statistic('error%i'%t, label='$\\leq$1m', best=min, norms=[avg], precision=lambda x:0)
    accuracy60 = Statistic('accuracy%i'%t, label='$\\leq$1m', best=min, norms=[avg], precision=lambda x:0)
    t = 300
    error300 = Statistic('error%i'%t, label='$\\leq$5m', best=min, norms=[avg], precision=lambda x:0)
    accuracy300 = Statistic('accuracy%i'%t, label='$\\leq$5m', best=min, norms=[avg], precision=lambda x:0)

    firsterror = Statistic('firsterror', label='first', best=min, norms=[avg], precision=lambda x:0)
    firstaccuracy = Statistic('firstaccuracy', label='first', best=min, norms=[avg], precision=lambda x:0)
    firsttime = Statistic('firsttime', label='cpu', best=min, norms=[avg], precision=lambda x:2)
    
    sorted_files = sorted([(e[3].path[b],b) for b in e[3].all_benchmarks if b != 'anneal-un' and b != 'zoo-1'])
    
    duplicates_and_dummies = set(['Ionosphere-bin', 'letter_recognition-bin', 'tic-tac-toe-bin', 'wine-bin', 'zoo-1', 'anneal-un', 'mnist_1', 'mnist_2', 'mnist_3', 'mnist_4', 'mnist_5', 'mnist_6', 'mnist_7', 'mnist_8', 'mnist_9'])
    
    
    sorted_benchs = sorted([b for b in e[3].all_benchmarks if b not in duplicates_and_dummies], key=str.lower)
    
    
    # print sorted_benchs
    
    
    print len(sorted_benchs)
    print '\n'.join(['%i %i %s'%(num_examples[b], num_features[b], b) for b in sorted_benchs])
    
    
    # sys.exit(1)
    
    ns = sorted([num_examples[b] for b in sorted_benchs])
    ms = sorted([num_features[b] for b in sorted_benchs])
    # print ' '.join([str(n) for n in ns])
    # print ' '.join([str(m) for m in ms])
    
    
    ncut = 1500
    mcut = 100
    ncrit = lambda x: '$\\numex \\geq %i$'%ncut if num_examples[x] >= ncut else '$\\numex < %i$'%ncut;
    mcrit = lambda x: '$\\numfeat \\geq %i$'%mcut if num_features[x] >= mcut else '$\\numfeat < %i$'%mcut;
    
    cl_benches = {}
    # for k in depths:
    # cl_benches[k] = {}
    for b in sorted_benchs:
        # key = '%s; %s'%(ncrit(b),mcrit(b))
        key = '%s'%(mcrit(b))
        if not cl_benches.has_key(key):
            cl_benches[key] = []
        cl_benches[key].append(b)
    
    sk = sorted([k for k in cl_benches.keys()])
    for k in sk:
        print k, len(cl_benches[k])
        
        
        
    def nice_label(x):
        
        if x.startswith('dt_minerror'):
            return '\\noheuristic'
        elif x.startswith('dt_nopreprocessing'):
            return '\\nopreprocessing'
        elif x.startswith('dt_nolb'):
            return '\\nolb'
        elif x.startswith('dt'):
            return '\\budalg'
        elif x.startswith('dl'):
            return '\\dleight'
        elif x.startswith('cart'):
            return '\\cart'
        elif x == themurtree:
            return '\\murtree'
        elif x == themurtree:
            return '\\binoct'
        elif x == 'cp':
            return '\\cp'
        
        return x.replace('_', ' ')

    
    solution = Statistic('solution', label='sol.', best=None, norms=[avg], precision=lambda x:2) 
    
    optimal3 = Statistic('optimal', label='opt.', best=max, norms=[avg], precision=lambda x:3)
    optimal = Statistic('optimal', label='opt.', best=max, norms=[avg], precision=lambda x:2)
    optimal0 = Statistic('optimal', label='opt.', best=None, norms=[avg], precision=lambda x:0)  

    gerror = Statistic('error', best=min, norms=[avg], precision=lambda x:1)
    error = Statistic('error', best=min, norms=[avg], precision=lambda x:0)
    errorno = Statistic('error', best=None, norms=[avg], precision=lambda x:0)
    # error = Statistic('error', best=min, norms=[avg], precision=lambda x:2)
    
    accuracy = Statistic('accuracy', label='acc.', best=max, norms=[avg], precision=lambda x:3)
    accuracy0 = Statistic('accuracy', label='acc.', best=None, norms=[avg], precision=lambda x:3)
    
    proofcpu = Statistic('prooftime', label='cpu', best=min, norms=[avg], precision=lambda x:2, printer=timeprinter)
    
    solcpu = Statistic('soltime', label='cpu (b)', best=min, norms=[avg], precision=lambda x:2)
    
    cpu = Statistic('time', label='cpu', best=min, norms=[avg], precision=lambda x:0)
    cpu1 = Statistic('time', label='cpu', best=min, norms=[avg], precision=lambda x:1)
    cpuno = Statistic('time', label='cpu', best=None, norms=[avg], precision=lambda x:0)
    cpuall = Statistic('time', label='cpu', best=None, norms=[avg], precision=lambda x:2, printer=timeprinter)
    
    # timeinc = Statistic('timeincrease', label='cpu$^*$', best=None, norms=[avg], precision=lambda x:1, printer=prodprinter)
    # timeinc2 = Statistic('timeincrease', label='cpu$^*$', best=None, norms=[avg], precision=lambda x:2, printer=prodprinter)

    # timeinc = Statistic('timeincrease', label='cpu$^*$', best=None, norms=[avg], precision=lambda x:1, printer=incprinter)
    timeinc2 = Statistic('timeincrease', label='cpu$^*$', best=None, norms=[avg], precision=lambda x:2, printer=incprinter)
    
    errorinc = Statistic('errorincrease', label='error$^*$', best=None, norms=[avg], precision=lambda x:0, printer=incprinter)

    gerrorinc = Statistic('errorincrease', label='error$^*$', best=None, norms=[avg], precision=lambda x:1, printer=incprinter)
    
    accuracyinc = Statistic('accuracydecrease', label='acc.$^*$', best=None, norms=[avg], precision=lambda x:2, printer=percentprinter)

    size = Statistic('size', best=min, norms=[avg], precision=lambda x:0)
    
    
    
    
    X = [Method(thebfs, stats=[optimal, error, proofcpu], label=nice_label(thebfs))]
    X.append(Method(themurtree, stats=[optimal, error, timeinc2], label=nice_label(themurtree)))
    X.append(Method('cp', stats=[optimal, error, timeinc2], label='\\cp'))
    X.append(Method('dl8.5', stats=[solution, optimal, errorinc, timeinc2], label='\\dleight'))
    X.append(Method('binoct', stats=[solution, errorinc], label='\\binoct'))
    
    ncol = sum([len(x.stats) for x in X])
    tname = 'summaryclasses'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    first = True
    for bclass in sk:
        if first:
            open('%s/%s.tex'%(where,tname), 'a+').write('\n&\\multicolumn{%i}{c}{%s (%i data sets)}\\\\\n\\midrule'%(ncol,bclass,len(cl_benches[bclass])))
        else:
            open('%s/%s.tex'%(where,tname), 'a+').write('\n\\midrule\n&\\multicolumn{%i}{c}{%s (%i data sets)}\\\\\n\\midrule'%(ncol,bclass,len(cl_benches[bclass])))
        first = False
        for k in depths:
            bk = Benchmark(cl_benches[bclass], label='%i'%k)
            o[k].write_line('%s/%s.tex'%(where,tname), X, bk) 
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    
    X = [Method(thebfs, stats=[optimal, gerror, proofcpu], label=nice_label(thebfs))]
    X.append(Method(themurtree, stats=[optimal, gerror, timeinc2], label=nice_label(themurtree)))
    X.append(Method('cp', stats=[optimal, gerror, timeinc2], label='\\cp'))
    X.append(Method('dl8.5', stats=[solution, optimal, gerrorinc, timeinc2], label='\\dleight'))
    X.append(Method('binoct', stats=[solution, gerrorinc], label='\\binoct'))
    
    ncol = sum([len(x.stats) for x in X])
    tname = 'summaryclassesgerror'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    first = True
    for bclass in sk:
        if first:
            open('%s/%s.tex'%(where,tname), 'a+').write('\n&\\multicolumn{%i}{c}{%s (%i data sets)}\\\\\n\\midrule'%(ncol,bclass,len(cl_benches[bclass])))
        else:
            open('%s/%s.tex'%(where,tname), 'a+').write('\n\\midrule\n&\\multicolumn{%i}{c}{%s (%i data sets)}\\\\\n\\midrule'%(ncol,bclass,len(cl_benches[bclass])))
        first = False
        for k in depths:
            bk = Benchmark(cl_benches[bclass], label='%i'%k, norm=lambda x: gavg if x == gerror or x == gerrorinc else avg)
            o[k].write_line('%s/%s.tex'%(where,tname), X, bk) 
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    
    X = [Method(thebfs, stats=[optimal, accuracy, proofcpu], label=nice_label(thebfs))]
    X.append(Method(themurtree, stats=[optimal, accuracy, timeinc2], label=nice_label(themurtree)))
    X.append(Method('cp', stats=[optimal, accuracy, timeinc2], label='\\cp'))
    X.append(Method('dl8.5', stats=[solution, optimal, accuracyinc, timeinc2], label='\\dleight'))
    X.append(Method('binoct', stats=[solution, accuracyinc], label='\\binoct'))
    
    ncol = sum([len(x.stats) for x in X])
    tname = 'summaryclassesacc'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    first = True
    for bclass in sk:
        if first:
            open('%s/%s.tex'%(where,tname), 'a+').write('\n&\\multicolumn{%i}{c}{%s (%i data sets)}\\\\\n\\midrule'%(ncol,bclass,len(cl_benches[bclass])))
        else:
            open('%s/%s.tex'%(where,tname), 'a+').write('\n\\midrule\n&\\multicolumn{%i}{c}{%s (%i data sets)}\\\\\n\\midrule'%(ncol,bclass,len(cl_benches[bclass])))
        first = False
        for k in depths:
            bk = Benchmark(cl_benches[bclass], label='%i'%k)
            o[k].write_line('%s/%s.tex'%(where,tname), X, bk) 
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    

    
    
    X = [Method(s, stats=[error, cpuall], label=nice_label(s)) for s in [thebfs, themurtree, 'dl8.5', 'cp', 'binoct', 'cart']]
    # X.append(Method('cart', stats=[error, cpuall], label='\\cart'))  
    ncol = sum([len(x.stats) for x in X])
    # tname = 'allclasses'
    # o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    # names = ['smallsmall', 'smallbig', 'bigsmall', 'bigbig']
    names = ['smallm', 'bigm']
    for k in depths:
        for name,bclass in zip(names,sk):
            tname = 'allclasses_%s_%i'%(name,k)
            o[k].write_header('%s/%s.tex'%(where,tname), X)
            for b in cl_benches[bclass]:
                startcolumns = []
                for i in range(len(X)):
                    if len(o[k].data[X[i].name][b].keys())>0:
                        seed = o[k].data[X[i].name][b].keys()[0]
                        if o[k].data[X[i].name][b][seed].has_key('optimal'):
                            if len(o[k].data[X[i].name][b][seed]['optimal'])>0:
                               if o[k].data[X[i].name][b][seed]['optimal'][-1]==1:
                                    startcolumns.append(2*i+1);
                                    
                boldcolumns = []
                errors = []
                for x in X:
                    seed = o[k].data[x.name][b].keys()[0]
                    dat = o[k].data[x.name][b][seed]
                    if not dat.has_key('error'):
                        errors.append(sys.maxint)
                    elif len(dat['error'])==0:
                        errors.append(sys.maxint)
                    else:
                        errors.append(int(dat['error'][-1]))
                    
                
                # errors = [dat['error'][-1] if o[k].data[x.name][b][seed].has_key('error') and len([o[k].data[x.name][b][o[k].data[x.name][b].keys()[0]]['error'])>0 else sys.maxint for x in X]
                minerror = min(errors)
                boldcolumns = [2*i for i in range(len(errors)) if errors[i] == minerror]
                    
                # print errors, minerror, boldcolumns,
                
                if len(boldcolumns) > 1:
                    boldcolumns = []
                    
                # print boldcolumns
                
                                    
                o[k].write_line('%s/%s.tex'%(where,tname), X, Benchmark([b], label=b.replace('-un','').replace('-bin','')), starred=set(startcolumns), bold=set(boldcolumns))
            o[k].write_footer('%s/%s.tex'%(where,tname))
            
    # X = [Method(s, stats=[accuracy, cpuall], label=nice_label(s)) for s in [thebfs, themurtree, 'dl8.5', 'cp', 'binoct', 'cart']]
    # # X.append(Method('cart', stats=[error, cpuall], label='\\cart'))
    # ncol = sum([len(x.stats) for x in X])
    # # tname = 'allclasses'
    # # o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    # # names = ['smallsmall', 'smallbig', 'bigsmall', 'bigbig']
    # names = ['smallm', 'bigm']
    # for k in depths:
    #     for name,bclass in zip(names,sk):
    #         tname = 'allclassesacc_%s_%i'%(name,k)
    #         o[k].write_header('%s/%s.tex'%(where,tname), X)
    #         for b in cl_benches[bclass]:
    #             columns = []
    #             for i in range(len(X)):
    #                 if len(o[k].data[X[i].name][b].keys())>0:
    #                     seed = o[k].data[X[i].name][b].keys()[0]
    #                     if o[k].data[X[i].name][b][seed].has_key('optimal'):
    #                         if len(o[k].data[X[i].name][b][seed]['optimal'])>0:
    #                            if o[k].data[X[i].name][b][seed]['optimal'][-1]==1:
    #                                 columns.append(2*i+1);
    #
    #             o[k].write_line('%s/%s.tex'%(where,tname), X, Benchmark([b], label=b.replace('-un','').replace('-bin','')), starred=set(columns))
    #         o[k].write_footer('%s/%s.tex'%(where,tname))
            
    X = [Method(s, stats=[error, cpuall], label=nice_label(s)) for s in [thebfs, 'dt_minerror', 'dt_nopreprocessingatall', 'dt_nolb']]
    # X.append(Method('cart', stats=[error, cpuall], label='\\cart'))  
    ncol = sum([len(x.stats) for x in X])
    # tname = 'allfactors'
    # o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    names = ['smallm', 'bigm']  
    for k in depths:
        for name,bclass in zip(names,sk):
            tname = 'allfactors_%s_%i'%(name,k)
            o[k].write_header('%s/%s.tex'%(where,tname), X)
            for b in cl_benches[bclass]:
                columns = []
                for i in range(len(X)):
                    if len(o[k].data[X[i].name][b].keys())>0:
                        seed = o[k].data[X[i].name][b].keys()[0]
                        if o[k].data[X[i].name][b][seed].has_key('optimal'):
                            if len(o[k].data[X[i].name][b][seed]['optimal'])>0:
                               if o[k].data[X[i].name][b][seed]['optimal'][-1]==1:
                                    columns.append(2*i+1);
                                    
                o[k].write_line('%s/%s.tex'%(where,tname), X, Benchmark([b], label=b.replace('-un','').replace('-bin','')), starred=set(columns))
            o[k].write_footer('%s/%s.tex'%(where,tname))
            
            
    def get(stat, x):
        return o[3].data[thebfs][x.instances[0]]['1'][stat][-1]
    infolabels = ('$|\\allex|$','$|\\features|$','$|\\allex|^*$','$|\\features|^*$','noise')
    infovalues = (lambda x : num_examples[x.label], lambda x : get('features',x), lambda x : get('final_count', x), lambda x : get('features',x) - get('feature_reduction',x), lambda x : 2.0*float(get('suppressed',x))/float(num_examples[x.label]))
    precs = [0,0,0,0,4]
    
    tname = '%s/datasetinfo.tex'%where
    tabfile = open(tname, 'w')
    tabfile.write('\\begin{tabular}{l%s}\n\\toprule\n'%('r'*len(infolabels)))
    tabfile.write('set & %s \\\\\n\\midrule\n'%(' & '.join(['%s'%(l) for l in infolabels])))
    
    benchs = [Benchmark([b], label=b.replace('-un','').replace('-bin','')) for bclass in sk for n,b in sorted([(num_examples[b], b) for b in cl_benches[bclass]])]
    # bs = sorted([(num_examples[b.label], b) for b in benchs])
    
    
    
    for b in benchs:
        # be = Benchmark([b], label=b.replace('-un',''))
        tabfile.write('\\texttt{%s}'%(b.label.replace('_','\_')))
        for v,p in zip(infovalues,precs):
            tabfile.write('& %s'%stprint(-1,p,v(b)))
        tabfile.write('\\\\\n')
    tabfile.write('\\bottomrule\n\\end{tabular}\n')
    
    
    X = [Method(s, stats=[firsttime, firsterror, error2, error10, error60, error300], label=nice_label(s)) for s in [thebfs, themurtree]]
    X.append(Method('cart', stats=[firsttime, firsterror], label='\\cart'))
    tname = 'summaryspeed'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        thebenches = Benchmark(sorted_benchs, label='%i'%k)
        o[k].write_line('%s/%s.tex'%(where,tname), X, thebenches)
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    X = [Method(s, stats=[firsttime, firstaccuracy, accuracy2, accuracy10, accuracy60, accuracy300], label=nice_label(s)) for s in [thebfs, themurtree]]
    X.append(Method('cart', stats=[firstaccuracy], label='\\cart'))
    tname = 'summaryaccspeed'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        thebenches = Benchmark(sorted_benchs, label='%i'%k)
        o[k].write_line('%s/%s.tex'%(where,tname), X, thebenches)
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    for k in depths:
        cname = 'xscerror%i'%k
        nlab = dict([(m, nice_label(m)) for m in [thebfs, themurtree]])#, 'binoct']])
        nlab['cart'] = '\\cart'
        o[k].write_simpler_cactus('%s/%s.tex'%(where,cname), X='accuracy', Y='time', methods=[thebfs, themurtree], epsilon=0, names=nlab, Xlegend='Average Accuracy', Ylegend='CPU time', options='legend pos=north west', point=['cart'])
    
    
    X = [Method(s, stats=[error, optimal, timeinc2 if s != thebfs else cpuno], label=nice_label(s)) for s in [thebfs, 'dt_minerror', 'dt_nopreprocessingatall', 'dt_nolb']]
    tname = 'factorbfs'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        thebenches = Benchmark(sorted_benchs, label='%i'%k)
        o[k].write_line('%s/%s.tex'%(where,tname), X, thebenches)
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    X = [Method(s, stats=[accuracy, optimal, timeinc2 if s != thebfs else cpuno], label=nice_label(s)) for s in [thebfs, 'dt_minerror', 'dt_nopreprocessingatall', 'dt_nolb']]
    tname = 'factorbfsacc'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        thebenches = Benchmark(sorted_benchs, label='%i'%k)
        o[k].write_line('%s/%s.tex'%(where,tname), X, thebenches)
    o[3].write_footer('%s/%s.tex'%(where,tname))
    

    
    sys.exit(1)
    
    
    # benches = [Benchmark([b], label=b.replace('-un','')) for p,b in sorted_files]
    
    bench = {}.fromkeys(depths)
    for k in depths:
        bench[k] = Benchmark([b for p,b in sorted_files], label='%i'%k)
        
        
    
    solved = {}
    for p,b in sorted_files:
        seed = o[10].data[thebfs][b].keys()[0]
        err = o[10].data[thebfs][b][seed]['error'][-1]
        if err == 0:
            solved[b] = True
        else:
            solved[b] = False
            
    pickle.dump(solved, open("solved.info", 'w'))
        
        
        
    # for p,b in sorted_files:
    #     print b,
    #     for s in iti_solvers:
    #         print s,
    #         if len(o_iti.data[s][b]) == 0 :
    #             print '- - - |',
    #         else:
    #             seed = o_iti.data[s][b].keys()[0]
    #             print o_iti.data[s][b][seed]['size'][-1], o_iti.data[s][b][seed]['depth'][-1], o_iti.data[s][b][seed]['error'][-1], '|',
    #     for k in depths:
    #         seed = o[k].data[thebfs][b].keys()[0]
    #         print o[k].data[thebfs][b][seed]['error'][-1],
    #     print
        
            

    
    
    
    benches = {}.fromkeys(['smalln','largen'])
    for typ in ['smalln','largen']:
        benches[typ] = {}.fromkeys()
        
    for k in depths:
        benches['small'][k] = Benchmark([b for p,b in sorted_files if num_examples[b] <= 10000], label='%i'%k)
        benches['large'][k] = Benchmark([b for p,b in sorted_files if num_examples[b] > 10000], label='%i'%k)
    
    
    

    
    dataset_info=(('$|\\allex|$','$|\\features|$'), (lambda x : num_examples[x.label], lambda x : num_features[x.label]))
    preprocessing_info=(('$|\\allex|$','$|\\features|$','$|\\allex|^*$','$|\\features|^*$','noise'), (lambda x : num_examples[x.label], lambda x : get('final_count', x), lambda x : get('features',x) - get('feature_reduction',x), lambda x : 2*get('suppressed',x)/num_examples[x.label]))


    def nice_label(x):
        
        if x.startswith('dt_minerror'):
            return '\\noheuristic'
        elif x.startswith('dt_nopreprocessing'):
            return '\\nopreprocessing'
        elif x.startswith('dt_nolb'):
            return '\\nolb'
        elif x.startswith('dt'):
            return '\\budalg'
        elif x.startswith('dl'):
            return '\\dleight'
        elif x.startswith('cart'):
            return '\\cart'
        elif x == themurtree:
            return '\\murtree'
        elif x == themurtree:
            return '\\binoct'
        elif x == 'cp':
            return '\\cp'
        
        return x.replace('_', ' ')

    
    solution = Statistic('solution', label='sol.', best=None, norms=[avg], precision=lambda x:2) 
    
    optimal3 = Statistic('optimal', label='opt.', best=max, norms=[avg], precision=lambda x:3)
    optimal = Statistic('optimal', label='opt.', best=max, norms=[avg], precision=lambda x:2)
    optimal0 = Statistic('optimal', label='opt.', best=None, norms=[avg], precision=lambda x:0)  

    error = Statistic('error', best=min, norms=[avg], precision=lambda x:0)
    errorno = Statistic('error', best=None, norms=[avg], precision=lambda x:0)
    # error = Statistic('error', best=min, norms=[avg], precision=lambda x:2)
    
    accuracy = Statistic('accuracy', label='acc.', best=max, norms=[avg], precision=lambda x:4)
    accuracy0 = Statistic('accuracy', label='acc.', best=None, norms=[avg], precision=lambda x:4)
    
    proofcpu = Statistic('prooftime', label='cpu (t)', best=min, norms=[avg], precision=lambda x:1)
    
    solcpu = Statistic('soltime', label='cpu (b)', best=min, norms=[avg], precision=lambda x:2)
    
    cpu = Statistic('time', label='cpu', best=min, norms=[avg], precision=lambda x:0)
    cpu1 = Statistic('time', label='cpu', best=min, norms=[avg], precision=lambda x:1)
    cpuno = Statistic('time', label='cpu', best=None, norms=[avg], precision=lambda x:0)
    cpuall = Statistic('time', label='cpu', best=None, norms=[avg], precision=lambda x:2, printer=timeprinter)
    
    timeinc = Statistic('timeincrease', label='cpu$^*$', best=None, norms=[avg], precision=lambda x:0, printer=prodprinter)
    timeinc2 = Statistic('timeincrease', label='cpu$^*$', best=None, norms=[avg], precision=lambda x:2, printer=prodprinter)
    
    errorinc = Statistic('errorincrease', label='error$^*$', best=None, norms=[avg], precision=lambda x:0, printer=incprinter)
    
    accuracyinc = Statistic('accuracydecrease', label='acc.$^*$', best=None, norms=[avg], precision=lambda x:2, printer=percentprinter)

    size = Statistic('size', best=min, norms=[avg], precision=lambda x:0)
    
    
    # tabfile = open('%s/tabfile.tex'%(where), 'a+')
    
    
    infolabels = ('$|\\allex|$','$|\\features|$','$|\\allex|^*$','$|\\features|^*$','noise')
    infovalues = (lambda x : num_examples[x.label], lambda x : get('features',x), lambda x : get('final_count', x), lambda x : get('features',x) - get('feature_reduction',x), lambda x : 2.0*float(get('suppressed',x))/float(num_examples[x.label]))
    precs = [0,0,0,0,4]
    
    tname = '%s/datasetinfo.tex'%where
    tabfile = open(tname, 'w')
    tabfile.write('\\begin{tabular}{l%s}\n\\toprule\n'%('r'*len(infolabels)))
    tabfile.write('set & %s \\\\\n\\midrule\n'%(' & '.join(['%s'%(l) for l in infolabels])))
    
    benchs = [Benchmark([b], label=b.replace('-un','')) for p,b in sorted_files]
    bs = sorted([(num_examples[b.label], b) for b in benchs])
    
    
    
    for c,b in bs:
        # be = Benchmark([b], label=b.replace('-un',''))
        tabfile.write('\\texttt{%s}'%(b.label.replace('_','\_')))
        for v,p in zip(infovalues,precs):
            tabfile.write('& %s'%stprint(-1,p,v(b)))
        tabfile.write('\\\\\n')
    tabfile.write('\\bottomrule\n\\end{tabular}\n')
            
    
      #
    # o[k].write_header('%s/%s.tex'%(where,tname), []) #, info=dataset_info)
    # for p,b in sorted_files:
    #     be = Benchmark([b], label=b.replace('-un',''))
    #     o[k].write_line('%s/%s.tex'%(where,tname), [], be, info=dataset_info)
    # o[k].write_footer('%s/%s.tex'%(where,tname))
    #
    #
    
    optimization_solvers = [thebfs, themurtree, 'dl8.5']
    
    # X = [Method(s, stats=[error, size, cpu, optimal, search], label=nice_label(s)) for s in solvers]
    # X = [Method(s, stats=[error, cpu, optimal], label=nice_label(s)) for s in [thebfs, themurtree]]
    X = [Method(thebfs, stats=[error, cpuno], label=nice_label(thebfs))]
    X.append(Method(themurtree, stats=[error, timeinc2, optimal], label=nice_label(themurtree)))
    X.append(Method('dl8.5', stats=[errorinc, timeinc2, optimal], label='\\dleight'))
    tname = 'summaryopt'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        o[k].write_line('%s/%s.tex'%(where,tname), X, bench[k])
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    # X = [Method(s, stats=[error, optimal, timeinc2 if s != thebfs else cpuno], label=nice_label(s)) for s in [thebfs, 'dt_minerror', 'dt_nopreprocessingatall', 'dt_nolb']]
    # tname = 'factorbfs'
    # o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    # for k in depths:
    #     o[k].write_line('%s/%s.tex'%(where,tname), X, bench[k])
    # o[3].write_footer('%s/%s.tex'%(where,tname))
    
    # X = [Method(s, stats=[error, optimal, cpuno], label=nice_label(s)) for s in ['murtree_default', 'murtree_cart', 'murtree_infogain', 'murtree_full']]
    # tname = 'factormur'
    # o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    # for k in depths:
    #     o[k].write_line('%s/%s.tex'%(where,tname), X, bench[k])
    # o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    os = [thebfs, themurtree, 'dl8.5', 'cp', 'binoct']
    X = [Method(s, stats=[error, cpuall], label=nice_label(s)) for s in [thebfs, themurtree, 'dl8.5', 'cp', 'binoct']]
    X.append(Method('cart', stats=[error, cpuall], label='\\cart'))
    # X.append(Method('dl8.5', stats=[error, accuracy, cpu2, optimal], label='\\dleight'))
    for k in depths:
        tname = 'allopt%i'%k
        o[k].write_header('%s/%s.tex'%(where,tname), X) #, info=dataset_info)
        for c,be in bs:
            
            b = be.instances[0]
            
            columns = []
            for i in range(len(os)):
                # print os[i], b, o[k].data[os[i]][b].keys()
                if len(o[k].data[os[i]][b].keys())>0:
                    seed = o[k].data[os[i]][b].keys()[0]
                    if o[k].data[os[i]][b][seed].has_key('optimal'):
                        if len(o[k].data[os[i]][b][seed]['optimal'])>0:
                           if o[k].data[os[i]][b][seed]['optimal'][-1]==1:
                                columns.append(2*i+1);
            
            # set([2*i+1 for i in range(len(os)) if o[k].data[os[i]][b][o[k].data[os[i]][b].keys()[0]]['optimal'][-1]==1])
            
            # be = Benchmark([b], label=b.replace('-un',''))
            o[k].write_line('%s/%s.tex'%(where,tname), X, be, starred=set(columns))#, info=dataset_info)
        o[k].write_footer('%s/%s.tex'%(where,tname))
       
    # os = [thebfs, 'dt_minerror', 'dt_nopreprocessingatall', 'dt_nolb']
    # X = [Method(s, stats=[error, cpuall], label=nice_label(s)) for s in [thebfs, 'dt_minerror', 'dt_nopreprocessingatall', 'dt_nolb']]
    # for k in depths:
    #     tname = 'allfact%i'%k
    #     o[k].write_header('%s/%s.tex'%(where,tname), X, info=dataset_info)
    #     for c,be in bs: #sorted_files:
    #
    #         # columns = set([2*i+1 for i in range(len(os)) if o[k].data[os[i]][b][o[k].data[os[i]][b].keys()[0]]['optimal'][-1]==1])
    #         b = be.instances[0]
    #
    #         columns = []
    #         for i in range(len(os)):
    #             # print os[i], b, o[k].data[os[i]][b].keys()
    #             if len(o[k].data[os[i]][b].keys())>0:
    #                 seed = o[k].data[os[i]][b].keys()[0]
    #                 if o[k].data[os[i]][b][seed].has_key('optimal'):
    #                     if len(o[k].data[os[i]][b][seed]['optimal'])>0:
    #                        if o[k].data[os[i]][b][seed]['optimal'][-1]==1:
    #                             columns.append(2*i+1);
    #
    #         # be = Benchmark([b], label=b.replace('-un',''))
    #         o[k].write_line('%s/%s.tex'%(where,tname), X, be, starred=set(columns))#, info=dataset_info)
    #     o[k ].write_footer('%s/%s.tex'%(where,tname))
        
    # X = [Method(s, stats=[error, size, optimal, cpu1], label=nice_label(s)) for s in [thebfs, themurtree]]
    # for k in depths:
    #     tname = 'allsize%i'%k
    #     o[k].write_header('%s/%s.tex'%(where,tname), X, info=dataset_info)
    #     for p,b in sorted_files:
    #         be = Benchmark([b], label=b.replace('-un',''))
    #         o[k].write_line('%s/%s.tex'%(where,tname), X, be, info=dataset_info)
    #     o[k ].write_footer('%s/%s.tex'%(where,tname))
        
        
    X = [Method(s, stats=[optimal, accuracy0, cpuno], label=nice_label(s)) for s in [thebfs]]
    X.append(Method(themurtree, stats=[optimal, accuracyinc, timeinc2], label=nice_label(themurtree)))
    X.append(Method('dl8.5', stats=[optimal, accuracyinc, timeinc2, solution], label='\\dleight'))
    X.append(Method('cp', stats=[optimal, accuracyinc, timeinc2], label='\\cp'))
    X.append(Method('binoct', stats=[optimal, accuracyinc, timeinc2], label='\\binoct'))
    tname = 'summaryaccall'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        o[k].write_line('%s/%s.tex'%(where,tname), X, bench[k])
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
        
    X = [Method(s, stats=[error, accuracy, cpuno, optimal], label=nice_label(s)) for s in [thebfs]]
    X.append(Method(themurtree, stats=[error, accuracy, timeinc2, optimal], label=nice_label(themurtree)))
    X.append(Method('dl8.5', stats=[errorinc, accuracyinc, timeinc2, solution, optimal], label='\\dleight'))
    # X.append(Method('cp', stats=[errorinc, accuracyinc, timeinc2, solution, optimal], label='\\cp'))
    tname = 'summaryacc'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        o[k].write_line('%s/%s.tex'%(where,tname), X, bench[k])
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    
    X = [Method(s, stats=[errorno, accuracy0, cpuno, optimal], label=nice_label(s)) for s in [thebfs]]
    # X.append(Method(themurtree, stats=[error, accuracy, timeinc2, optimal], label=nice_label(themurtree)))
    # X.append(Method('dl8.5', stats=[errorinc, accuracyinc, timeinc2, solution, optimal], label='\\dleight'))
    X.append(Method('cp', stats=[errorinc, accuracyinc, timeinc2, optimal], label='\\cp'))
    X.append(Method('binoct', stats=[errorinc, accuracyinc, timeinc2, optimal], label='\\binoct'))
    tname = 'summaryaccother'
    o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
    for k in depths:
        o[k].write_line('%s/%s.tex'%(where,tname), X, bench[k])
    o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    for typ in ['small', 'large']:
        X = [Method(s, stats=[optimal, accuracy0, cpuno], label=nice_label(s)) for s in [thebfs]]
        X.append(Method(themurtree, stats=[optimal, accuracyinc, timeinc2], label=nice_label(themurtree)))
        X.append(Method('dl8.5', stats=[optimal, accuracyinc, timeinc2, solution], label='\\dleight'))
        X.append(Method('cp', stats=[optimal, accuracyinc, timeinc2], label='\\cp'))
        X.append(Method('binoct', stats=[optimal, accuracyinc, timeinc2], label='\\binoct'))
        tname = 'summaryaccall%s'%typ
        o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
        for k in depths:
            o[k].write_line('%s/%s.tex'%(where,tname), X, benches[typ][k])
        o[3].write_footer('%s/%s.tex'%(where,tname))
    
        
        X = [Method(s, stats=[error, accuracy, cpuno, optimal], label=nice_label(s)) for s in [thebfs]]
        X.append(Method(themurtree, stats=[error, accuracy, timeinc2, optimal], label=nice_label(themurtree)))
        X.append(Method('dl8.5', stats=[errorinc, accuracyinc, timeinc2, solution, optimal], label='\\dleight'))
        # X.append(Method('cp', stats=[errorinc, accuracyinc, timeinc2, solution, optimal], label='\\cp'))
        tname = 'summaryacc%s'%typ
        o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
        for k in depths:
            o[k].write_line('%s/%s.tex'%(where,tname), X, benches[typ][k])
        o[3].write_footer('%s/%s.tex'%(where,tname))
    
    
    
        X = [Method(s, stats=[errorno, accuracy0, cpuno, optimal], label=nice_label(s)) for s in [thebfs]]
        # X.append(Method(themurtree, stats=[error, accuracy, timeinc2, optimal], label=nice_label(themurtree)))
        # X.append(Method('dl8.5', stats=[errorinc, accuracyinc, timeinc2, solution, optimal], label='\\dleight'))
        X.append(Method('cp', stats=[errorinc, accuracyinc, timeinc2, optimal], label='\\cp'))
        X.append(Method('binoct', stats=[errorinc, accuracyinc, timeinc2, optimal], label='\\binoct'))
        tname = 'summaryaccother%s'%typ
        o[3].write_header('%s/%s.tex'%(where,tname), X, benchlegend='$\\mdepth$')
        for k in depths:
            o[k].write_line('%s/%s.tex'%(where,tname), X, benches[typ][k])
        o[3].write_footer('%s/%s.tex'%(where,tname))
    
        # X = [Method(s, stats=[accuracy, optimal], label=nice_label(s)) for s in [thebfs, themurtree]]
        # X.append(Method('dl8.5', stats=[accuracy, accuracyinc, timeinc, optimal], label='\\dleight'))
        # for k in depths:
        #     tname = 'allacc%i'%k
        #     o[k].write_header('%s/%s.tex'%(where,tname), X)
        #     for p,b in sorted_files:
        #         be = Benchmark([b], label=b.replace('-un',''))
        #         o[k].write_line('%s/%s.tex'%(where,tname), X, be)
        #     o[k ].write_footer('%s/%s.tex'%(where,tname))
    
    
    

    

    # for k in depths:
    #     tname = 'allspeed%i'%k
    #     o[k].write_header('%s/%s.tex'%(where,tname), X)
    #     for p,b in sorted_files:
    #         be = Benchmark([b], label=b.replace('-un',''))
    #         o[k].write_line('%s/%s.tex'%(where,tname), X, be)
    #     o[k].write_footer('%s/%s.tex'%(where,tname))
        
    for k in depths:
        cname = 'xscerror%i'%k
        nlab = dict([(m, nice_label(m)) for m in [thebfs, themurtree]])#, 'binoct']])
        nlab['cart'] = '\\cart'
        o[k].write_simpler_cactus('%s/%s.tex'%(where,cname), X='accuracy', Y='time', methods=[thebfs, themurtree], epsilon=0, names=nlab, Xlegend='Average Accuracy', Ylegend='CPU time', options='legend pos=north west', point=['cart'])
    # tabfile.write('\\begin{figure}\\begin{center}\\input{%s.tex}\end{center}\\caption{Accuracy over time (max depth=%i)}\\end{figure}'%(cname,kdepth))
    # tabfile.write('\n\n')
    
    # tabfile.write(tabularstring("Comparaison: optimization of shallow trees (max depth=%i)"%kdepth, tname, sep=5, font='normalsize'))
    # tabfile.write('\n\n')
            
