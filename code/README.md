# Bud First Search

Install by running

```
pip3 install --user .
```

from the `code` directory.

## Usage

- High level API (Sklearn estimator)

```python
from bud_first_search import BudFirstSearch

test_X = [[0, 0], [0, 1]]
test_Y = [0, 1]

clf = BudFirstSearchClassifier()
clf.fit(test_X, test_Y)

print(clf.nodes)
print(clf.edges)
print(clf.predict(test_X))
```

- Direct C++ wrapping : see file `code/examples/adaboost.py`

## Add wrapped methods

The simplest way to add a method to the python wrapper is to add its definition
to the file `bud_first_search/wrapper/swig/budFirstSearch.i`

Do the same when you want to add new classes, class fields, constructors, etc.
