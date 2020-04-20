import sys
import os
import copy
import csv
import re
import random

from dtencoder import *

from utils import *
from data import Data
from options import Options

import numpy as np
from sklearn.model_selection import KFold
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator

# Call example:
# python3 mindt/exp1.py --nbnodes 15 --add-amount 5 --start-amount 5 --kfold 2 test/mouse-un.csv

def get_seed(seed=-1):
    """
        Returns a random seed if seed == -1. Otherwise, returns seed.
    """
    if seed == -1:
        seed = random.randrange(sys.maxsize)
    return seed

def compute_mean(x, values):
    if x == None:
        x = [[i for i in range(len(v))] for v in values]

    fold_count = len(values)
    mean_x = list({a for b in x for a in b})
    mean_x.sort()
    mean_count = len(mean_x)

    mean_y = [0 for i in range(mean_count)]

    for j in range(mean_count):
        # TODO check that padding is done correctly (use only the first)
        for i in range(fold_count):
            yvals = values[i]
            xvals = x[i]

            val = 0
            for k in range(len(xvals)):
                if xvals[k] > mean_x[j]:
                    break
                val = yvals[k]

            mean_y[j] += val

    mean_y = [a / fold_count for a in mean_y]
    return mean_x, mean_y

def save_graphs(filename, values, ylabel, x = None, xlabel = "Iterations", title = "", max_y = None, int_y = False, xlog = False):
    max_it = max([len(v) for v in values])
    fold_count = len(values)

    if x == None:
        x = [[i for i in range(len(v))] for v in values]

    # Compute mean values
    mean_x, mean_y = compute_mean(x, values)

    fig = plt.figure(figsize=(6, 6))
    ax = fig.gca()

    # All curves on one graph
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)

    if xlog:
        plt.xscale("log")

    if max_y:
        plt.ylim(0, max_y)

    ax.xaxis.set_major_locator(MaxNLocator(integer=True))

    if int_y:
        ax.yaxis.set_major_locator(MaxNLocator(integer=True))

    for i in range(fold_count):
        plt.plot(x[i], values[i])

    # Mean graph
    if fold_count > 1:
        plt.plot(mean_x, mean_y, linewidth=4.0)

    # plt.savefig(filename + "_mean.png")
    plt.savefig(filename + ".png")
    fig.clear()

def calculate_bounds(results):
    bounds = {}

    for entry in results:
        k = entry.sample_count

        if k in bounds:
            bounds[k] = min(entry.nodes_count, bounds[k])
        else:
            bounds[k] = entry.nodes_count
    return [(k, n) for k, n in bounds.items()]


def read_results(files):
    all_results = []

    for filename in files:
        results = []

        with open(filename, 'r') as csvfile:
            reader = csv.reader(csvfile, delimiter=',')
            i = 0

            for row in reader:
                if i != 0:
                    results.append(DTIncrementResultEntry(None, *row))
                i += 1

        bounds = calculate_bounds(results)
        print(bounds)
        if (len(results) != 0):
            print(results[0].features())
        all_results.append((results, bounds))

    return all_results

def read_trace(trace, options):
    """
        Parse trace to get data that were not saved in the results.csv
    """

    try:
        regex = r"(\d+) nodes with (\d+) samples: (\d+\.\d+(e[\+\-]\d+)?) s"
        all_times = []

        with open(trace, "r") as f:
            lines = f.readlines()

            if "END" not in lines[-2]:
                return False

            times = []
            time = 0

            for i in range(len(lines)):
                match = re.match(regex, lines[i])

                if match:
                    time += float(match.group(3))

                    if "UNSAT" in lines[i-1] or not options.minimize:
                        times.append(time)

                if "Total time" in lines[i]:
                    all_times.append(times)
                    times = []
                    time = 0

        return all_times
    except:
        return False

def look_for_conflicting_labels(data):
    found = set()

    for i in range(len(data.samps)):
        s1 = data.samps[i]

        for j in range(i + 1, len(data.samps)):
            s2 = data.samps[j]

            if s1[:-1] == s2[:-1] and s1[-1] != s2[-1]:
                print("Found identical features with different labels at ({}, {})".format(i, j))
                print("Features:", ",".join([str(u) for u in s1[:-1]]))
                found = found | {i, j}
    return found

def make_graphs(options, all_results, sol_dir, trace_file = None):
    """
        all_results: list of tuple (results, bounds) w/ results being a list
        of ResultEntry and bounds a list of tuple (sample count, number of nodes)
    """

    # Get graph data
    max_it = 0
    train_accs = []
    max_train_accs = []
    test_accs = []
    sample_counts = []
    sizes = []
    # time for one iteration (-> constant sample count)
    times = []

    for results, bounds in all_results:
        if options.minimize:
            entries = [e for e in results if (e.sample_count, e.nodes_count) in bounds]
            t = [sum([e.exec_time for e in results if e.sample_count == b[0]]) for b in bounds]
        else:
            entries = results
            t = [e.exec_time for e in results]

        train_accs.append([e.train_acc for e in entries])
        max_train_accs.append(list(np.maximum.accumulate(train_accs[-1])))
        test_accs.append([e.test_acc for e in entries])
        sample_counts.append([e.sample_count for e in entries])
        sizes.append([e.nodes_count for e in entries])
        times.append(list(np.cumsum(t)))

    if trace_file:
        trace_content = read_trace(trace_file, options)

        if trace_content:
            times = trace_content
        else:
            print("Trace could not be read.")

    # use whatever to print the graphs and so on
    max_sample_counts = [max(u) for u in sample_counts]
    max_sizes = [max(u) for u in sizes]
    max_times = [max(u) for u in times]
    save_graphs(sol_dir + "train_acc", train_accs, "Accuracy", title = "Training accuracy", max_y = 1.0)
    save_graphs(sol_dir + "max_train_acc", max_train_accs, "Accuracy", title = "Maximum training accuracy", max_y = 1.0)

    if options.kfold != -1 or options.split < 1.0:
        save_graphs(sol_dir + "test_acc", test_accs, "Accuracy", title = "Accuracy on test set", max_y = 1.0)

    save_graphs(sol_dir + "sample_count", sample_counts, "Sample count", title = "Sample count", int_y=True, max_y=max(max_sample_counts)*1.05)

    if options.minimize:
        save_graphs(sol_dir + "size", sizes, "Size of the tree", x = sample_counts, xlabel = "Sample count", title = "Minimum size tree against example count", int_y=True, max_y=max(max_sizes)*1.05)
        save_graphs(sol_dir + "size_acc", train_accs, "Accuracy", x = sizes, xlabel = "Size of the tree", title = "Training accuracy against size of the tree", max_y = 1.0)

    save_graphs(sol_dir + "times", times, "Time (s)", title = "Cumulative time", max_y=max(max_times)*1.05)
    xtimes = [times[i][:len(train_accs[i])] for i in range(len(times))]
    save_graphs(sol_dir + "time_acc", train_accs, "Accuracy", x = xtimes, xlabel = "Time (s)", title = "Training accuracy against time", max_y=1.0, xlog=True)
    save_graphs(sol_dir + "max_time_acc", max_train_accs, "Accuracy", x = xtimes, xlabel = "Time (s)", title = "Maximum training accuracy against time", max_y=1.0, xlog=True)

    print("RAM usage: {} Ko".format(resource.getrusage(resource.RUSAGE_SELF).ru_maxrss))

    # Print LateX stuff
    mean_train_acc = compute_mean(None, train_accs)[1]
    mean_time = compute_mean(None, times)[1]
    training_set_size = all_results[0][0][0].train_total
    #  num sample & Final acc / Max acc & Duration & sample count (%)
    """print("{} & {:.3f}/{:.3f} & {:.2f}/{:.2f} & {} ({:.1f}\\%)/{} ({:.1f}\\%)".format(
        training_set_size,
        mean_train_acc[-1], max(mean_train_acc),
        mean_time[-1], max(max_times),
        min(max_sample_counts), min(max_sample_counts) / training_set_size * 100,
        max(max_sample_counts), max(max_sample_counts) / training_set_size * 100
    ))"""

    # accuracy (avg of max) & time (avg) & num_samples (avg of max)
    print("{:.3f} & {:.2f} & {:.1f}({:.1f}\\%)".format(
        np.mean([max(u) for u in train_accs]),
        mean_time[-1],
        np.mean(max_sample_counts), np.mean(max_sample_counts) / training_set_size * 100
    ))

def run_exp1(options, filename):
    data = Data(filename=filename, mapfile=options.mapfile,
            separator=options.separator)

    # Remove conflicting labels
    conflicting_labels = look_for_conflicting_labels(data)

    if (len(conflicting_labels) != 0):
        data = data.select({i for i in range(len(data.samps))} - conflicting_labels)
        print("Removed {} examples with conflicting labels".format(len(conflicting_labels)))

    N_to_try = options.nbnodes

    # Split up data in kfold
    if options.kfold != -1:
        print("K-fold with k = {}".format(options.kfold))
        datasets = KFold(n_splits=options.kfold, shuffle = True).split(data.samps)
    else:
        print("Repeating {} times".format(options.repeat))
        # account for split will be included later
        datasets = [([i for i in range(len(data.samps))], []) for _ in range(options.repeat)]

    all_results = []
    total_time = 0

    sol_dir = data.fname + "_dir/"
    csv_dir = create_directory(sol_dir)

    k = 0
    sep = ", "

    for train_index, test_index in datasets:
        print("training set: {}, test set: {}".format(train_index, test_index))
        train_data = data.select(train_index)
        test_data = data.select(test_index) if len(test_index) != 0 else None

        N_to_try = options.nbnodes
        # seed differently
        options.seed = random.Random(options.seed).randrange(sys.maxsize)
        print("Changed seed: {}".format(options.seed))

        dtincrement = DTIncrement(train_data, options, test_data = test_data, save_results = False)
        dtincrement.graph_dir = "graphs{}/".format(k)
        # bounds: list of tuples (sample count, size of the smallest tree)
        if options.minimize:
            bounds, sat_time = dtincrement.optimal_by_max_samples(N_to_try)
        else:
            bounds, sat_time = dtincrement.fixed_node_count(N_to_try)

        print("Total time {}s\n\n".format(sat_time))

        results = dtincrement.results
        total_time += sat_time
        all_results.append((results, bounds))

        # Write CSV
        with open(sol_dir + "results{}.csv".format(k), 'w') as csv_file:
            csv_file.write(sep.join(DTIncrement.results_feat_names()) + "\n")

            for entry in results:
                csv_file.write(sep.join([str(f) for f in entry.features()]) + "\n")

        k += 1

    print("Total SAT solving time of the whole experience, but only for this dataset: {}s".format(total_time))
    make_graphs(options, all_results, sol_dir)


if __name__ == '__main__':
    print(sys.stdout.fileno())
    # making output unbuffered
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w')

    print(" ".join(sys.argv))

    # parsing command-line options
    options = Options(sys.argv)

    # Limit memory usage
    max_mem = options.max_memory * 2**30 if options.max_memory > 0 else options.max_memory
    soft, hard = resource.getrlimit(resource.RLIMIT_AS)
    resource.setrlimit(resource.RLIMIT_AS, (max_mem, hard))
    print("Memory limit set from {} to {} (hard is {})".format(soft, max_mem, hard))

    # seed
    options.seed = get_seed(options.seed)
    print("seed=", options.seed)

    if options.only_graphs:
        sol_dir = os.path.dirname(options.files[0]) + "/"
        all_results = read_results(options.files)
        make_graphs(options, all_results, sol_dir, trace_file=sol_dir + "out.txt")
        # make_graphs(options, all_results, sol_dir, trace_file=None)
    else:
        # parsing data (multiple datasets)
        for file in options.files:
            run_exp1(options, file)

    print("END")
