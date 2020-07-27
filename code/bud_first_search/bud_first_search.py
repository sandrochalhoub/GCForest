from . import wrapper
from .utils import *
import copy

from enum import IntEnum

# utility enums from CmdLine.hpp

class Verbosity(IntEnum):
    SILENT=0
    QUIET=1
    NORMAL=2
    YACKING=3
    SOLVERINFO=4

class NodeStrategy(IntEnum):
    FIRST=0
    RANDOM=1
    ERROR=2
    ERROR_REDUCTION=3

class FeatureStrategy(IntEnum):
    MIN_ERROR=0
    ENTROPY=1
    GINI=2
    HYBRID=3

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

    def __init__(self, cmd_line_args = []):
        self.args = ["bud_first_search.py", "--file", ""] + cmd_line_args
        self.opt = wrapper.parse(to_str_vec(self.args))

        self.tree = None
        self.nodes = []
        self.edges = []

    def fit(self, samples):
        self.wood = wrapper.Wood()

        self.algo = wrapper.BacktrackingAlgo(self.wood, self.opt)

        for sample in samples:
            self.algo.addExample(sample)

        self.algo.minimize_error()
        self.tree = self.algo.getSolution()

        self.nodes, self.edges = read_tree(self.tree)

    def correct_count(self, samples):
        correct_count = 0

        for sample in samples:
            y_pred = self.predict(sample[:-1])

            if y_pred == sample[-1]:
                correct_count += 1

        return correct_count

    def predict(self, features):
        if not self.tree:
            raise ValueError("please call fit before predict!")

        node_id = 0
        node = self.nodes[node_id]

        while not node["leaf"]:
            val = features[node["feat"]]
            node_id = [e["child"] for e in self.edges if e["parent"] == node_id and e["val"] == val][0]
            node = self.nodes[node_id]

        return node["feat"]

TEST_SAMPLE = [[1, 0, 1], [1, 1, 0], [0, 1, 1], [0, 0, 0]]
