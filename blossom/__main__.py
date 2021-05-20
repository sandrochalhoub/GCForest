import sys
import blossom 

opt = blossom.parse(bfs.to_str_vec(sys.argv))
filename = str(opt.instance_file)

# wood = blossom.WoodI()
dataset = blossom.WeightedDataset()

if filename == "":
    print("No input file!")
    sys.exit(-1)
else:
    blossom.read_binary(dataset, opt)

dataset.preprocess()

algo = blossom.BacktrackingAlgo(datset, opt)

# dataset.setup(algo)

algo.minimize_error()
tree = algo.getSolution()
nodes, edges = blossom.read_tree(tree)
print("nodes:", nodes,"\n\nedges: ", edges)
