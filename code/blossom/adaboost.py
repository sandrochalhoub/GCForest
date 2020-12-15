from . import wrapper
from .utils import *

class AdaBoostClassifier:
    def __init__(self):
        self.opt = wrapper.parse(to_str_vec(["adaboost.py", "file"]))

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
