#!/usr/bin/python3

import sys
import bud_first_search as bfs


if __name__ == "__main__":
    # Options can be provided by command line arguments
    opt = bfs.parse(bfs.to_str_vec(sys.argv))
    # Or by changing the fields of opt
    # opt.verbosity = bfs.Verbosity.YACKING

    wood = bfs.Wood()
    algo = bfs.BacktrackingAlgo(wood, opt)

    bfs.read_binary(algo, opt)

    algo.minimize_error()

    tree = algo.getSolution()

    # Read the tree the same way as the C++ API does
    # or use:
    nodes, edges = bfs.read_tree(tree)
    print(nodes, edges)
