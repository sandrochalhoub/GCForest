from . import wrapper
from .utils import *

class AdaBoostClassifier:
    def __init__(self):
        self.opt = wrapper.parse(to_str_vec(["adaboost.py", "file"]))

    def fit(self, X, Y):
        self.algo = wrapper.Adaboost(self.opt)

        for x, y in zip(X, Y):
            self.algo.addExample(x + [y])

        self.algo.train()

    def predict(self, X):
        Y = []

        for x in X:
            Y.append(self.algo.predict(x))

        return Y
