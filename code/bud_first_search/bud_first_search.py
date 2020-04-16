
import wrapper.swig.budFirstSearch as wrapper

class BudFirstSearch:
    """
    Paramètres:
    """

    def __init__(self, args):
        self.args = args
        self.tree = None

    def fit(self, samples):
        args_vec = wrapper.str_vec(len(self.args))

        for i in range(len(self.args)):
            args_vec[i] = self.args[i]

        samples_vec = wrapper.example_vec(len(samples))

        for i in range(len(samples)):
            sample = samples[i]
            features_vec = wrapper.int_vec(sample[:-1])
            samples_vec[i] = wrapper.Example(features_vec, sample[-1])

        results = wrapper.search(args_vec, samples_vec)

        # Read tree
        nodes = []

        for n in results.nodes:
            nodes += [{"leaf": n.leaf, "feat": n.feat}]

        for e in results.edges:
            if e.val == 0:
                nodes[e.parent]["left"] = nodes[e.child]
            else:
                nodes[e.parent]["right"] = nodes[e.child]

        self.tree = nodes[0]


if __name__ == "__main__":
    b = BudFirstSearch(["au", "b", "a"])
    b.fit([[1, 0, 1], [0, 1, 0], [0, 1, 0]])