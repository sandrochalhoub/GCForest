# Bud First Search

Install by running

```
pip3 install --user .
```

## Usage

- High level API

```python
from bud_first_search import BudFirstSearch

test = [[0, 0, 1], [0, 1, 0]]

clf = BudFirstSearch()
clf.fit(test)

print(clf.nodes)
print(clf.edges)
```

- Direct C++ wrapping

```python
import bud_first_search as bfs

test = [[0, 0, 1], [0, 1, 0]]

# Options can be provided by command line arguments
opt = bfs.parse(bfs.to_str_vec(["--max_depth", "3"]))
# Or by changing the fields of opt
opt.verbosity = bfs.Verbosity.YACKING

wood = bfs.Wood()
algo = bfs.BacktrackingAlgo(wood, opt)

for sample in test:
  algo.addExample(sample)

algo.minimize_error()

tree = algo.getSolution()
print(tree.idx)

# Read the tree the same way as the C++ API does
# or use:
nodes, edges = bfs.read_tree(tree)
print(nodes, edges)
```

## Add wrapped methods

The simplest way to add a method to the python wrapper is to add its definition
to the file `bud_first_search/wrapper/swig/budFirstSearch.i`