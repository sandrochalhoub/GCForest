from . import wrapper
from .utils import *

class AdaBoostClassifier:
    def __init__(self, cmd_line_args = [], **kwargs):
        self.args = ["adaboost.py", "--file", ""] + cmd_line_args
        self.opt = wrapper.parse(to_str_vec(self.args))
        
        for key in kwargs:
            setattr(self.opt, key, kwargs[key])

    def fit(self, X, Y):
        self.dataset = wrapper.WeightedDatasetI()
        
        for x, y in zip(X, Y):
            sample = to_int_vec(x + [y])
            self.dataset.addExample(sample)
        
        if self.opt.preprocessing:
            self.dataset.preprocess(self.opt.verbosity>=2)
        
        self.classifiers = wrapper.Adaboost(self.dataset, self.opt)

        self.classifiers.train()

    def predict(self, X):
        if not self.classifiers:
            raise ValueError("please call fit before predict!")
        Y = []

        for x in X:
            Y.append(self.classifiers.predict(to_int_vec(x)))

        return Y
