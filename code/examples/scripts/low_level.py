import bud_first_search as bfs

test = [[0, 0, 1], [0, 1, 0]]

# Options can be provided by command line arguments
opt = bfs.parse(bfs.to_str_vec(["--max_depth", "3"]))
# Or by changing the fields of opt
opt.verbosity = bfs.Verbosity.YACKING

wood = bfs.Wood()
algo = bfs.BacktrackingAlgorithm(wood, opt)
# TODO change to algo.addExamples(test) ?
bfs.addExamples(algo, bfs.to_sample_vec(test))
algo.minimize_error()

tree = algo.getSolution()
print(tree.idx)

# Read the tree the same way as the C++ API does
# or use:
nodes, edges = bfs.read_tree(tree)
print(nodes, edges)