# decisiontrees


Example to run:


python3 mindt/mindt.py --nbnodes 19 -a dtencoding  ./test/mouse-un.csv


"--nbnodes" is the starting number of nodes (should be odd)

the last argument is the input file. We assume that all features are binary and outcomes are binary as well.

<!-- Louis 12/02/2020 -->
## With other approaches

```bash
python3 mindt/mindt.py --nbnodes 19 -a increment ./test/mouse-un.csv
```

```bash
python3 mindt/mindt.py --nbnodes 15 -a phi ./test/mouse-un.csv
```

## Tests

Compute accuracy on the given dataset for given binary tree.
```bash
python3 mindt/test_dt.py --nbnodes 19 ./test/mouse-un.csv test/mouse-un.csv_dir/*_nodes/graphs/graph*.sol
```

## Experiment 1

```bash
python3 mindt/exp1.py --nbnodes 19 --add-amount 5 --start-amount 5 --kfold 3 test/mouse-un.csv
```

Regenerate the graphs

```bash
python3 mindt/exp1.py --only-graphs test/mouse-un.csv_dir/results*
```

See all the options in mindt/options.py
