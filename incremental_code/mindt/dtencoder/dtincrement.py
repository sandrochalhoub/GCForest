import random
import copy
import sys
import os
import time
import collections
import subprocess

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import MaxNLocator

from .dtencoder import DTEncoder
from dl85 import DL85Classifier

from utils import *
from utils.dl85_helper import save_dl85
import cart
from dt import BinaryDecisionTree

class ResultEntry:
    # sample_count = number of samples in the formula
    def __init__(self, model, it_count, sample_count, nodes_count, train_good, train_bad, test_good = 0, test_bad = 0, exec_time = 0):
        self.model = model
        self.it_count = int(it_count)
        self.sample_count = int(sample_count)
        self.nodes_count = int(nodes_count)
        self.train_good = int(train_good)
        self.train_bad = int(train_bad)
        self.test_good = int(test_good)
        self.test_bad = int(test_bad)
        self.exec_time = float(exec_time)

        self.train_total = self.train_good + self.train_bad
        self.test_total = self.test_good + self.test_bad
        self.train_acc = self.train_good / self.train_total
        out_size = self.train_total - self.sample_count;
        self.out_acc = 1 - self.train_bad / out_size if out_size != 0 else 0.0
        self.test_acc = self.test_good / self.test_total if self.test_total != 0 else 0.0

    def features(self):
        return [self.it_count, self.sample_count, self.nodes_count, self.train_good, self.train_bad, self.test_good, self.test_bad, self.exec_time]

    def str(self):
        return "{}/{} [{}/{}]".format(self.train_acc, self.out_acc, self.train_bad, self.train_good - self.sample_count)


class DTIncrement:
    def results_feat_names():
        return ["it_count", "sample_count", "nodes_count", "train_good", "train_bad", "test_good", "test_bad"]

    def __init__(self, data, options, test_data = None, save_results = True):
        self.options = options
        # Number of examples for first training
        self.start_count = options.start_amount
        # Number of examples added for each iteration
        self.add_count = options.add_amount
        self.save_results = save_results
        self.max_it = options.max_it

        self.remove = options.remove_amount != 0
        self.remove_count = options.remove_amount
        self.remove_threshold = 0.0
        # beta
        self.remove_prob = 0.5
        # alpha
        self.remove_grow = 0.95
        # a
        self.weight_avg_factor = 0.95

        # Method to choose the examples to add
        available_sample_selections = {"random", "one_per_leaf", "weight"}
        if options.sample_selection not in available_sample_selections:
            raise ValueError("Unkown method: {}. Valid are {}".format(options.sample_selection, available_sample_selections))
        self.sample_selection = options.sample_selection

        # Method to choose when we remove an example instead of adding one
        available_removing_choices = {"fixed", "accuracy"}
        if options.removing_choice not in available_removing_choices:
            raise ValueError("Unkown method: {}. Valid are {}".format(options.removing_choice, available_removing_choices))
        self.removing_choice = options.removing_choice

        available_stop_conditions = {"fail", "cart"}
        if options.stop_condition not in available_stop_conditions:
            raise ValueError("Unkown method: {}. Valid are {}".format(options.stop_condition, available_stop_conditions))
        self.stop_condition = options.stop_condition


        self.data = data
        self.test_data = test_data
        self.sol_dir = self.data.fname + "_dir/"
        self.graph_dir = "graphs/"
        self.bud_exec = "bin/bud_first_search"

        # Internal variables
        self.results = []
        self.current_it = 0
        self.seed = self.options.seed
        self.random = random.Random(self.seed)

        self.cart_tree, self.cart_accuracy = cart.train_cart(self.data, max_depth = self.options.max_depth)
        print("CART accuracy for this dataset is: {}".format(self.cart_accuracy))

        self.remove_weights = collections.defaultdict(float)
        self.removed_ids = set()


    # =====

    def optimal_by_min_node_count(self, N, K=None):
        res = True
        N_start = N
        tot_time = 0

        while self.should_continue(res):
            used_ids = set()

            while self.should_continue(res):
                # Increment used examples count
                used_ids = self.get_next_dataset(used_ids, N)

                if not used_ids:
                    break

                data = self.data.select(used_ids)

                # Generate formula
                res, sat_time = self.train_classifier(data, N, K)
                print("{} nodes with {} samples: {} s".format(N, len(used_ids), sat_time))

                if res:
                    self.save_model(N, self.current_it)
                    self.add_result(N, used_ids, exec_time = sat_time)

                tot_time += sat_time

            if res:
                N -= 2
            elif N != N_start:
                N += 2
            else:
                N = None

        if self.save_results:
            self.save_by_min_results()

        return N, tot_time

    def save_by_min_results(self):
        Ns = {e.nodes_count for e in self.results}

        for N in Ns:
            samp_counts = [e.sample_count for e in self.results if e.nodes_count == N]
            samp_counts.sort()
            iters = [i for i in range(len(samp_counts))]

            # Write results to file for later use
            accs = []
            accs_on_test = []
            corrects = []

            for count in samp_counts:
                entry = self.get_result(N, count)
                accs.append(entry.train_acc)
                accs_on_test.append(entry.out_acc)
                corrects.append(entry.train_good)

            # Matplotlib graphs
            dir = create_directory(self.sol_dir + "{}_nodes".format(N) + "/")

            fig = plt.figure(figsize=(6, 6))
            # plot accs
            plt.title("Accuracy against number of examples for trees of {} nodes".format(N))
            plt.ylim(0, 1)
            plt.xlabel('Number of examples')
            plt.ylabel('Accuracy')
            plt.plot(samp_counts, accs)
            plt.savefig(dir + "acc.png")
            fig.clear()
            # plot accs_on_test
            plt.title("Accuracy on examples out of the formula \nagainst number of examples for trees of {} nodes".format(N))
            plt.ylim(0, 1)
            plt.xlabel('Number of examples')
            plt.ylabel('Accuracy')
            plt.plot(samp_counts, accs_on_test)
            plt.savefig(dir + "acc_on_rem.png")
            fig.clear()
            # plot number of correctly classified examples vs correct
            plt.title("Correctly classified examples \nvs number of examples in the formula \nfor trees of {} nodes".format(N))
            plt.ylim(0, len(self.data.samps))
            plt.xlabel('Iteration')
            plt.ylabel('Number of examples')
            plt.plot(iters, samp_counts)
            plt.plot(iters, corrects)
            plt.savefig(dir + "correct.png")
            fig.clear()


    # =====

    def optimal_by_max_samples(self, N, K=None):
        """
        Returns: a list of tuples (sample_count, nodes_count)
        """
        used_ids = set()
        tot_time = 0
        bounds = []
        found_bound = True

        while len(used_ids) < len(self.data.samps) and found_bound and self.should_continue(True):
            # Increment used examples count
            prev_bound = bounds[-1] if len(bounds) != 0 else 0, N
            used_ids = self.get_next_dataset(used_ids, prev_bound[1])

            if not used_ids:
                break

            data = self.data.select(used_ids)

            # Find minimal N for the amount of examples we have
            N2 = N
            found_bound = False
            res = True

            while self.should_continue(res):
                res, sat_time = self.train_classifier(data, N2, K)
                print("{} nodes with {} samples: {} s".format(N2, len(used_ids), sat_time))

                if res:
                    self.save_model(N2, self.current_it)
                    self.add_result(N2, used_ids, exec_time = sat_time)

                tot_time += sat_time

                if res:
                    N2 -= 2
                elif N2 < N:
                    found_bound = True
                    bounds.append((len(used_ids), N2 + 2))
                    self.encoder = oldencoder
                    print("{} examples: found {} nodes".format(*bounds[-1]))

                oldencoder = self.encoder

        if self.save_results:
            self.save_by_sample_results(bounds, N)

        return bounds, tot_time

    def save_by_sample_results(self, bounds, N):
        dir = create_directory(self.sol_dir)

        iter = [i for i in range(len(bounds))]
        Ks = [b[0] for b in bounds]
        Ns = [b[1] for b in bounds]

        accs = []
        for i in range(len(bounds)):
            entry = self.get_result(Ns[i], Ks[i])
            accs.append(entry.train_acc)

        fig = plt.figure(figsize=(6, 6))
        ax = fig.gca()
        plt.title("Training accuracy")
        plt.xlabel("Iteration")
        plt.ylabel("Accuracy")
        plt.ylim(0, 1)
        ax.xaxis.set_major_locator(MaxNLocator(integer=True))
        plt.plot(iter, accs)
        plt.savefig(dir + "train_acc.png")
        fig.clear()

        plt.title("Min tree size against number of examples used for training")
        plt.xlabel("Number of examples")
        plt.ylabel("Number of nodes")
        plt.ylim(0, N)
        ax.yaxis.set_major_locator(MaxNLocator(integer=True))
        plt.plot(Ks, Ns)
        plt.savefig(dir + "size.png")
        fig.clear()

        plt.title("Accuracy against number of nodes")
        plt.xlabel("Number of nodes")
        plt.ylabel("Accuracy")
        plt.ylim(0, 1)
        ax.xaxis.set_major_locator(MaxNLocator(integer=True))
        plt.plot(Ns, accs)
        plt.savefig(dir + "train_acc_size.png")
        fig.clear()


    # =====

    def fixed_node_count(self, N, K = None):
        """
            Run the formula only on the N given in parameters
        """
        tot_time = 0
        used_ids = set()
        bounds = []
        res = True

        while self.should_continue(res):
            # Increment used examples count
            used_ids = self.get_next_dataset(used_ids, N)

            if not used_ids:
                break

            data = self.data.select(used_ids)

            # Generate formula
            res, sat_time = self.train_classifier(data, N, K)
            print("{} nodes with {} samples: {} s".format(N, len(used_ids), sat_time))

            if res:
                self.save_model(N, self.current_it)
                self.add_result(N, used_ids, exec_time = sat_time)

            tot_time += sat_time
            bounds.append((len(used_ids), N))

        return bounds, tot_time


    # =====

    def should_continue(self, last_success):
        """
        last_success: if the last iteration was successful (= not UNSAT)
        """
        # Did we reach the maximum iterations count ?
        if self.max_it > -1 and self.current_it < self.max_it:
            return False

        if self.stop_condition == "cart":
            if len(self.results) == 0:
                return True
            else:
                return self.results[-1].train_acc < self.cart_accuracy

        elif self.stop_condition == "fail":
            return last_success

        else:
            raise ValueError("should_continue: Unknown policy".format(self.stop_condition))


    def train_classifier(self, data, N, K):
        if self.options.solver == "dl85":
            self.encoder = DL85Classifier(max_depth = self.options.max_depth, min_sup = 1, time_limit = max(0, self.options.solver_time))
            X = np.array([s[:-1] for s in data.samps])
            Y = np.array([s[-1] for s in data.samps])
            t = time.time()

            res = True
            try:
                self.encoder.fit(X, Y)
            except ...:
                print("Error in DL8.5")
                res = False

            if len(data.samps) <= self.start_count:
                self.check_all_correct(self.encoder, data)

            res, sat_time = res, time.time() - t

        elif self.options.solver == "bud_first":
            bud_dir = os.path.dirname(self.bud_exec)
            data_path = bud_dir + "/temp_data"
            tree_path = bud_dir + "/tree"

            # Write data
            with open(data_path, "w") as f:
                f.writelines([",".join(s) for s in data.samps])

            # Run solver
            solver_process = subprocess.Popen("{} {} --max-depth {}".format(self.bud_exec, data_path, self.max_depth), shell = True)

            # Collect results
            # time
            # success

        else:
            # SAT
            self.encoder = DTEncoder(data, self.options)
            res, sat_time = self.encoder.generate_formula(N, K)
        return res, sat_time

    def get_next_dataset(self, used_ids, N):
        """
            Compute the dataset for the next iteration, from the dataset used
            during the previous iteration.

            Returns a set of ids, or False if the algorithm should stop.
        """
        self.prev_ids = copy.deepcopy(used_ids)

        if len(used_ids) == 0:
            used_ids = self.get_start_samples()
        else:
            ## TEMP
            # self.add_lazy_constraints(used_ids, N)
            # break
            ##
            self.compute_weights(N)

            if self.removing_choice == "accuracy":
                acc = self.results[-1].smooth_acc
                p = self.remove_prob * acc

            elif self.removing_choice == "fixed":
                p = self.remove_prob * (1 - self.remove_grow**self.current_it)

            # Remove according to the previously stated probability
            if self.remove and self.random.random() < p:
                remove_ids = self.get_removed_samples(used_ids, N)
                assert len(used_ids & remove_ids) == len(remove_ids)
            else:
                remove_ids = set()

            if len(remove_ids) != 0:
                used_ids -= remove_ids
                print("Removed {} samples".format(len(remove_ids)))
            else:
                # If no example was removed we add an example. The dataset
                # must not be identical for two iterations!
                add_ids = self.get_additional_samples(used_ids, N)

                if len(add_ids) == 0:
                    return False

                used_ids |= add_ids
                print("Added {} samples".format(len(add_ids)))

        self.current_it += 1
        return used_ids

    def get_start_samples(self):
        """
            Get a subset of all the samples for generating the first formula
        """
        if self.start_count >= 0:
            count = max(2, min(self.start_count, len(self.data.samps)))
            print("Starting with {} samples".format(count))
        else:
            # if start_count == -1, we start with the entire dataset.
            print("No increment: using the whole dataset")
            count = len(self.data.samps)

        # start_samples = set(np.random.choice(range(len(self.data.samps)), count, replace = False))
        ids = [a for a in range(len(self.data.samps))]
        self.random.shuffle(ids)

        # Select two samples with a different class
        pos_samples = [i for i in ids if self.data.samps[i][-1] == 1]
        neg_samples = [i for i in ids if self.data.samps[i][-1] == 0]
        start_samples = [pos_samples[-1], neg_samples[-1]]

        for s in start_samples:
            ids.remove(s)

        # Add enough samples to fill the gap
        start_samples += ids[:(count-2)]

        return set(start_samples)

    def compute_weights(self, N):
        # Add weight to samples in branches with more wrongly classified than
        # correctly classified example
        tree = self.read_model(N, self.current_it)
        distrib = self.get_node_distribution(tree, [i for i in range(len(self.data.samps))])

        for node_id, samples in distrib.items():
            wrong_in_node = [i for i in samples if not samples[i]]
            # used_in_node = [i for i in samples if i in used_ids]

            if len(samples) == 0:
                continue

            weight = len(wrong_in_node) / len(samples)

            for id in samples:
                a = self.weight_avg_factor
                self.remove_weights[id] = (1-a) * self.remove_weights[id] + a * weight

    def get_additional_samples(self, used_ids, N):
        """
            Add several wrongly classified classified examples to the data.

            used_ids: ids that we do not test, typically the samples that we
                used to solve the problem.
        """
        wrong_ids = self.get_wrongly_classified(used_ids, N)

        if self.remove:
            wrong_ids = [i for i in wrong_ids if i not in self.removed_ids]

        if len(wrong_ids) == 0:
            print("No more data to add")
            return set()

        if self.sample_selection == "random":
            count = min(len(wrong_ids), self.add_count)
            # shuffle ids so we take at random
            self.random.shuffle(wrong_ids)
            wrong_ids = set(wrong_ids[:count])
            return wrong_ids

        elif self.sample_selection == "weight":
            count = min(len(wrong_ids), self.add_count)
            sorted_ids = sorted(wrong_ids, key = lambda x: self.remove_weights[x], reverse = True)
            return set(sorted_ids[:count])

        elif self.sample_selection == "one_per_leaf":
            # TODO make compatible with DL8.5
            tree = self.read_model(N, self.current_it)
            distrib = self.get_node_distribution(tree, used_ids)
            add_ids = set()

            for node_id in distrib:
                node_samps = distrib[node_id]
                wrong_samps = [i for i in node_samps if not node_samps[i]]

                if len(wrong_samps) != 0:
                    self.random.shuffle(wrong_samps)
                    add_ids.add(wrong_samps[0])

            return add_ids
        else:
            raise ValueError("Unknown method to add more samples: {}".format(self.method))

    def get_removed_samples(self, used_ids, N):
        # we cannot remove a sample if it is the only one of its class
        classes = collections.defaultdict(int)

        for i in used_ids:
            s = self.data.samps[i]
            classes[s[-1]] += 1

        # Remove N examples with the highest weights if their weights are above
        # a particular threshold
        sorted_ids = sorted(used_ids, key = lambda x: self.remove_weights[x], reverse = True)
        # Remove samples that are alone (see above)
        sorted_ids = [i for i in sorted_ids if classes[self.data.samps[i][-1]] > 1]

        remove_ids = set()

        for i in range(len(sorted_ids)):
            id = sorted_ids[i]
            w = self.remove_weights[id]

            if i >= self.remove_count or self.remove_threshold > w:
                break

            remove_ids.add(id)
            self.removed_ids.add(id)
            del self.remove_weights[id]

        return remove_ids


    def get_wrongly_classified(self, used_ids, N):
        """
            Returns an array containing the ids of wrongly classified examples
        """
        rem_ids = self.get_rem_ids(used_ids)

        tree = self.read_model(N, self.current_it)

        if self.options.solver == "dl85":
            pass
        else:
            # Check that all the used samples are correctly classified.
            # If not, we have a bug.
            self.check_all_correct(tree, self.data, used_ids)

        return self.get_classification_result(tree, self.data, rem_ids)[1]

    def check_all_correct(self, tree_or_model, data, ids = None):
        """
            Check that all the "used_ids" samples are correctly classified
        """
        expected = len(ids) if ids else len(data.samps)
        obtained = self.get_correct_count(tree_or_model, data, ids)
        assert obtained == expected, "{} == {}".format(obtained, expected)

    def get_correct_count(self, tree_or_model, data, ids = None):
        return len(self.get_classification_result(tree_or_model, data, ids)[0])

    def get_classification_result(self, tree_or_model, data, ids = None):
        """
            Test the whole dataset, returns a tuple of lists (correct_ids, wrong_ids).
            With each list containing all the ids of either correctly or incorrectly
            classified example
        """
        if ids == None:
            ids = [i for i in range(len(data.samps))]

        correct_ids = []
        wrong_ids = []

        if isinstance(tree_or_model, BinaryDecisionTree):
            for i in ids:
                sample = data.samps[i]

                if tree_or_model.classify_example(sample):
                    correct_ids.append(i)
                else:
                    wrong_ids.append(i)
        elif isinstance(tree_or_model, DL85Classifier):
            X = np.array([s[:-1] for s in data.samps])
            Y = np.array([s[-1] for s in data.samps])
            y_pred = tree_or_model.predict(X)

            for i in ids:
                if y_pred[i] == Y[i]:
                    correct_ids.append(i)
                else:
                    wrong_ids.append(i)

        return correct_ids, wrong_ids



    def get_rem_ids(self, used_ids):
        """
            Returns ids that were not used for training

            ids: the ids of all the samples already used for training
        """
        res = set(range(len(self.data.samps)))
        res -= used_ids
        return res

    def add_result(self, N, used_ids, exec_time = 0):
        tree = self.read_model(N, self.current_it)

        test_good = 0
        test_bad = 0

        if self.test_data:
            test_good = self.get_correct_count(tree, self.test_data)
            test_bad = len(self.test_data.samps) - test_good

        train_good = self.get_correct_count(tree, self.data)
        # check = self.get_correct_count(self.encoder, self.data)
        # assert train_good == check "{} == {} for {} samples".format(train_good, check, len(self.data.samps))

        entry = ResultEntry(self.encoder, self.current_it, len(used_ids), N,
            train_good, len(self.data.samps) - train_good, test_good, test_bad, exec_time)
        self.results.append(entry)

        # compute smooth accuracy
        accsum = 0
        # TODO
        k = min(5, len(self.results))

        for i in range(k):
            accsum += self.results[-1 - i].train_acc

        entry.smooth_acc = accsum / k

        if len(self.results) > 1:
            # self.save_dataset(tree, used_ids)
            pass

    def get_result(self, N, sample_count):
        return [e for e in self.results if e.nodes_count == N and e.sample_count == sample_count][0]

    def save_model(self, N, iteration, model = None):
        if model == None:
            model = self.encoder

        K = len(self.data.samps[0]) - 1
        dir = create_directory(self.sol_dir + self.graph_dir)
        graph_path = dir + "graph_{}.sol".format(iteration)

        if isinstance(model, DTEncoder):
            model.build_graph(N=N, model=model.s.get_model(), filename=graph_path, K=K, labeled=True)
        elif isinstance(model, DL85Classifier):
            save_dl85(graph_path, model, self.data.names)
        else:
            raise ValueError("model type not supported")

    def read_model(self, N, iteration):
        # if self.options.solver == "dl85":
        #    return self.encoder

        graph_path = self.sol_dir + self.graph_dir + "graph_{}.sol".format(iteration)
        return BinaryDecisionTree(graph_path, name_feature=self.data.names)

    def save_dataset(self, model, used_ids):
        """
            Save the dataset for this iteration, and also, added samples, removed
            samples, node distribution ?
            For every item in the dataset, save if it's correctly classified by the new
            model only (you can see how it's going with the previously saved iteration)
        """
        dir = create_directory(self.sol_dir + "datasets/")

        cur_res = self.results[-1]
        prev_res = self.results[-2]
        up = cur_res.train_acc > prev_res.train_acc

        path = dir + "iteration{}_{}".format(self.current_it, "up" if up else "down")

        assert(used_ids != self.prev_ids)
        added_ids = used_ids - self.prev_ids
        removed_ids = self.prev_ids - used_ids
        remaining_ids = self.get_rem_ids(added_ids | self.prev_ids)

        good_ids, bad_ids = self.get_classification_result(model, self.data)

        with open(path, "w") as f:
            f.write("Training accuracy going {} from {}/{} ({}%) to {}/{} ({}%)\n".format(
                "up" if up else "down", prev_res.train_good, prev_res.train_total,
                prev_res.train_acc, cur_res.train_good, cur_res.train_total, cur_res.train_acc
            ))

            f.write("\nAdded:\n")

            for i in added_ids:
                samp = self.data.samps[i]
                f.write(",".join([str(u) for u in samp]))
                f.write(" : {}\n".format("good" if i in good_ids else "bad"))

            f.write("\nRemoved\n")

            for i in removed_ids:
                samp = self.data.samps[i]
                f.write(",".join([str(u) for u in samp]))
                f.write(" : {}\n".format("good" if i in good_ids else "bad"))

            f.write("\nStarted with dataset:\n")

            for i in self.prev_ids:
                samp = self.data.samps[i]
                f.write(",".join([str(u) for u in samp]))
                f.write(" : {}\n".format("good" if i in good_ids else "bad"))

            f.write("\nRest of the dataset:\n")

            for i in remaining_ids:
                samp = self.data.samps[i]
                f.write(",".join([str(u) for u in samp]))
                f.write(" : {}\n".format("good" if i in good_ids else "bad"))



    # =====

    def add_lazy_constraints(self, used_ids, N):
        tree = self.read_model(N, self.current_it)
        distrib = self.get_node_distribution(tree, self.get_rem_ids(used_ids))
        features = self.get_discriminating_features(tree, distrib)

    def get_node_distribution(self, tree, ids):
        """
            Returns {node_id: {sample_id: (True if correctly classified, false otherwise)}}
        """
        # Init result dict
        distrib = {}
        for key, node in tree.map_nodes.items():
            if node.is_leaf():
                distrib[key] = {}

        # Run over all the examples
        for id in ids:
            sample = self.data.samps[id]
            distrib[tree.get_example_leaf(sample)][id] = tree.classify_example(sample)

        return distrib

    def get_discriminating_features(self, tree, distrib):
        ### <!!!> Do not use this method yet it does not work
        """
            For each node, remove the feature and search what features
            could replace this node. This will be used for adding new constraints
            to the encoding.
        """
        results = {}
        K = len(self.data.samps[0]) - 1

        for node_id, node in tree.map_nodes.items():
            if not node.is_leaf():
                # Find every leaf node impacted by this node
                # TODO not all descendant should be taken into account
                leaves = tree.get_descendant_leaves(node_id)
                print(node_id, leaves)

                # Samples belonging to class 1
                pos_samples = []
                # Samples belonging to class 0
                neg_samples = []

                for leaf_id in leaves:
                    samples = distrib[leaf_id]

                    for samp_id in samples:
                        sample = copy.deepcopy(self.data.samps[samp_id])
                        target = sample[-1]
                        sample = sample[:-1]

                        if target == 1:
                            pos_samples.append(sample)
                        else:
                            neg_samples.append(sample)

                # Compute and and or
                if len(pos_samples) != 0:
                    pos_samples = np.array(pos_samples, dtype=np.bool)
                    pos_and = np.logical_and.reduce(pos_samples)
                    pos_or = np.logical_or.reduce(pos_samples)
                else:
                    pos_and = [True for i in range(K)]
                    pos_or = [False for i in range(K)]

                if len(neg_samples) != 0:
                    neg_samples = np.array(neg_samples, dtype=np.bool)
                    neg_and = np.logical_and.reduce(neg_samples)
                    neg_or = np.logical_or.reduce(neg_samples)
                else:
                    neg_and = [True for i in range(K)]
                    neg_or = [False for i in range(K)]

                print(pos_and)

                # For each feature, test if they discriminate the good and bad examples
                features = []

                for k in range(K):
                    print(pos_and[k], neg_or[k], neg_and[k], pos_or[k])
                    if (pos_and[k] and not neg_or[k]) or (neg_and[k] and not pos_or[k]):
                        features.append(k)

                results[node_id] = features
                print(features)

        return results
