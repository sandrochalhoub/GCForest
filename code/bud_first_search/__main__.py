import sys
import bud_first_search as bfs

test = bfs.TEST_SAMPLE

opt = bfs.parse(bfs.to_str_vec(sys.argv))
filename = str(opt.instance_file)

wood = bfs.Wood()
algo = bfs.BacktrackingAlgo(wood, opt)

if filename == "":
    print("No input file!")
    sys.exit(-1)
else:
    bfs.read_binary(algo, opt)

algo.minimize_error()
tree = algo.getSolution()
nodes, edges = bfs.read_tree(tree)
print("nodes:", nodes,"\n\nedges: ", edges)