# GCForest

## Usage

- Help

```
./gcforest -- help
```

- Command line example (the parameters are mandatory for the coherence of the CSV files, but are otherwise optional)

```
./gcforest file_path --max_depth 2 --ada_stop 200 --test_sample 0.2 --itermax 2 --obj_check 6 --obj_eps 1e-9
```

## Description of the output CSV files

Executing gcforest.cpp will produce two output CSV files: trial.csv and iter.csv.

Each row of the trial.csv file corresponds to a separate execution of gcforest, and is not overriden. The columns represent, in order from left to right: the file path, the maximal depth of the forest, ada_stop, the maximal number of iterations, the number of iterations after which the objective gap is verified, the objective epsilon, Adaboost's accuracy,	Adaboost's duration in ms,	GCForest's accuracy, GCForest's	number of iterations, and GCForest's duration in ms.

Each row of the iter.csv file corresponds to a separate iteration of a single gcforest execution. The file is overriden at each execution. The columns represent, in order from left to right: the objective and the forest accuracy.
