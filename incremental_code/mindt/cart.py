
from sklearn import tree
from sklearn import metrics

import numpy as np


def train_cart(data, max_depth = 2):
    """
    Returns a tuple (accuracy, tree : sklearn.DecisionTreeClassifier)
    """

    # train a classifier with CART
    X = np.array([s[:-1] for s in data.samps])
    Y = np.array([s[-1] for s in data.samps])

    # "scikit-learn uses an optimised version of the CART algorithm"
    # src: https://scikit-learn.org/stable/modules/tree.html
    clf = tree.DecisionTreeClassifier(max_depth = max_depth)
    clf.fit(X, Y)

    # Compute accuracy
    y_pred = clf.predict(X)
    acc = metrics.accuracy_score(Y, y_pred)

    return clf, acc

def save_cart(clf, filename):
    """
    Save a tree generated with CART
    """
    raise Exception("Not implemented yet")
