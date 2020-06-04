#!/usr/bin/python3

import sys
import csv

import argparse

import numpy as np

from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import AdaBoostClassifier
from sklearn.model_selection import train_test_split
import sklearn.metrics as metrics

def read_dataset(filename):
    Y = []
    X = []

    with open(filename, newline='') as csvfile:
        reader = csv.reader(csvfile, delimiter=",")

        # Skip one example if the CSV has no header
        next(reader)

        for row in reader:
            Y.append(int(row[-1]))
            X.append([int(x) for x in row[:-1]])

    return X, Y

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("dataset", type=str, help="Dataset file name")
    parser.add_argument("--max_depth", default=3, type=int, help="Max depth of the trees")
    parser.add_argument("--n_estimators", default=30, type=int, help="Number of estimators in Adaboost")
    parser.add_argument("--split", default=0, type=float, help="Split between train and test data")
    parser.add_argument("--seed", default=12345, type=int, help="Seed for random generator, -1 for no seed (not implemented)")
    args = parser.parse_args();

    X, Y = read_dataset(args.dataset)

    if args.split != 0:
        print("split={}".format(args.split))
        train_x, test_x, train_y, test_y = train_test_split(X, Y, test_size=args.split, random_state=args.seed)
        do_test = True
    else:
        print("No test data")
        train_x, train_y = X, Y
        do_test = False

    rng = np.random.RandomState(args.seed)

    print("Running adaboost with max_depth={}, n_estimators={}".format(args.max_depth, args.n_estimators))
    adaboost = AdaBoostClassifier(DecisionTreeClassifier(max_depth=args.max_depth),
                            n_estimators=args.n_estimators, random_state=rng)

    adaboost.fit(train_x, train_y)

    train_pred = adaboost.predict(train_x)
    train_acc = metrics.accuracy_score(train_y, train_pred)
    print("[Results] Train acc={}".format(train_acc))

    if do_test:
        test_pred = adaboost.predict(test_x)
        test_acc = metrics.accuracy_score(test_y, test_pred)
        print("[Results] Test acc={}".format(test_acc))
