#!/usr/bin/python3

import argparse
import csv


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Dataset preprocessing")
    parser.add_argument("dataset", type=str, help="Dataset file name")
    parser.add_argument("--remove", type=str, nargs="*", help="Remove a column from the dataset")
    parser.add_argument("--move_last", type=str, nargs="*", help="Move a column to the last position")
    parser.add_argument("--btoi", type=str, nargs="*", help="Convert yes/no / true/false to 0 and 1")
    parser.add_argument("-o", "--output", type=str, help="Output file")
    args = parser.parse_args();

    # Read dataset
    header = []
    rows = []

    with open(args.dataset) as datafile:
        reader = csv.reader(datafile, delimiter=",")
        header = next(reader)

        for row in reader:
            rows.append(row)

    # Process the dataset
    if args.remove:
        for col in args.remove:
            i = header.index(col)

            if i != -1:
                for row in rows:
                    del row[i]

                del header[i]

    if args.move_last:
        for col in args.move_last:
            i = header.index(col)

            if i != -1:
                for row in rows:
                    e = row.pop(i)
                    row.insert(len(row), e)

                e = header.pop(i)
                header.insert(len(header), e)

    if args.btoi:
        for col in args.btoi:
            i = header.index(col)

            if i != -1:
                for row in rows:
                    e = row[i]

                    if isinstance(e, str):
                        e = e.lower()
                        row[i] = 1 if e == "yes" or e == "true" else 0

    # Write the processed version of the dataset
    if args.output != None:
        with open(args.output, "w") as outfile:
            writer = csv.writer(outfile, delimiter=",")
            writer.writerow(header)

            for row in rows:
                writer.writerow(row)
