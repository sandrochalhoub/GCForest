#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
class GenericParser(object):

    def __call__(self,respath):
        res = {}
        for line in respath:
            if line.startswith("r "):
                data = line[2:].strip().split("=")
                res[data[0].strip()] = [float(data[1])]
        return res

def write_methods_table(o, tabname, methods, lvals):
    with open(tabname, 'r') as tabfile:
        # header
        tabfile.write("\\begin\{longtable\}\{lrrrr\}\n\\toprule\n")
        # columns
        tabfile.write("& \\multicolumn\{2\}\{c\}\{cart\} & \\multicolumn\{2\}\{c\}\{bud\}\\\\\n")
        tabfile.write("\\cmidrule(rr)\{2-3\}\\cmidrule(rr)\{4-5\}\n")

        for i in range(2):
            for val in ["train acc.", "test acc."]:
                tabfile.write("& \\multicolumn\{1\}\{c\}\{" + val + "\} ")

        tabfile.write("\\\\\n\\midrule")

        for val in lvals:
            tabfile.write("\\texttt{")

            # values
            for ...:
                tabfile.write("")

            tabfile.write("}")


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, GenericParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    o = Observation(e, parsers)
    train_acc = Statistic('train acc', label='train acc.', precision=lambda x:3, best=max)
    test_acc = Statistic('test acc', label='test acc.', precision=lambda x:3, best=max)

    l_max_depth = [3, 4, 5, 7, 10, 15]
    cart_methods = []
    bud_methods = []

    # One table per max_depth
    for max_depth in l_max_depth:
        m_cart = Method('cart_%i' % max_depth, stats=[train_acc, test_acc])
        m_bud = Method('bud_%i' % max_depth, stats=[train_acc, test_acc])

        cart_methods.append(m_cart)
        bud_methods.append(m_bud)

        o.write_table('tex/max_depth_%i.tex' % max_depth, [m_cart,m_bud], benches)

    # Summary table: Best value for all the proposed depths, for each datasets
    o.write_summary_table("tex/summary.tex", [train_acc, test_acc], methods=)

    # Table per dataset
    for b in benches:

        write_methods_table(o, "tex/%s.tex" % b.label, bud_methods, cart_methods)
        # o.write_summary_table('tex/%s.tex' % b.label, )
