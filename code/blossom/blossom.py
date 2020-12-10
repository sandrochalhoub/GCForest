from . import wrapper
from .utils import *
import copy

import numpy as np
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

class BlossomClassifier:
    """
    Scikit learn-compatible estimator to use Blossom with Scikit Learn
    meta-algorithms (like Adaboost)
    """

    def __init__(self, cmd_line_args = [], **kwargs):
        self.args = ["blossom.py", "--file", ""] + cmd_line_args
        self.opt = wrapper.parse(to_str_vec(self.args))

        for key in kwargs:
            setattr(self.opt, key, kwargs[key])

        """
        # Mimics the sanity check of Scikit learn to understand why it does not pass
        params = self.get_params()
        for name in kwargs:
            if params[name] is kwargs[name]:
                print(type(params[name]), params[name], "is", type(kwargs[name]), kwargs[name])
            else:
                print(type(params[name]), params[name], "is not", type(kwargs[name]), kwargs[name])
        """
        # When the classifier is cloned by AdaBoost, there is a check to verify if parameters
        # are the same on the original and the clone. The test uses the keyword "is" as above.
        # For some values, (eg floats or big integers) the check does not pass because the
        # values are taken from the C++ DTOptions object.
        # If we return kwargs as the parameter dict, we pass the sanity check.
        self.params = { key : kwargs[key] for key in self.get_param_names() } if len(kwargs) != 0 else None

        self.tree = None
        self.nodes = []
        self.edges = []

        self.classes_ = np.array([0, 1])
        self.n_classes_ = 2
        
        self.dataset = None

    def get_param_names(self):
        # TODO add useful parameters
        return {"max_depth", "time", "search", "seed", "mindepth", "minsize"}

    def get_params(self, deep = False):
        params = { key : getattr(self.opt, key) for key in self.get_param_names() }

        # if we were cloned: pass the sanity check
        if self.params == params:
            return self.params
        else:
            self.params = None

        return params

    def set_params(self, **kwargs):
        for key in kwargs:
            if hasattr(self.opt, key):
                setattr(self.opt, key, kwargs[key])

    def _binarize_data(self, X, Y):
        """
        Binarize data (only if necessary).
        Does not support multiclass.
        This methods just operates a translation.
        """
        vals = set()
        for x, y in zip(X, Y):
            vals.update(x)
            vals.update([y])

        if len(vals) != 2:
            print("Classification is not binary!")

        if vals == {0, 1}:
            Xb = X
            Yb = Y
        else:
            Xb = copy.deepcopy(X)
            Yb = copy.deepcopy(Y)

            for old, new in zip(vals, {0, 1}):
                print(old, new)
                for i in range(len(X)):
                    for j in range(len(X[i])):
                        if X[i][j] == old:
                            Xb[i][j] = new
                    if Y[i] == old:
                        Yb[i] = new
        return Xb, Yb


    def fit(self, X, Y, sample_weight=None):
        self.wood = wrapper.Wood()

        Xb, Yb = X, Y # self._binarize_data(X, Y)

        self.dataset = wrapper.WeightedDataset()
        if sample_weight is not None:
            self.algo = wrapper.WeightedBacktrackingAlgod(self.wood, self.opt)

            for x, y, w in zip(Xb, Yb, sample_weight):
                # scikit learn classes start at 1
                # self.algo.addExample(to_int_vec(list(x) + [y]), w)
                sample = to_int_vec(list(x) + [y])
                for i in range(w):
                    self.dataset.addExample(sample.begin(), sample.end(), -1) 
        else:
            self.algo = wrapper.BacktrackingAlgo(self.wood, self.opt)

            for x, y in zip(Xb, Yb):
                # scikit learn classes & features start at 1
                # self.algo.addExample(to_int_vec(list(x) + [y]))
                sample = to_int_vec(list(x) + [y])
                self.dataset.addExample(sample.begin(), sample.end(), -1)
                
        self.dataset.toInc(self.algo)     

        if self.opt.mindepth:
            if self.opt.minsize:
                self.algo.minimize_error_depth_size()
            else:
                self.algo.minimize_error_depth()
        else:
            self.algo.minimize_error()

        self.tree = self.algo.getSolution()
        self.nodes, self.edges = read_tree(self.tree)

        # free memory
        del self.algo
        del self.wood

    def predict(self, X):
        if not self.tree:
            raise ValueError("please call fit before predict!")

        Y = []

        for x in X:
            node_id = 0
            node = self.nodes[node_id]

            while not node["leaf"]:
                val = x[node["feat"]]
                node_id = [e["child"] for e in self.edges if e["parent"] == node_id and e["val"] == val][0]
                node = self.nodes[node_id]

            Y.append(node["feat"])

        return np.array(Y)

    def correct_count(self, samples):
        """
        Returns the number of examples that are correctly classified
        """
        correct_count = 0

        for sample in samples:
            y_pred = self.predict(sample[:-1])

            if y_pred == sample[-1]:
                correct_count += 1

        return correct_count


TEST_SAMPLE = [[1, 0, 1], [1, 1, 0], [0, 1, 1], [0, 0, 0]]
