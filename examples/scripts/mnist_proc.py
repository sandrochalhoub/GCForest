#!/usr/bin/python3

import os
import argparse
from mnist import MNIST


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Binarize mnist")
    parser.add_argument("data", type=str, help="MNIST data location")
    parser.add_argument("-o", "--output-dir", type=str, default="mnist", help="Output file containing the mnist dataset")
    parser.add_argument("--binarize", action="store_true", help="Binarize dataset")
    args = parser.parse_args();

    mndata = MNIST(args.data)
    # TODO merge train and test
    images, labels = mndata.load_training()

    feat_count = len(images[0])

    try:
        os.mkdir(args.output_dir)
        print("Created directory %s" % args.output_dir)
    except FileExistsError:
        print("Directory %s already exists." % args.output_dir)

    # Write one file per digit
    for d in range(10):
        filename = "%s/mnist_%d.csv" % (args.output_dir, d)

        with open(filename, "w") as outfile:
            # header
            for i in range(feat_count):
                outfile.write("f%d," % i)

            outfile.write("digit\n")

            # examples
            for i in range(len(images)):
                # Binarize features
                if args.binarize:
                    feats = [1 if u > 128 else 0 for u in images[i]]
                else:
                    feats = images[i]
                label = 1 if labels[i] == d else 0

                outfile.write("%s,%d\n" % (",".join([str(u) for u in feats]), label))
