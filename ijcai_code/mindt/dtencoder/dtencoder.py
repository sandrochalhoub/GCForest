#!/usr/bin/env python
# -*- coding:utf-8 -*-
# #
# # primer.py
# #
# #  Created on: Dec 4, 2017
# #      Author: Alexey S. Ignatiev
# #      E-mail: aignatiev@ciencias.ulisboa.pt
# #

#
#==============================================================================
from __future__ import print_function
import inspect, os, sys
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../pysat-module/'))
sys.path.insert(0, os.path.join(os.path.realpath(os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0])), '../hitman/py'))

from  utils import *
import collections
import itertools
from pysat.card import *
from pysat.formula import WCNF
from pysat.solvers import Solver
import resource
import socket
from pysat.card import *
from pysat.formula import WCNF
import math
import time

from pysat.solvers import Solver  # standard way to import the library
from pysat.solvers import Minisat22, Glucose3  # more direct way

#
# import matplotlib.pyplot as plt
# import networkx as nx
# from networkx.drawing.nx_pydot import write_dot
def hierarchy_pos(G, root, width=1., vert_gap=0.2, vert_loc=0, xcenter=0.5,
                  pos=None, parent=None):
    '''If there is a cycle that is reachable from root, then this will see infinite recursion.
       G: the graph
       root: the root node of current branch
       width: horizontal space allocated for this branch - avoids overlap with other branches
       vert_gap: gap between levels of hierarchy
       vert_loc: vertical location of root
       xcenter: horizontal location of root
       pos: a dict saying where all nodes go if they have been assigned
       parent: parent of this branch.'''
    if pos == None:
        pos = {root:(xcenter, vert_loc)}
    else:
        pos[root] = (xcenter, vert_loc)
    neighbors = G.neighbors(root)
    nbs = []
    for neighbor in neighbors:
        nbs.append(neighbor)

    if parent != None:  # this should be removed for directed graphs.
        if (parent in nbs):
            nbs.remove(parent)  # if directed, then parent not in neighbors.
    if len(nbs) != 0:
        dx = width / len(nbs)

        nextx = xcenter - width / 2 - dx / 2
        for neighbor in nbs:
            nextx += dx
            pos = hierarchy_pos(G, neighbor, width=dx, vert_gap=vert_gap,
                                vert_loc=vert_loc - vert_gap, xcenter=nextx, pos=pos,
                                parent=root)

    return pos



#==============================================================================
class DTEncoder(object):

    def __init__(self, data,  options):
        """
            Constructor.
        """

        self.init_stime = resource.getrusage(resource.RUSAGE_SELF).ru_utime
        self.init_ctime = resource.getrusage(resource.RUSAGE_CHILDREN).ru_utime

        # saving data
        self.data = data
        # binarizing the data properly
        outs = {}
        nb_classes = 0
        for i in range(len(self.data.samps)):
            samp_bin, out = self.data.samps[i][:-1], self.data.samps[i][-1]
            if (out not in outs):
                outs[out] = nb_classes
                nb_classes += 1
            print("samp_bin", samp_bin)
            for l in samp_bin:
                if l > 0:  
                    name, lit = self.data.fvmap.opp[l]
                    j = self.data.nm2id[name]
                    #print(l, name, lit, j, self.data.feats[j])
                    if len(self.data.feats[j]) > 2:
                        assert(False)
                        #samp_bin += [-self.data.fvmap.dir[(name, l)] for l in list(self.data.feats[j].difference(set([lit])))]
                else:
                    if (l == DONOTCARE):
                        assert(not(options.prepfile is None))
            #exit()

            self.data.samps[i] = samp_bin + [outs[out]]
            #print(self.data.samps[i])
            #exit()
            # ##print(self.data.samps[i])
        # ##print(self.data.fvmap.dir.keys())
        self.max_id = max(self.data.fvmap.dir.values()) + 1
        # ##print("max id is {}".format(self.max_id))

        if (len(outs) > 2):
            print("Non-binary classes", outs)
            exit()
        else:
            print("Ok, binaryclasses", outs)
        # saving options
        self.options = options
        self.var2ids = {}

        self.s = Solver(name='g3')
        self.s.file = open(self.options.files[0] + ".dump", "w+")
        print(self.options.files[0] + ".dump")


    def format_indexes(self, inds):
        return "_" + "".join(["[{}]".format(x) for x in inds])

    def create_indexed_variable_name(self, name, inds):
        x_id = ("{}".format(name))
        x_id = x_id + self.format_indexes(inds)
        x_id = x_id.replace("-", "n_")
        return x_id

    def get_varid(self, var):
        if var not in self.var2ids:
            self.var2ids[var] = self.max_id
            self.max_id += 1
        return self.var2ids[var]

    def lookup_varid(self, var):
        if var not in self.var2ids:
             print("Requested a variable  {} that does not exist".format(var))
             exit()
        return var, self.var2ids[var]

    def soft_lookup_varid(self, var, replace=True):
        if var not in self.var2ids:
             # ##print("Requested a variable  {} that does not exist".format(var), end = " Replace with ")

             if (replace == True):
                    # ##print(" True")
                    return self.lookup_varid(self.create_indexed_variable_name(LABEL_TRUE_VARS, []))
             if (replace == False):
                    # ##print(" False")
                    return self.lookup_varid(self.create_indexed_variable_name(LABEL_FALSE_VARS, []))
             exit()
        return var, self.var2ids[var]

    def get_l_bounds(self, i, N=None):
        return range(i + 1, min(2 * i, N - 1) + 1)

    def get_r_bounds(self, i, N=None):
        return range(i + 2, min(2 * i + 1, N) + 1)

    def get_p_bounds(self, j, N=None):
        return range(int(max(math.floor(j / 2), 1)), (min((j - 1), N) + 1))

    def get_lambda_bounds(self, i):
        return range(int(math.floor(i / 2)) + 1)

    def get_tau_bounds(self, i):
        return range(i + 1)


    def generete_l_variables(self, N):
        # l ij to indicate that node i has node j as the *left* child
        # i+1 <= j <= min(2*i, N-1)
        for i in range(1, N + 1):
            for j in self.get_l_bounds(i=i, N=N):
                if (j % 2 == 0):
                    var_name = self.create_indexed_variable_name(LABEL_L_VARS, [i, j])
                    self.get_varid(var_name)
    def generete_r_variables(self, N):
        # r ij to indicate that node i has node j as the *right* child
        # i+2 <= j <= min(2*i+1, N)
        for i in range(1, N + 1):
            for j in self.get_r_bounds(i=i, N=N):
                if (j % 2 == 1):
                    var_name = self.create_indexed_variable_name(LABEL_R_VARS, [i, j])
                    self.get_varid(var_name)
    def generete_p_variables(self, N):
        # p ji = 1 iff the parent of node j is i.
        for j in range(2, N + 1):
            for i in self.get_p_bounds(j=j, N=N):
                var_name = self.create_indexed_variable_name(LABEL_P_VARS, [j, i])
                self.get_varid(var_name)
    def generete_v_variables(self, N):
        # A propositional variable v i = 1 iff i is a leaf node
        for i in range(1, N + 1):
            var_name = self.create_indexed_variable_name(LABEL_V_VARS, [i])
            self.get_varid(var_name)
    def generete_lambda_variables(self, N):
        for i in range(1, N + 1):
            for t in self.get_lambda_bounds(i=i):
                var_name = self.create_indexed_variable_name(LABEL_LAMBDA_VARS, [t, i])
                self.get_varid(var_name)
    def generete_tau_variables(self, N):
        for i in range(1, N + 1):
            for t in self.get_tau_bounds(i=i):
                var_name = self.create_indexed_variable_name(LABEL_TAU_VARS, [t, i])
                self.get_varid(var_name)

    def generete_constant_variables(self):
        var_name = self.create_indexed_variable_name(LABEL_TRUE_VARS, [])
        self.get_varid(var_name)

        var_name = self.create_indexed_variable_name(LABEL_FALSE_VARS, [])
        self.get_varid(var_name)

    def generete_a_variables(self, N, K):
        # this variable is assigned value 1 iff feature f r is assigned to node j, with 1 ≤ r ≤ K.
        for r in range(1, K + 1):
            for j in range(1, N + 1):
                var_name = self.create_indexed_variable_name(LABEL_A_VARS, [r, j])
                self.get_varid(var_name)
    def generete_u_variables(self, N, K):
        # u rj : this variable is assigned value 1 iff feature f r is being discriminated against by node j.
        for r in range(1, K + 1):
            for j in range(1, N + 1):
                var_name = self.create_indexed_variable_name(LABEL_U_VARS, [r, j])
                self.get_varid(var_name)
    def generete_d0_variables(self, N, K):
        # this variable is assigned value 1 iff feature f r is discriminated for value 0 by node j, or by one of its
        # ancestors.Concretely, any example exhibiting f r = 0 will be discriminated by node j iff d 0 rj = 1.
        for r in range(1, K + 1):
            for j in range(1, N + 1):
                var_name = self.create_indexed_variable_name(LABEL_D_ZERO_VARS, [r, j])
                self.get_varid(var_name)
    def generete_d1_variables(self, N, K):
        # this variable is assigned value 1 iff feature f r is discriminated for value 1 by node j, or by one of its
        # ancestors.Concretely, any example exhibiting f r = 1 will be discriminated by node j iff d 1 rj = 1.
        for r in range(1, K + 1):
            for j in range(1, N + 1):
                var_name = self.create_indexed_variable_name(LABEL_D_ONE_VARS, [r, j])
                self.get_varid(var_name)
    def generete_c_variables(self, N):
        # c j : class of leaf node i, which can either be 0 or 1.
        for i in range(1, N + 1):
            var_name = self.create_indexed_variable_name(LABEL_C_VARS, [i])
            self.get_varid(var_name)

    def generate_variables(self, N, K):
        self.generete_l_variables(N=N)
        self.generete_r_variables(N=N)
        self.generete_v_variables(N=N)
        self.generete_p_variables(N=N)
        self.generete_a_variables(N=N, K=K)
        self.generete_u_variables(N=N, K=K)
        self.generete_d0_variables(N=N, K=K)
        self.generete_d1_variables(N=N, K=K)
        self.generete_c_variables(N=N)
        self.generete_lambda_variables(N=N)
        self.generete_tau_variables(N=N)
        self.generete_constant_variables()


    def generate_constants_fixed(self):
        v_var_name, v_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_TRUE_VARS, []))
        self.s.add_clause([v_id])
        # ##print("True is fixed: add constraint -{}({})".format(v_var_name, [v_id]))

        v_var_name, v_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_FALSE_VARS, []))
        self.s.add_clause([-v_id])
        # ##print("False is fixed: add constraint -{}({})".format(v_var_name, [-v_id]))

    def generate_root_is_fixed(self, N):
        v_var_name, v_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [1]))
        self.s.add_clause([-v_id])
        # ##print("Root is not leaf: add constraint -{}({})".format(v_var_name, [-v_id]))

    def generate_leaf_has_no_children(self, N):
        # v_i\limply\neg l_{ij} &\quad\quad & i+1\le j\le\min(2i,N-1) \land\even(j)
        for i in range(1, N + 1):
            for j in self.get_l_bounds(i, N):
                clause = []
                v_var_name, v_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [i]))
                clause.append(-v_id)

                if (j % 2 == 1):
                    continue
                l_var_name, l_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
                clause.append(-l_var_id)
                self.s.add_clause(clause)
                # ##print("Leaf has not children: add constraint -{}({}) \/ not {}({}, {})".format(v_var_name, v_id, l_var_name, -l_var_id, clause))

    def generate_leaf_and_right(self, N):
       # l_{ij}\leftrightarrow r_{ij+1} & \quad\quad & \tn{with~} i+1\le j\le\min(2i,N-1)\land\even(j)\\
        for i in range(1, N + 1):
            for j in self.get_l_bounds(i, N):
                if (j % 2 == 0):
                    clause_f = []
                    l_var_name, l_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
                    r_var_name, r_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_R_VARS, [i, j + 1]))
                    self.s.add_clause([-l_var_id, r_var_id])
                    self.s.add_clause([l_var_id, -r_var_id])
                    # ##print("Left and right children together: add constraint {}({}) <->  {}({})".format(l_var_name, l_var_id, r_var_name, r_var_id))

    def generate_reified_card(self, reif_var_name, l_names):
        # -v -> l1+..ln=1

        _, v_id = self.lookup_varid(reif_var_name)

        clause = []
        for l_var in l_names:
            _, l_id = self.lookup_varid(l_var)
            clause.append(l_id)

        # ATLEAST
        atleast_clause = clause[:]
        atleast_clause.append(v_id)
        self.s.add_clause(atleast_clause)
        if (len(l_names) == 1):
               # trival case
               return

        # ATMOST
        if (len(l_names) == 2):
            at_most = []
            at_most.append(v_id)
            at_most.append(-clause[0])
            at_most.append(-clause[1])
            self.s.add_clause(at_most)
            ####print("NonLeaf has  children: ATMOST : add constraint -{} -> {} <= 1".format(v_var_name, l_names))
            ####print(at_most)

        if (len(l_names) > 2):
            at_most = clause[:]
            one_out = CardEnc().atmost(lits=at_most, top_id=self.max_id, encoding=self.options.enc)
            self.max_id = one_out.nv
            # copying cardinality constraint
            for cl in one_out.clauses[:-1]:
                self.s.add_clause(cl)
                ####print(cl)
            cl = one_out.clauses[-1]
            cl.append(v_id)
            ####print(cl)
            self.s.add_clause(cl)
            ####print("Leaf has not children: ATMOST : add constraint -{} -> {} <= 1".format(v_var_name, l_names))
    def generate_nonleaf_at_least_one_child(self, N):
        #   \neg v_i\rightarrow\left(\sum\limits_{\substack{j=i+1\\\even(j)}}^{\min(2i,N-1)}l_{ij}\geq 1\right)
        # v_i \/ li(i+1)\/ .. \/ li(min(2i,N-1)
        for i in range(1, N + 1):
            v_var_name, v_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [i]))
            l_names = []
            for j in self.get_l_bounds(i, N):
                if (j % 2 == 1):
                    continue
                l_var_name, l_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
                l_names.append(l_var_name)
#
            # ##print("Leaf has not children: card : add constraint -{} -> {} = 1".format(v_var_name, l_names))
            self.generate_reified_card(v_var_name, l_names)
#             ###print("NonLeaf has children: ATLEAST: add constraint -{}({}) -> {} >= 1".format(v_var_name, v_id,l_names))
#             ####print(clause)
#             #   \neg v_i\rightarrow\left(\sum\limits_{\substack{j=i+1\\\even(j)}}^{\min(2i,N-1)}l_{ij}\leq 1\right)
#             #v_i \/ l_{}
#             if (len(l_names) == 1):
#                # trival case
#                continue
#
#             if (len(l_names) == 2):
#                 at_most = []
#                 at_most.append(v_id)
#                 at_most.append(-clause[1])
#                 at_most.append(-clause[2])
#                 self.s.add_clause(at_most)
#                 ###print("NonLeaf has  children: ATMOST : add constraint -{} -> {} <= 1".format(v_var_name, l_names))
#                 ####print(at_most)
#
#             if (len(l_names) > 2):
#                 at_most = clause[1:]
#                 one_out = CardEnc().atmost(lits=at_most, top_id=self.max_id, encoding=self.options.enc)
#                 self.max_id = one_out.nv
#                 # copying cardinality constraint
#                 for cl in one_out.clauses[:-1]:
#                     self.s.add_clause(cl)
#                     ####print(cl)
#                 cl = one_out.clauses[-1]
#                 cl.append(v_id)
#                 ####print(cl)
#                 self.s.add_clause(cl)
#                 ###print("Leaf has not children: ATMOST : add constraint -{} -> {} <= 1".format(v_var_name, l_names))


    def generate_parent_has_child(self, N):
        for j in range(2, N + 1):
            for i in self.get_p_bounds(j=j, N=N):
                p_var_name, p_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_P_VARS, [j, i]))
                if (j % 2 == 0):
                    if (j in self.get_l_bounds(i, N)):
                        l_var_name, l_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
                        # ##print("A parent has a child: add constraint {} <-> {}".format(p_var_name, l_var_name))
                        self.s.add_clause([-p_var_id, l_var_id])
                        self.s.add_clause([p_var_id, -l_var_id])
                else:
                    if (j in self.get_r_bounds(i, N)):
                        r_var_name, r_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_R_VARS, [i, j]))
                        # ##print("A parent has a child: add constraint {} <-> {}".format(p_var_name, r_var_name))
                        self.s.add_clause([-p_var_id, r_var_id])
                        self.s.add_clause([p_var_id, -r_var_id])

    def generate_node_has_one_parent(self, N):
        for j in range(2, N + 1):
            p_var_ids = []
            p_var_names = []
            for i in self.get_p_bounds(j=j, N=N):
                p_var_name, p_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_P_VARS, [j, i]))
                p_var_ids.append(p_var_id)
                p_var_names.append(p_var_name)

            if (len(p_var_ids) > 1):
                ####print(p_var_names)
                one_out = CardEnc().equals(lits=p_var_ids, top_id=self.max_id, encoding=self.options.enc)
                self.max_id = one_out.nv
                # copying cardinality constraint
                for cl in one_out.clauses:
                    self.s.add_clause(cl)
            else:
                # if we have only one variable we just set this var = 1
                self.s.add_clause([p_var_id])
                ####print(p_var_ids)
                # ##print("A child has one parent: add constraint {} = 1".format(p_var_names))

    def generate_ordering_enforce(self, N):
        for i in range(1, N + 1):
            for k in range(i + 1, N + 1):
                v_i_var_name, v_i_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [i]))
                v_k_var_name, v_k_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [k]))
                for j in self.get_l_bounds(i, N):
                    if (j % 2 == 1):
                        continue
                    if (j <= k):
                        continue
                    l_i_j_var_name, l_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
                    for a in range(j - k - 1 + 1):
                        l_k_j_m_a_var_name, l_k_j_m_a_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [k, j - a]))
                        # ##print("Ordering  is enforced: add constraint {} \/ {}  \/ {} \/ {}".format(v_i_var_name, v_k_var_name, l_i_j_var_name, "-" + l_k_j_m_a_var_name))

    def generate_refine_bounds_lambda_enforce(self, N):
        for i in range(1, N + 1):
            lambda_var_name, lambda_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_LAMBDA_VARS, [0, i]))
            self.s.add_clause([lambda_id])
            # ##print("Refine bounds lambda: add constraint {} = 1".format(lambda_var_name))

        for i in range(1, N + 1):
            for t in self.get_lambda_bounds(i=i):
                if (t == 0):
                    continue
                assert(i > 0)
                lambda_t_i_var_name, lambda_t_i_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_LAMBDA_VARS, [t, i]))
                lambda_t_m_1_i_m_1_var_name, lambda_t_m_1_i_m_1_id = self.soft_lookup_varid(self.create_indexed_variable_name(LABEL_LAMBDA_VARS, [t - 1, i - 1]))
                lambda_t_i_m_1_var_name, lambda_t_i_m_1_id = self.soft_lookup_varid(self.create_indexed_variable_name(LABEL_LAMBDA_VARS, [t, i - 1]), replace=False)
                v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [i]))

                temp_t_m_1_i_m_1_v_var_name, temp_t_m_1_i_m_1_v_var_id = self.reified_and(lambda_t_m_1_i_m_1_var_name, v_var_name)
                # la[t,i] <-> la[t,i-1] \/ temp
                or_names = [lambda_t_i_m_1_var_name, temp_t_m_1_i_m_1_v_var_name]
                self.equivalence_lhs_literal_rhs_or(lambda_t_i_var_name, or_names)
                # ##print("Refine bounds lambda: add constraint {} <-> {}".format(lambda_t_i_var_name, or_names))

        for i in range(1, N + 1):
            # t = 0 case
            for t in self.get_lambda_bounds(i=i):
                if (t == 0):
                    continue
                lambda_t_i_var_name, lambda_t_i_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_LAMBDA_VARS, [t, i]))
                l_index = 2 * (i - t + 1)
                if (l_index in self.get_l_bounds(i=i, N=N)):
                    l_i_2it1_var_name, l_i_2it1_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, l_index]))
                    self.s.add_clause([-lambda_t_i_id, -l_i_2it1_id])
                    # ##print("Refine bounds lambda:  {} -> -{}".format(lambda_t_i_var_name, l_i_2it1_var_name))
                r_index = 2 * (i - t + 1) + 1
                if (r_index in self.get_r_bounds(i=i, N=N)):
                    r_i_2it2_var_name, r_i_2it2_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_R_VARS, [i, r_index]))
                    self.s.add_clause([-lambda_t_i_id, -r_i_2it2_id])
                    # ##print("Refine bounds lambda:  {} -> -{}".format(lambda_t_i_var_name, r_i_2it2_var_name))

    def generate_refine_bounds_tau_enforce(self, N):
        for i in range(1, N + 1):
            tau_var_name, tau_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_TAU_VARS, [0, i]))
            self.s.add_clause([tau_id])
            # ##print("Refine bounds  tau: add constraint {} = 1".format(tau_var_name))

        for i in range(1, N + 1):
            for t in self.get_tau_bounds(i=i):
                if (t == 0):
                    continue
                assert(i > 0)
                tau_t_i_var_name, tau_t_i_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_TAU_VARS, [t, i]))
                tau_t_m_1_i_m_1_var_name, tau_t_m_1_i_m_1_id = self.soft_lookup_varid(self.create_indexed_variable_name(LABEL_TAU_VARS, [t - 1, i - 1]))
                tau_t_i_m_1_var_name, tau_t_i_m_1_id = self.soft_lookup_varid(self.create_indexed_variable_name(LABEL_TAU_VARS, [t, i - 1]), replace=False)
                v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [i]))

                temp_t_m_1_i_m_1_v_var_name, temp_t_m_1_i_m_1_v_var_id = self.reified_and(a_var_name=tau_t_m_1_i_m_1_var_name, b_var_name=v_var_name, a_pos=1, b_pos=-1)
                # la[t,i] <-> la[t,i-1] \/ temp
                or_names = [tau_t_i_m_1_var_name, temp_t_m_1_i_m_1_v_var_name]
                self.equivalence_lhs_literal_rhs_or(tau_t_i_var_name, or_names)
                # ##print("Refine bounds tau: add constraint {} <-> {}".format(tau_t_i_var_name, or_names))

        for i in range(1, N + 1):
            # t = 0 case
            for t in range(int(math.ceil(i / 2)), i + 1):
                if (t == 0):
                    continue
                tau_t_i_var_name, tau_t_i_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_TAU_VARS, [t, i]))
                l_index = 2 * (t - 1)
                if (l_index in self.get_l_bounds(i=i, N=N)):
                    l_i_2it1_var_name, l_i_2it1_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, l_index]))
                    self.s.add_clause([-tau_t_i_id, -l_i_2it1_id])
                    # ##print("Refine bounds tau:  {} -> -{}".format(tau_t_i_var_name, l_i_2it1_var_name))
                r_index = 2 * t - 1
                if (r_index in self.get_r_bounds(i=i, N=N)):
                    r_i_2it2_var_name, r_i_2it2_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_R_VARS, [i, r_index]))
                    self.s.add_clause([-tau_t_i_id, -r_i_2it2_id])
                    # ##print("Refine bounds tau:  {} -> -{}".format(tau_t_i_var_name, r_i_2it2_var_name))



        # exit()

    def generate_bin_tree_constraints(self, N):
        # bin tree encoding
        self.generate_constants_fixed()
        self.generate_root_is_fixed(N=N)
        self.generate_leaf_has_no_children(N=N)
        self.generate_leaf_and_right(N=N)
        self.generate_nonleaf_at_least_one_child(N=N)
        self.generate_parent_has_child(N=N)
        self.generate_node_has_one_parent(N=N)
        # self.generate_ordering_enforce(N=N)
        self.generate_refine_bounds_lambda_enforce(N=N)
        self.generate_refine_bounds_tau_enforce(N=N)
#         # block
#         i = 8
#         j = 14
#         label = LABEL_L_VARS
#         v, id = self.lookup_varid(self.create_indexed_variable_name(label, [i, j]))
#         self.test_5_nodes(id)
#
#         i = 4
#         j = 7
#         label = LABEL_R_VARS
#         v, id = self.lookup_varid(self.create_indexed_variable_name(label, [i, j]))
#         self.test_5_nodes(id)

        # if self.s.solve() == True:
        #    self.build_graph(N = N, model = self.s.get_model(), filename = "graph.png")
            # build a graph
        #    ###print("Tree exists")
        # else:
        #    ###print("Error: Can not build a binary tree, UNSAT")
        #    exit()


    def reified_and(self, a_var_name, b_var_name, a_pos=1, b_pos=1):

        # create temp var
        temp_var_name = self.create_indexed_variable_name(LABEL_T_VARS + "_and_" + str(a_pos) + "_" + a_var_name + "_" + str(b_pos) + "_" + b_var_name, [])
        temp_id = self.get_varid(temp_var_name)
        _, a_id = self.lookup_varid(a_var_name)
        _, b_id = self.lookup_varid(b_var_name)

        # add a AND b <-> c
        self.s.add_clause([-a_id * a_pos, -b_id * b_pos, temp_id])
        self.s.add_clause([-temp_id, a_id * a_pos])
        self.s.add_clause([-temp_id, b_id * b_pos])
        ####print([-a_id*a_pos, -b_id*b_pos, temp_id])
        ####print([-temp_id, a_id*a_pos])
        ####print([-temp_id, b_id*b_pos])

        return temp_var_name, temp_id


    def equivalence_lhs_literal_rhs_or(self, a_var_name, b_var_names):
        # a <-> b or .. or b_n
        _, a_id = self.lookup_varid(a_var_name)
        rhs_or = []
        for b_var_name in b_var_names:
            _, b_id = self.lookup_varid(b_var_name)
            rhs_or.append(b_id)

        # -a or b or .. or b_n
        clause_f = rhs_or[:]
        clause_f.append(-a_id)
        self.s.add_clause(clause_f)

        for key, lit in enumerate(rhs_or):
            # a  \/ -b_i
            self.s.add_clause([a_id, -lit])

    def generate_discriminate_feature_0(self, N, K):
         # Discriminating a feature for value 0 at node $j$, with
        # $1< j\le N$:
        # \begin{equation} \label{eq:d0}
        # d^0_{rj}\leftrightarrow
        # \left(\bigvee_{i=\lfloor\frac{j}{2}\rfloor}^{j-1}
        # \left((p_{ji}\land d^0_{ri}) \lor (a_{ri}\land r_{ij})\right)
        # \right)
        # \end{equation}
        # And $d^{0}_{r,1}=0$.

        for r in range(1, K + 1):
            # d^{0}_{r,1}=0
            d0_var_name, d0_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ZERO_VARS, [r, 1]))
            self.s.add_clause([-d0_var_id])
            # ##print("Discriminate feature 0: add constraint -{}".format(d0_var_name))
            for j in range(2, N + 1):
                rhs_or_names = []
                for i in self.get_p_bounds(j=j , N=N):
                    # (p_{ji}\land d^0_{ri})
                    p_var_name, p_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_P_VARS, [j, i]))
                    d0_i_var_name, d0_i_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ZERO_VARS, [r, i]))
                    temp_p_d_var_name, temp_p_d_var_id = self.reified_and(p_var_name, d0_i_var_name)
                    rhs_or_names.append(temp_p_d_var_name)

                    # ##print("Discriminate feature 0: add constraint {}<-> {} AND {} ".format(temp_p_d_var_name, p_var_name, d0_i_var_name))

                    # (a_{ri}\land r_{ij})
                    if (j % 2 == 1 and j in self.get_r_bounds(i=i, N=N)):
                        a_var_name, a_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, i]))
                        r_var_name, r_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_R_VARS, [i, j]))
                        temp_a_r_var_name, temp_a_r_var_id = self.reified_and(a_var_name, r_var_name)
                        rhs_or_names.append(temp_a_r_var_name)
                        # ##print("Discriminate feature 0: add constraint {}<-> {} AND {} ".format(temp_a_r_var_name, a_var_name, r_var_name))


                # d0_{r,j} --> \/ of tem vars
                d0_r_var_name, d0_r_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ZERO_VARS, [r, j]))
                self.equivalence_lhs_literal_rhs_or(d0_r_var_name, rhs_or_names)
                # ##print("Discriminate feature 0: add constraint {}<-> {}  ".format(d0_r_var_name, rhs_or_names))


    def generate_discriminate_feature_1(self, N, K):
        #  Discriminating a feature for value 1 at node $j$, with
        #     $1< j\le N$:
        #     \begin{equation} \label{eq:d1}
        #       d^1_{rj}\leftrightarrow
        #       \left(\bigvee_{i=\lfloor\frac{j}{2}\rfloor}^{j-1}
        #       \left((p_{ji}\land d^1_{ri}) \lor (a_{ri}\land l_{ij})\right)
        #       \right)
        #     \end{equation}
        #     And $d^{1}_{r,1}=0$.

        for r in range(1, K + 1):
            # d^{0}_{r,1}=0
            d1_var_name, d1_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ONE_VARS, [r, 1]))
            self.s.add_clause([-d1_var_id])
            # ##print("Discriminate feature 1: add constraint -{}".format(d1_var_name))
            for j in range(2, N + 1):
                rhs_or_names = []
                for i in self.get_p_bounds(j=j , N=N):
                    # (p_{ji}\land d^0_{ri})
                    p_var_name, p_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_P_VARS, [j, i]))
                    d1_i_var_name, d1_i_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ONE_VARS, [r, i]))
                    temp_p_d_var_name, temp_p_d_var_id = self.reified_and(p_var_name, d1_i_var_name)
                    rhs_or_names.append(temp_p_d_var_name)

                    # ##print("Discriminate feature 1: add constraint {}<-> {} AND {} ".format(temp_p_d_var_name, p_var_name, d1_i_var_name))

                    # (a_{ri}\land r_{ij})
                    if (j % 2 == 0 and j in self.get_l_bounds(i=i, N=N)):
                        a_var_name, a_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, i]))
                        l_var_name, l_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
                        temp_a_l_var_name, temp_a_l_var_id = self.reified_and(a_var_name, l_var_name)

                        rhs_or_names.append(temp_a_l_var_name)
                        # ##print("Discriminate feature 1: add constraint {}<-> {} AND {} ".format(temp_a_l_var_name, a_var_name, l_var_name))


                # d1_{r,j} --> \/ of tem vars
                d1_r_var_name, d1_r_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ONE_VARS, [r, j]))
                self.equivalence_lhs_literal_rhs_or(d1_r_var_name, rhs_or_names)
                # ##print("Discriminate feature 1: add constraint {}<-> {}  ".format(d1_r_var_name, rhs_or_names))


    def generate_feature_used_in_node(self, N, K):
    #         \item Using a feature $r$ at node $j$, with
    #     $1\le r\le K$ and $1\le j\le N$:
    #     \begin{equation} \label{eq:uf1}
    #       \begin{array}{l}
    #         \bigwedge\limits_{i=\lfloor\frac{j}{2}\rfloor}^{j-1}
    #         \left(u_{ri}\land p_{ji}\limply \neg a_{rj}\right) \\[4pt]
    #         %
    #         %
    #         u_{rj} \leftrightarrow \left(a_{rj} \lor
    #         \bigvee\limits_{i=\lfloor\frac{j}{2}\rfloor}^{j-1}
    #         (u_{ri}\land p_{ji}) \right)\\
    #       \end{array}
    #     \end{equation}
        for r in range(1, K + 1):
            for j in range(1, N + 1):
                for i in self.get_p_bounds(j=j , N=N):
                    # \left(u_{ri}\land p_{ji}\limply \neg a_{rj}\right)
                    u_var_name, u_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_U_VARS, [r, i]))
                    p_var_name, p_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_P_VARS, [j, i]))
                    a_var_name, a_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, j]))
                    self.s.add_clause([-u_var_id, -p_var_id, -a_var_id])
                    # ##print("Feature used in a node: add constraint {}/\{} - > -{}  ".format(u_var_name, p_var_name,a_var_name))

                #         u_{rj} \leftrightarrow \left(a_{rj} \lor
                #         \bigvee\limits_{i=\lfloor\frac{j}{2}\rfloor}^{j-1}
                #         (u_{ri}\land p_{ji}) \right)\\
                u_j_var_name, u_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_U_VARS, [r, j]))
                a_var_name, a_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, j]))

                rhs_or_names = []
                for i in self.get_p_bounds(j=j , N=N):
                    u_var_name, u_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_U_VARS, [r, i]))
                    p_var_name, p_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_P_VARS, [j, i]))
                    temp_u_p_var_name, temp_u_p_var_id = self.reified_and(u_var_name, p_var_name)
                    rhs_or_names.append(temp_u_p_var_name)

                rhs_or_names.append(a_var_name)
                self.equivalence_lhs_literal_rhs_or(u_j_var_name, rhs_or_names)
                # ##print("Feature used in a node: add constraint {} <-> {}  ".format(u_j_var_name, rhs_or_names))

    def generate_nonleaf_has_one_feature(self, N, K):
#         \item For a non-leaf node $j$, exactly one feature is used:
#     \begin{equation} \label{eq:uf2}
#       \begin{array}{lcr}
#         %\neg v_j\rightarrow\left(\sum_{r=1}^{K}b_{rj}=1\right)
#         \neg v_j\rightarrow\left(\sum\limits_{r=1}^{K}a_{rj}=1\right)
#         & \quad\quad &
#         \tn{with~} j=1\ldots,N\\
#       \end{array}
#     \end{equation}
#
        for j in range(1, N + 1):
            v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [j]))
            a_names = []
            for r in range(1, K + 1):
                a_var_name, a_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, j]))
                a_names.append(a_var_name)

            self.generate_reified_card(v_var_name, a_names)
            # ##print("Non leaf has one feature: card : add constraint -{} -> {} = 1".format(v_var_name, a_names))

    def generate_leaf_has_no_features(self, N, K):
    # \item For a leaf node $j$, no feature is used:
    #     \begin{equation} \label{eq:uf3}
    #       \begin{array}{lcr}
    #         %v_j\rightarrow\left(\sum_{r=1}^{K}b_{rj}=0\right)
    #         v_j\rightarrow\left(\sum\limits_{r=1}^{K}a_{rj}=0\right)
    #         & \quad\quad &
    #         \tn{with~} j=1\ldots,N\\
    #       \end{array}
    #     \end{equation}
        for j in range(1, N + 1):
            v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [j]))
            for r in range(1, K + 1):
                a_var_name, a_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, j]))
                self.s.add_clause([-v_var_id, -a_var_id])
                # ##print("Leaf has no features: card : add constraint -{} \/ -{}".format(v_var_name, a_var_name))


    def generate_account_for_examples(self, N, K):
        for sample in self.data.samps:
            pos = sample[-1]
            sample = sample[:-1]
            print("generate_account_for_examples", sample)
            #sample = [1 if x > 0 else 0 for x in sample]
            #print("binary", sample)
            for j in range(1, N + 1):
                v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [j]))
                c_var_name, c_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_C_VARS, [j]))
                #
                r_or_names = []
                r_or = []
                for r in range(1, K + 1):
                    if (sample[r - 1] == 1):
                        d1_var_name, d1_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ONE_VARS, [r, j]))
                        r_or_names.append(d1_var_name)
                        r_or.append(d1_var_id)

                    if (sample[r - 1] == 0):
                        d0_var_name, d0_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ZERO_VARS, [r, j]))
                        r_or_names.append(d0_var_name)
                        r_or.append(d0_var_id)

                    if (self.options.prepfile is None):
                        assert((sample[r - 1] == 1)  or (sample[r - 1] == 0))
                    else:
                        assert((sample[r - 1] == DONOTCARE) or (sample[r - 1] == 1)  or (sample[r - 1] == 0))


                if (pos == 1):
                    temp_v_c_var_name, temp_v_c_var_id = self.reified_and(a_var_name=v_var_name, b_var_name=c_var_name, a_pos=1, b_pos=-1)
                else:
                    temp_v_c_var_name, temp_v_c_var_id = self.reified_and(a_var_name=v_var_name, b_var_name=c_var_name, a_pos=1, b_pos=1)

                r_or.append(-temp_v_c_var_id)
                self.s.add_clause(r_or)
                # self.s.add_clause([-c_var_id])
                ####print([-c_var_id])
                # ##print("Example sign {}: add constraint {} -> {}".format(pos, temp_v_c_var_name, r_or_names))
        #exit()
    def generate_atmost_one4non_binary_features(self, N, K):
        for j in range(1, N + 1):
            for nonbin, binfeats in self.data.nonbin2bin.items():
                #print(nonbin, binfeats)
                d0_vars = []
                d0_names = []
                for binfeat in binfeats:
                    r = self.data.nm2id[binfeat] + 1
                    if (r <= K):
                        # print("column ", index)
                        d0_var_name, d0_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ZERO_VARS, [r, j]))
                        d0_vars.append(d0_var_id)
                        d0_names.append(d0_var_name)
                # print(d0_vars, d0_names)
                if (len(d0_vars) > 1):
                    at_most = d0_vars[:]
                    one_out = CardEnc().atmost(lits=at_most, top_id=self.max_id, encoding=self.options.enc)
                    self.max_id = one_out.nv
                    # copying cardinality constraint
                    for cl in one_out.clauses:
                        self.s.add_clause(cl)
                
        #exit()
                        

                    
                

    def generate_classifer_constraints(self, N, K):
        self.generate_discriminate_feature_0(N=N, K=K)
        self.generate_discriminate_feature_1(N=N, K=K)
        self.generate_feature_used_in_node(N=N, K=K)
        self.generate_nonleaf_has_one_feature(N=N, K=K)
        self.generate_leaf_has_no_features(N=N, K=K)
        self.generate_account_for_examples(N=N, K=K)
        self.generate_atmost_one4non_binary_features(N=N, K=K)



#
#         d1_i_j_var_name, d1_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ONE_VARS, [1, 4]))
#         self.s.add_clause([d1_i_j_var_id])
#
#         d1_i_j_var_name, d1_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ONE_VARS, [1, 4]))
#         self.s.add_clause([d1_i_j_var_id])
#
#
#         c_var_name, c_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_C_VARS, [3]))
#         self.s.add_clause([-c_var_id])
#
#         c_var_name, c_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_C_VARS, [4]))
#         self.s.add_clause([c_var_id])
#
#         c_var_name, c_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_C_VARS, [5]))
#         self.s.add_clause([c_var_id])
        print("nb of clauses", self.s.nof_clauses())
        print("nb of vars", self.s.nof_vars())
        # exit()



    def print_solution_c(self, N, model, K=None):
        # ##print("c")
        for i in range(1, N + 1):
            c_var_name, c_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_C_VARS, [i]))
            if (model[c_var_id - 1] > 0):
                print(c_var_name + "= {}".format(1))
            else:
                print(c_var_name + "= {}".format(0))

    def print_solution_v(self, N, model, K=None):
        # ##print("v")
        for i in range(1, N + 1):
            v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [i]))
            if (model[v_var_id - 1] > 0):
                print(v_var_name + "= {}".format(1))
            else:
                print(v_var_name + "= {}".format(0))

    def print_solution_d_zero(self, N, model, K=None):
        # ##print("D0")
        for r in range(1, K + 1):
            for j in range(1, N + 1):
                d0_i_j_var_name, d0_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ZERO_VARS, [r, j]))
                if (model[d0_i_j_var_id - 1] > 0):
                    print(d0_i_j_var_name + "= {}".format(1))
                else:
                    print(d0_i_j_var_name + "= {}".format(0))

    def print_solution_d_one(self, N, model, K=None):
        # ##print("D1")
        for r in range(1, K + 1):
            for j in range(1, N + 1):
                d1_i_j_var_name, d1_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_D_ONE_VARS, [r, j]))
                if (model[d1_i_j_var_id - 1] > 0):
                    print(d1_i_j_var_name + "= {}".format(1))
                else:
                    print(d1_i_j_var_name + "= {}".format(0))

    def build_graph(self, N, model, filename="graph.png", K=None, labeled=False):
        self.s.file_dt = open(filename, "w")
        nodes = []
        labeldict = None
        self.s.file_dt.write("NODES\n");
        if (labeled == True):
                labeldict = {}
                for r in range(1, K + 1):
                    for j in range(1, N + 1):
                        a_r_j_var_name, a_r_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, j]))
                        if (model[a_r_j_var_id - 1] > 0):
                            labeldict[j] = "             " + self.data.names[r - 1] + "_" + str(r)
                            nodes.append((j, self.data.names[r - 1]))
                            self.s.file_dt.write("{} {}\n".format(j, self.data.names[r - 1]));
                            # ##print("r = {}, j = {}".format(r, j))
                for j in range(1, N + 1):
                    v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [j]))
                    c_var_name, c_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_C_VARS, [j]))
                    if (model[v_var_id - 1] > 0):
                        # terminal
                        v = 1  if (model[c_var_id - 1] > 0) else 0
                        labeldict[j] = "             c_" + str(v)
                        self.s.file_dt.write("{} {}\n".format(j, v));

        self.s.file_dt.write("EDGES\n");
        #print(nodes)
        edges = []
        for i in range(1, N + 1):
            for j in self.get_l_bounds(i=i, N=N):
                if (j % 2 == 0):
                    l_i_j_var_name, l_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
                    if (model[l_i_j_var_id - 1] > 0):
                        # we know that i is parent of j
                        #G.add_edge(i, j, weight=0)
                        edges.append((i, j, 1))
                        self.s.file_dt.write("{} {} {}\n".format(i, j, 1));

            for j in self.get_r_bounds(i=i, N=N):
                if(j % 2 == 1):
                    r_i_j_var_name, r_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_R_VARS, [i, j]))
                    if (model[r_i_j_var_id - 1] > 0):
                        # we know that i is parent of j
                        edges.append((i, j, 0))
                        self.s.file_dt.write("{} {} {}\n".format(i, j, 0));

                        #G.add_edge(i, j, weight=1)



        print (edges);
        self.s.file_dt.close()
#         exit()
#         G = nx.DiGraph()
#         edges = []
#         
#         
#         # ##print(model)
#         for i in range(1, N + 1):
#             for j in self.get_l_bounds(i=i, N=N):
#                 if (j % 2 == 0):
#                     l_i_j_var_name, l_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_L_VARS, [i, j]))
#                     if (model[l_i_j_var_id - 1] > 0):
#                         # we know that i is parent of j
#                         G.add_edge(i, j, weight=0)
#                         edges.append((i, j))
#             for j in self.get_r_bounds(i=i, N=N):
#                 if(j % 2 == 1):
#                     r_i_j_var_name, r_i_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_R_VARS, [i, j]))
#                     if (model[r_i_j_var_id - 1] > 0):
#                         # we know that i is parent of j
#                         edges.append((i, j))
#                         G.add_edge(i, j, weight=1)
# 
#         print(edges)
# 
#         labeldict = None
#         if (labeled == True):
#                 labeldict = {}
#                 for r in range(1, K + 1):
#                     for j in range(1, N + 1):
#                         a_r_j_var_name, a_r_j_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_A_VARS, [r, j]))
#                         if (model[a_r_j_var_id - 1] > 0):
#                             labeldict[j] = "             " + self.data.names[r - 1] + "_" + str(r)
#                             # ##print("r = {}, j = {}".format(r, j))
#                 for j in range(1, N + 1):
#                     v_var_name, v_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_V_VARS, [j]))
#                     c_var_name, c_var_id = self.lookup_varid(self.create_indexed_variable_name(LABEL_C_VARS, [j]))
#                     if (model[v_var_id - 1] > 0):
#                         # terminal
#                         v = 1  if (model[c_var_id - 1] > 0) else 0
#                         labeldict[j] = "             c_" + str(v)
# 
# 
#         # ##print(labeldict)
# 
#         pos = hierarchy_pos(G, 1)
#         nx.draw(G, pos=pos, with_labels=True)
# 
#         # node_labels = nx.get_node_attributes(G,'state')
#         nx.draw_networkx_labels(G, pos, labels=labeldict)
#         edge_labels = nx.get_edge_attributes(G, 'state')
#         nx.draw_networkx_edge_labels(G, pos, labels=edge_labels)
#         plt.savefig("bn_" + filename)
#         # ##print("nb of clauses", self.s.nof_clauses())
#         # plt.show()

    def test_5_nodes(self, var_id):
        self.s.add_clause([-var_id])

    def generate_formula(self, N, K=None):
        if K is None:
            K = len(self.data.samps[0]) - 1
        self.generate_variables(N=N, K=K)
        # ##print(self.var2ids)
        self.generate_bin_tree_constraints(N=N)
        self.generate_classifer_constraints(N=N, K=K)

        t = time.time()
        res = self.s.solve()
        if  res == True:
            sol_file_name = self.options.files[0].replace("Samples", "Results") + "_" +  "{}".format("best") + ".sol";
            self.build_graph(N=N, model=self.s.get_model(), filename=sol_file_name, K=K, labeled=True)
            # self.print_solution_d_zero(model = self.s.get_model(), N = N, K = K)
            # self.print_solution_d_one(model = self.s.get_model(), N = N, K = K)
            # self.print_solution_c(model = self.s.get_model(), N = N, K = K)
            # self.print_solution_v(model = self.s.get_model(), N = N, K = K)
            # build a graph
        else:
            print("UNSAT")
            
        sat_time = time.time() - t
        # ##print(sat_time)
        
        return res, sat_time



        # create and initialize primer
        # if self.options.indprime == False:
        #    self.init_solver()
#
#     def prepare_formula(self):
#         """
#             Prepare a MaxSAT formula for prime enumeration.
#         """
#
#         # creating a formula
#         self.formula = WCNF()
#
#         # formula's variables
#         self.orig_vars = max(self.data.fvmap.opp.keys())
#         self.formula.nv = self.orig_vars * 2
#
#         # soft clauses, p clauses and soft weights
#         self.id2var = {}
#         for v in xrange(1, self.orig_vars + 1):
#             if v not in self.data.deleted:
#                 self.formula.soft.append([-v])
#                 self.id2var[len(self.formula.soft)] = v
#
#                 self.formula.soft.append([-v - self.orig_vars])
#                 self.id2var[len(self.formula.soft)] = -v
#
#                 self.formula.hard.append([-v, -v - self.orig_vars])  # p clauses
#
#         self.formula.wght = [1 for cl in self.formula.soft]
#         self.formula.topw = len(self.formula.soft) + 1
#
#         # hard clauses
#         for sample in self.data.samps:
#             cl = map(lambda l: -l if l < 0 else l + self.orig_vars, sample[:-1]) + [sample[-1]]
#             self.formula.hard.append(cl)
#
#         # additional hard clauses (exactly one outcome possible)
#         outs = [self.data.fvmap.dir[(self.data.names[-1], l)] for l in self.data.feats[-1]]
#         one_out = CardEnc().equals(lits=outs, top_id=2 * self.orig_vars, encoding=self.options.enc)
#
#         # updating top id (take Tseitin variable into account)
#         self.formula.nv = one_out.nv
#
#         # copying cardinality constraint
#         for cl in one_out.clauses:
#             cl_dr = map(lambda x: -x + self.orig_vars if abs(x) <= self.orig_vars and x < 0 else x, cl)
#             self.formula.hard.append(cl_dr)
#
#         # add hard clauses filtering out unnecessary combinations
#         if self.options.filter:
#             self.filter_combinations()
#
#         if self.options.pdump:
#             fname = 'primes.{0}@{1}.wcnf'.format(os.getpid(), socket.gethostname())
#             self.formula.to_file(fname)
#
#     def init_solver(self):
#         """
#             Create an initialize a solver for prime enumeration.
#         """
#
#         # initializing prime enumerator
#         if self.options.primer == 'lbx':
#             self.mcsls = LBX(self.formula, use_cld=self.options.use_cld,
#                     solver_name=self.options.solver, use_timer=True)
#         elif self.options.primer == 'mcsls':
#             self.mcsls = MCSls(self.formula, use_cld=self.options.use_cld,
#                     solver_name=self.options.solver, use_timer=True)
#         else:  # sorted or maxsat
#             self.hitman = HitMinimum()
#
#             if self.options.trim:
#                 self.hitman.trim(self.options.trim)
#
#             # hard clauses are modeled as hard
#             for cl in self.formula.hard:
#                 self.hitman.hard(cl)
#
#             # soft clauses
#             for cl in self.formula.soft:
#                 self.hitman.soft(-cl[0])
#
#             # mapping is not really needed when working with Hitman
#             self.id2var = {}
#             for v in xrange(1, self.orig_vars + 1):
#                 self.id2var[v] = v
#                 self.id2var[v + self.orig_vars] = -v
#
#     def filter_combinations(self):
#         """
#             Filter out unnecessary combinations of a given size.
#         """
#
#         if self.options.filter == -1:
#             self.block_noncovers()
#             return
#
#         samps = []
#         for s in self.data.samps:
#             samps.append(set(s))
#
#         filtered = 0
#         for sz in xrange(1, self.options.filter + 1):
#             if sz == 1:
#                 for i, f in enumerate(self.data.feats[:-1]):
#                     if len(f) == 1:
#                         filtered += 1
#                         var = self.data.fvmap.dir[(self.data.names[i], list(f)[0])]
#                         if var > 0:
#                             self.formula.hard.append([-var - self.orig_vars])
#                         else:
#                             self.formula.hard.append([var])
#             else:  # currently a mess, but does the job
#                 for tup in itertools.combinations(xrange(len(self.data.feats[:-1])), sz):
#
#                     to_hit = []
#                     for i in tup:
#                         set_ = []
#
#                         for l in self.data.feats[i]:
#                             v = self.data.fvmap.dir[(self.data.names[i], l)]
#                             set_.extend([v, -v])
#
#                         to_hit.append(set_)
#
#                     # enumerating combinations with hitman
#                     # they are all trivial and of size == sz
#                     # because all sets to hit are disjoint
#                     # with HitMinimum(bootstrap_with=to_hit) as h:
#                     with HitMinimal(bootstrap_with=to_hit) as h:
#                         for hs in h.enum():
#                             hs = set(hs)
#
#                             # we have to check all samples
#                             for s in samps:
#                                 if hs.issubset(s):
#                                     break
#                             else:  # no sample covered by this combination
#                                 filtered += 1
#
#                                 cl = [-l if l > 0 else l - self.orig_vars for l in hs]
#                                 self.formula.hard.append(cl)
#
#         if self.options.verb:
#             ###print('c1 blocked {0} prime combs'.format(filtered))
#
#     def block_noncovers(self):
#         """
#             Add hard constraints to block all non-covering primes.
#         """
#
#         topv = self.formula.nv
#         ncls = len(self.formula.hard)
#         tvars = []  # auxiliary variables
#
#         allv = []
#         for v in xrange(1, self.data.fvars + 1):
#             allv.append(v)
#             allv.append(v + self.orig_vars)
#         allv = set(allv)
#
#         self.tcls = {self.data.fvmap.dir[(self.data.names[-1], c)]: [] for c in self.data.feats[-1]}
#
#         for sample in self.data.samps:
#             s = set([l if l > 0 else -l + self.orig_vars for l in sample[:-1]])
#
#             # computing the complement of the sample
#             compl = allv.difference(s)
#
#             # encoding the complement (as a term) into a set of clauses
#             if compl:
#                 topv += 1
#                 tvars.append(topv)
#                 self.tcls[sample[-1]].append(topv)
#
#                 for l in compl:
#                     self.formula.hard.append([-l, -topv])
#
#                 self.formula.hard.append(list(compl) + [topv])
#
#         if tvars:
#             if self.options.nooverlap:
#                 # at most one class can be covered by each prime
#                 classes = []
#                 for cls in self.tcls.itervalues():
#                     if len(cls) > 1:
#                         topv += 1
#                         classes.append(topv)
#                         for v in cls:
#                             self.formula.hard.append([-v, topv])
#                         self.formula.hard.append(cls + [-topv])
#                     else:
#                         classes.append(cls[0])
#
#                 one_c = CardEnc().atmost(lits=classes, top_id=topv, encoding=self.options.enc)
#                 self.formula.hard.extend(one_c.clauses)
#                 topv = one_c.nv
#
#             if self.options.indprime:
#                 self.tvars = tvars
#             else:
#                 # add final clause forcing to cover at least one sample
#                 self.formula.hard.append(tvars)
#
#             ###print('c1 blocked all non-covering primes')
#             ###print('c1 added more {0} vars and {1} clauses'.format(
#                 topv - self.formula.nv, len(self.formula.hard) - ncls))
#
#             self.formula.nv = topv
#
#     def compute(self):
#         """
#             Enumerate all prime implicants.
#         """
#
#         if self.options.primer in ('lbx', 'mcsls'):
#             return self.compute_mcsls()
#         else:  # sorted or maxsat
#             return self.compute_sorted()
#
#     def compute_mcsls(self):
#         """
#             Call an MCS enumerator.
#         """
#
#         if self.options.verb:
#             ###print('c1 enumerating primes (mcs-based)')
#
#         self.primes = collections.defaultdict(lambda: [])
#
#         if self.options.indprime:
#             self.formula.hard.append([])
#
#             mcses = []
#             for t in self.tvars:
#                 # set sample to cover
#                 self.formula.hard[-1] = [t]
#
#                 # initialize solver from scratch
#                 self.init_solver()
#
#                 # block all previosly computed MCSes
#                 for mcs in mcses:
#                     self.mcsls.block(mcs)
#
#                 # enumerate MCSes covering sample t
#                 for i, mcs in enumerate(self.mcsls.enumerate()):
#                     prime, out = self.process_mcs(mcs)
#
#                     # recording prime implicant
#                     self.primes[out].append(prime)
#
#                     # block
#                     self.mcsls.block(mcs)
#
#                     # record mcs to block later
#                     mcses.append(mcs)
#
#                     if self.options.plimit and i + 1 == self.options.plimit:
#                         break
#
#                 self.mcsls.delete()
#         else:
#             for mcs in self.mcsls.enumerate():
#                 prime, out = self.process_mcs(mcs)
#
#                 # recording prime implicant
#                 self.primes[out].append(prime)
#
#                 # block
#                 self.mcsls.block(mcs)
#
#             self.mcsls.delete()
#
#         # recording time
#         self.stime = resource.getrusage(resource.RUSAGE_SELF).ru_utime - self.init_stime
#         self.ctime = resource.getrusage(resource.RUSAGE_CHILDREN).ru_utime - self.init_ctime
#         self.time = self.stime + self.ctime
#
#         return self.primes
#
#     def compute_sorted(self):
#         """
#             MaxSAT-based prime implicant enumeration.
#         """
#
#         if self.options.verb:
#             ###print('c1 enumerating primes (maxsat-based)')
#
#         self.primes = collections.defaultdict(lambda: [])
#
#         if self.options.indprime:
#             self.formula.hard.append([])
#
#             mcses = []
#             for t in self.tvars:
#                 # set sample to cover
#                 self.formula.hard[-1] = [t]
#
#                 # initialize solver from scratch
#                 self.init_solver()
#
#                 # block all previosly computed MCSes
#                 for mcs in mcses:
#                     self.hitman.block(mcs)
#
#                 # enumerate MCSes covering sample t
#                 for i, mcs in enumerate(self.hitman.enum()):
#                     prime, out = self.process_mcs(mcs)
#
#                     # recording prime implicant
#                     self.primes[out].append(prime)
#
#                     # record mcs to block later
#                     mcses.append(mcs)
#
#                     if self.options.plimit and i + 1 == self.options.plimit:
#                         break
#
#                 self.hitman.delete()
#         else:
#             for mcs in self.hitman.enum():
#                 prime, out = self.process_mcs(mcs)
#
#                 # recording prime implicant
#                 self.primes[out].append(prime)
#
#             self.hitman.delete()
#
#         # recording time
#         self.stime = resource.getrusage(resource.RUSAGE_SELF).ru_utime - self.init_stime
#         self.ctime = resource.getrusage(resource.RUSAGE_CHILDREN).ru_utime - self.init_ctime
#         self.time = self.stime + self.ctime
#
#         return self.primes
#
#     def process_mcs(self, mcs):
#         """
#             Extract a prime implicant from MCS.
#         """
#
#         prime, out = [], None
#
#         for i in mcs:
#             # getting the corresponding variable
#             v_orig = self.id2var[i]
#
#             # filtering out all labels
#             if self.data.nm2id[self.data.fvmap.opp[abs(v_orig)][0]] == len(self.data.names) - 1:
#                 if v_orig > 0:
#                     out = v_orig
#
#                 continue
#
#             prime.append(v_orig)
#
#         # ###printing prime implicant
#         if self.options.verb > 2:
#             premise = []
#
#             for l in prime:
#                 feat, val = self.data.fvmap.opp[abs(l)]
#                 premise.append('{0}\'{1}: {2}\''.format('' if l > 0 else 'not ', feat, val))
#
#             if self.options.verb > 3:
#                 ###print('c1 mcs: {0}'.format(' '.join([str(l) for l in mcs])))
#
#             ###print('c1 prime: {0} => \'{1}: {2}\''.format(', '.join(premise), *self.data.fvmap.opp[out]))
#
#         return prime, out
