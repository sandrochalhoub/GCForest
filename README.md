# Blossom, a Fast Decision Tree Learner

Install by running

```
pip3 install --user .
```

from the root directory.

## Usage

- Command line

to learn a decision tree classifier:

```
bin/blossom <datafile> [options]
```

to binarize a non-binary dataset:

```
bin/binarizer <datafile> --print_ins [--output filename]
```	

to compile a table:
	
```
bin/compile <datafile> [options]
```	

- High level API (Sklearn estimator)

```python
import blossom

X = [[0, 0], [0, 1]]
Y = [0, 1]

clf = blossom.BlossomClassifier()
clf.fit(X, Y)

print(clf.nodes)
print(clf.edges)
print(clf.predict(X))
```

- Direct C++ wrapping : see file `code/examples/blossom.py`

## Add wrapped methods

The simplest way to add a method to the python wrapper is to add its definition
to the file `blossom/wrapper/swig/blossom.i`

Do the same when you want to add new classes, class fields, constructors, etc.
