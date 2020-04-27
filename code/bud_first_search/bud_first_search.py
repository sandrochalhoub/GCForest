
import wrapper.budFirstSearch as wrapper
import copy

def to_str_vec(str_list):
    vec = wrapper.str_vec(len(str_list))

    for i in range(len(str_list)):
        vec[i] = str_list[i]

    return vec

def to_sample_vec(samples):
    samples_vec = wrapper.example_vec(len(samples))

    for i in range(len(samples)):
        sample = samples[i]
        features_vec = wrapper.int_vec(sample[:-1])
        samples_vec[i] = wrapper.Example(features_vec, sample[-1])

    return samples_vec

def read_tree(tree):
    nodes = []
    edges = []

    def add_node(tree, node):
        if (node <= 1):
            nodes.append({"leaf": True, "feat": node})
        else:
            id = len(nodes)
            nodes.append({"leaf": False, "feat": tree.getFeature(node)})

            # Add edge 1
            node0 = tree.getChild(node, 0)
            edges.append({"parent": id, "child": len(nodes), "val": 0})
            add_node(tree, node0)

            # Add edge 2
            node1 = tree.getChild(node, 1)
            edges.append({"parent": id, "child": len(nodes), "val": 1})
            add_node(tree, node1)

    add_node(tree, tree.idx)
    return nodes, edges

class BudFirstSearch:
    """
    Parameters:
    ...
    """

    def __init__(self, args):
        self.args = ["bud_first_search.py", "--file", ""] + args
        self.nodes = None
        self.edges = None
        self.tree = None

    def fit(self, samples):
        args_vec = to_str_vec(self.args)
        samples_vec = to_sample_vec(samples)

        results = wrapper.search(args_vec, samples_vec)

        # Read tree
        nodes = []
        self.nodes = []
        self.edges = []

        for n in results.nodes:
            nodes += [{"leaf": n.leaf, "feat": n.feat}]
            self.nodes.append(copy.deepcopy(nodes[-1]))

        for e in results.edges:
            self.edges += [{"parent": e.parent, "child": e.child, "val": e.val}]

            if e.val == 0:
                nodes[e.parent]["left"] = nodes[e.child]
            else:
                nodes[e.parent]["right"] = nodes[e.child]

        self.tree = nodes[0]


def test_training():
    global TEST_SAMPLE
    opt = wrapper.parse(to_str_vec(["--max_depth", "3"]))
    wood = wrapper.Wood()

    alg = wrapper.BacktrackingAlgorithm(wood, opt)
    wrapper.addExamples(alg, to_sample_vec(TEST_SAMPLE))

    alg.minimize_error()
    tree = alg.getSolution()

    nodes, edges = read_tree(tree)
    print("nodes:", nodes)
    print("edges:", edges)


TEST_SAMPLE = [[1, 0, 1], [1, 1, 0], [0, 1, 1], [0, 0, 0]]

if __name__ == "__main__":
    test_training()
    # b = BudFirstSearch(["--max_depth", "3", "--erroronly"])
    # b.fit(TEST_SAMPLE)
