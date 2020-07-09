#! /usr/bin/env python

from rocknrun import *


# Parsers should return a dict stat-name -> value-list. value-list is a list of the different values that the stat takes during a run
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
        res[stat].append(val)

    def __call__(self,output):

        res = {}

        for line in output:
            if not line.startswith(self.dataflag):
                continue

            data = line[len(self.dataflag):].split()
            for st in data:
                stat,val = st.split(self.equal)
                self.store(stat,val,res)

        return res


def write_methods_table(o, tabname, methods, lvals):
    # TODO MARCHE PAS ###  PAS TESTE
    cols = ["cart", "bud"]
    subcols = ["train acc.", "test acc."]

    with open(tabname, 'r') as tabfile:
        # header
        tabfile.write("\\begin\{longtable\}\{lrrrr\}\n\\toprule\n")
        # columns
        for m in cols:
            tabfile.write("& \\multicolumn\{2\}\{c\}\{%s\} " % m)
        tabfile.write("\n")

        i = 2
        for m in cols:
            tabfile.write("\\cmidrule(rr)\{%i-%i\}" % (i, i + len(subcols) - 1))

        for i in range(2):
            for val in subcols:
                tabfile.write("& \\multicolumn\{1\}\{c\}\{%s\} " % val)

        tabfile.write("\\\\\n\\midrule\n")

        for val in lvals:
            tabfile.write("\\texttt\{%i\}" % val)

            # values
            for m in cols:
                for s in subcols:
                    tabfile.write(" & %.3f" % o.data)
                    pass

            tabfile.write("\\\\\n")


if __name__ == '__main__':
    e = Experiment()
    parsers = dict([(m, GenericParser()) for m in e.all_methods])
    benches = [Benchmark([b]) for b in e.all_benchmarks]

    o = Observation(e, parsers)
    train_acc = Statistic('ada_train_acc', label='train acc.', precision=lambda x:3, best=max)
    test_acc = Statistic('ada_test_acc', label='test acc.', precision=lambda x:3, best=max)

    l_max_depth = [3, 4, 5, 7, 10, 15]
    cart_methods = []
    bud_methods = []

    # One table per max_depth
    for max_depth in l_max_depth:
        m_cart = Method('cart_%i' % max_depth, stats=[train_acc, test_acc])
        m_bud = Method('bud_%i' % max_depth, stats=[train_acc, test_acc])

        cart_methods.append(m_cart.name)
        bud_methods.append(m_bud.name)

        o.write_table('tex/max_depth_%i.tex' % max_depth, [m_cart,m_bud], benches)

    # Summary table: Best value for all the proposed depths, for each datasets
    # o.write_summary_table("tex/summary_cart.tex", ["train acc", "test acc"], methods=cart_methods, bests=[max, max], benches=)
    # o.write_summary_table("tex/summary_bud.tex", ["train acc", "test acc"], methods=bud_methods, bests=[max, max])

    # Table per dataset
    for b in benches:
        o.write_summary_table("tex/%s.tex" % b.label, ["ada_train_acc", "ada_test_acc"], methods=cart_methods + bud_methods,
            bests=[max, max], precisions= [[3]] * 2, benchmarks={b.label})
        # write_methods_table(o, "tex/%s.tex" % b.label, bud_methods, cart_methods)
