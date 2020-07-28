#!/usr/bin/python3

import sys
import csv

import argparse

import numpy as np

from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import AdaBoostClassifier
from sklearn.model_selection import train_test_split
import sklearn.metrics as metrics

import bud_first_search as bud

def read_dataset(filename):
    Y = []
    X = []

    with open(filename, newline='') as csvfile:
        if filename.endswith(".dl8") or filename.endswith(".txt"):
            target_first = filename.endswith(".dl8")

            for line in csvfile.readlines():
                values = line.split(" ")

                if target_first:
                    Y.append(int(values[0]))
                    X.append([int(x) for x in values[1:]])
                else:
                    Y.append(int(values[-1]))
                    X.append([int(x) for x in values[:-1]])

        elif filename.endswith(".csv"):
            reader = csv.reader(csvfile, delimiter=",")

            # FIXME This skips one example if the CSV has no header :/
            next(reader)

            for row in reader:
                Y.append(int(row[-1]))
                X.append([int(x) for x in row[:-1]])

    return X, Y

def create_adabud(args):
    solver = bud.AdaBoostClassifier()
    solver.opt.ada_it = args.n_estimators
    solver.opt.max_depth = args.max_depth
    solver.opt.seed = args.seed
    solver.opt.search = args.search
    solver.opt.ada_stop = args.ada_stop

    return solver

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("dataset", type=str, help="Dataset file name")
    parser.add_argument("--solver", default="cart", type=str, help="What solver to use: cart, bud")
    parser.add_argument("--max_depth", default=3, type=int, help="Max depth of the trees")
    parser.add_argument("--n_estimators", default=30, type=int, help="Number of estimators in Adaboost")
    parser.add_argument("--ada_stop", default=0, type=int, help="For solver 'bud', number of iterations without any improvement before the algorithm stops.")
    parser.add_argument("--search", default=-1, type=int, help="Search size (for solver \"bud\")")
    parser.add_argument("--split", default=0, type=float, help="Split between train and test data")
    parser.add_argument("--seed", default=12345, type=int, help="Seed for random generator, -1 for no seed (not implemented)")
    # TODO use print_par & print_data
    parser.add_argument("--print_par", action="store_true", help="Print parameters")
    parser.add_argument("--print_data", action="store_true", help="Print datasets properties (number of instances, etc)")
    args = parser.parse_args();

    X, Y = read_dataset(args.dataset)

    if args.split != 0:
        print("p split={}".format(args.split))
        train_x, test_x, train_y, test_y = train_test_split(X, Y, test_size=args.split, random_state=args.seed)
        do_test = True
    else:
        print("No test data")
        train_x, train_y = X, Y
        do_test = False

    rng = np.random.RandomState(args.seed)

    print("p max_depth={}".format(args.max_depth))
    print("p ada_it={}".format(args.n_estimators))

    if args.solver == "cart":
        adaboost = AdaBoostClassifier(DecisionTreeClassifier(max_depth=args.max_depth),
                        n_estimators=args.n_estimators, random_state=rng)
    elif args.solver == "bud":
        adaboost = create_adabud(args)
    else:
        print("Unknown solver: %s" % args.solver)
        sys.exit(-1)

    adaboost.fit(train_x, train_y)

    # Get average size of the trees
    if args.solver == "cart":
        for clf in adaboost.estimators_:
            print("d ada_size={}".format(clf.tree_.node_count))

    train_pred = adaboost.predict(train_x)
    train_acc = metrics.accuracy_score(train_y, train_pred)
    train_err = len(train_y) - np.sum(np.array(train_y) == np.array(train_pred))
    print("d ada_train_acc={} ada_train_err={}".format(train_acc, train_err), end="")

    if do_test:
        test_pred = adaboost.predict(test_x)
        test_acc = metrics.accuracy_score(test_y, test_pred)
        test_err = len(test_y) - np.sum(np.array(test_y) == np.array(test_pred))
        print(" ada_test_acc={} ada_test_err={}".format(test_acc, test_err), end="")

    print("")
