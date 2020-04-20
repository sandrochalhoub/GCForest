# Compatible with dl85 v0.0.10

import copy
import itertools

from dl85 import DL85Classifier, DL85Predictor

def save_dl85(path, model, feature_names):
    idpool = itertools.count(start=1)
    root = copy.deepcopy(model.tree_)
    root['id'] = next(idpool)
    current_nodes = [root]
    nodes = []
    edges = []

    while len(current_nodes) != 0:
        next_nodes = []

        for cnode in current_nodes:
            if not model.is_leaf_node(cnode):
                nodes.append((cnode['id'], feature_names[cnode['feat']]))

                cnode['left']['id'] = next(idpool)
                edges.append((cnode['id'], cnode['left']['id'], 0))
                next_nodes.append(cnode['left'])

                cnode['right']['id'] = next(idpool)
                edges.append((cnode['id'], cnode['right']['id'], 1))
                next_nodes.append(cnode['right'])
            else:
                nodes.append((cnode['id'], cnode['class']))

        current_nodes = next_nodes

    with open(path, "w") as f:
        f.write("NODES\n")

        for node in nodes:
            f.write("{} {}\n".format(*node))

        f.write("EDGES\n")

        for edge in edges:
            f.write("{} {} {}\n".format(*edge))
