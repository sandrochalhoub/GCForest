from . import wrapper
from .utils import *

class AdaBoostClassifier:
    def __init__(self, cmd_line_args = [], **kwargs):
        self.args = ["adaboost.py", "--file", ""] + cmd_line_args
        self.opt = wrapper.parse(to_str_vec(self.args))
        
        for key in kwargs:
            setattr(self.opt, key, kwargs[key])

    def fit(self, X, Y):
        self.input = wrapper.WeightedDatasetI(self.opt)
        
        for x, y in zip(X, Y):
            self.input.addExample(x + [y])
        
        self.algo = wrapper.Adaboost(self.input, self.opt)

        self.algo.train()

    def predict(self, X):
        Y = []

        for x in X:
            Y.append(self.algo.predict(x))

        return Y
