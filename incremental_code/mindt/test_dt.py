import sys
import copy

from options import Options
from data import Data
from dt import BinaryDecisionTree

if __name__ == '__main__':
    # parsing command-line options
    options = Options(sys.argv)

    # parsing data
    if options.files and len(options.files) >= 2:
        data = Data(filename=options.files[0], mapfile=options.mapfile,
                separator=options.separator)
    else:
        raise ValueError("This program needs at least 2 files ([dataset file] [tree file])")

    N = options.nbnodes
    K = len(data.samps[0]) - 1

    for f in options.files[1:]:
        tree = BinaryDecisionTree(f, name_feature=data.names)

        correct_count = 0

        for sample in data.samps:
            sample = copy.deepcopy(sample)
            # Change the random value of the label to 0 or 1. See data.fvmap for
            # more details
            sample[-1] = int(data.fvmap.opp[sample[-1]][1])

            if tree.classify_example(sample):
                correct_count += 1

        print("{}: Accuracy = {}, {}/{} correct".format(f, correct_count / len(data.samps), correct_count, len(data.samps)))
