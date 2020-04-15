#!/usr/bin/env python
# -*- coding:utf-8 -*-
## created: 2019-3-11
## author： Hao HU
## dt.py => for constructing the binary decision tree from the .sol file
##          and can make the classification for given example
#---------------------------------------------------------------------------------#



#---------------------------------------------------------------------------------#
class BinaryNode(object):
    """
        class of Binary node in the binary decision tree
        Every node has a feature_value to assign;
                       two children
        If the node is leaf, the feature_value will be the class of this node,
        and all children will be none
    """
    def __init__(self, feature_value=None):
        self.feature_value = feature_value
        self.left_child = None
        self.right_child = None

    def set_left_child(self, left_child):
        self.left_child = left_child

    def set_right_child(self, right_child):
        self.right_child = right_child

    def is_leaf(self):
        if not self.left_child and not self.right_child:
            return True
        else:
            return False

    def get_feature_value(self):
        return self.feature_value

    def get_left_child(self):
        return self.left_child

    def get_right_child(self):
        return self.right_child

#--------------------------------------------------------------------------------#
class BinaryDecisionTree(object):
    """
        class of binary decision tree constructed from the file of .sol
        Every binary decision tree has serveral BinaryNodes defined before,
        It has the interface of transforming the file into the tree,
        and also can identify the class of a given example
    """
    def __init__(self, sol_path=None, name_feature=None):
        self.map_nodes = {}
        self.name_feature = []
        self.root = None
        self.nbnodes = 0

        if name_feature:
            self.name_feature = name_feature

        if sol_path:
           with open(sol_path, 'r') as fp:
               self.parse(fp)

        # root is the node with index 1
        if 1 in self.map_nodes.keys():
            self.root = self.map_nodes[1]

        # Louis 2020-02-21:
        # Save the id of each node so that we can use this decision tree in dtencoder
        for k, v in self.map_nodes.items():
            v.id = k

    def parse(self, fp):
        """
            the format of fp is defined before, see details in the .sol file
        """
        lines = fp.readlines()
        flag = -1
        for line in lines:
            line = line.strip()
            if line == "NODES":
                flag = 1
                continue
            elif line == "EDGES":
                flag = 2
                continue
            else:
                if flag == 1:
                    self.parse_node(line)
                    self.nbnodes += 1
                elif flag == 2:
                    self.parse_edge(line)
                else:
                    print("There are some wrong lines before the Nodes!!")
                    exit()

    def parse_node(self, line):
        index, feature_value = line.split(" ")
        binary_node = BinaryNode(feature_value=feature_value)
        self.map_nodes[int(index)] = binary_node

    def parse_edge(self, line):
        parent_index, child_index, direction = [int(item) for item in line.split(" ")]

        # find the binary nodes used in the parsement
        if parent_index not in self.map_nodes.keys() or child_index not in self.map_nodes.keys():
            print("Node " + str(parent_index) + " is not defined before!!")
            exit()
        if direction == 1:
            self.map_nodes[parent_index].set_left_child(self.map_nodes[child_index])
        elif direction == 0:
            self.map_nodes[parent_index].set_right_child(self.map_nodes[child_index])
        else:
            print("Direction " + str(direction) + " is not defined before !!")
            exit()

    def get_nbnodes(self):
        """
            2019-12-11: get the real number of nodes in the decision tree
        """
        return self.nbnodes

    def get_depth(self):
        """
            2019-12-6: get the depth of binary decision tree constructed
            the depth count from 0 -> root is in depth 0
        """
        if not self.root:
            raise ValueError("The solution file does not exist the node as root!!")

        return self.get_current_node_depth(self.root) - 1

    def get_current_node_depth(self, node):
        """
            get the depth for current node
            Input: node -> BinaryNode class
        """
        assert isinstance(node, BinaryNode)
        if node.is_leaf():
            return 1

        return max(self.get_current_node_depth(node.get_left_child()), self.get_current_node_depth(node.get_right_child())) + 1


    def draw_tree(self):
        pass

    def classify_exmaple_without_label(self, exmaple_w_label):
        """
            classify the example without the label, return the label classified by the decision tree
        """
        if not self.root:
            raise ValueError("The solution file dose not exist the node as root!!")
        current_node = self.root

        while(not current_node.is_leaf()):
            feature_value = current_node.get_feature_value()
            if not feature_value in self.name_feature:
                print(self.name_feature)
                raise ValueError("Feature " + feature_value + " is not in the list of features!!")

            feature_index = self.name_feature.index(feature_value)
            # 2019-6-21: find a bug of the direction for value of features
            if exmaple_w_label[feature_index] == 0:
                current_node = current_node.get_left_child()
            elif exmaple_w_label[feature_index] == 1:
                current_node = current_node.get_right_child()
            else:
                print("There are third kind of value for a binary feature!!")
                print(exmaple_w_label[feature_index])
                print(type(exmaple_w_label[feature_index]))
                raise ValueError("There are third kind of value for a binary feature!!")
        return int(current_node.get_feature_value())


    def classify_example(self, example):
        """
            classify the example, need the sequence of binary value of example, and
            the list of name of feature for the dataset, which comes from the self

            Returns true if the predicted label corresponds to the target label, False
            otherwise.
        """
        if not self.root:
            raise ValueError("The solution file dose not exist the node as root!!")

        label = example[-1]
        example = example[:-1]

        label_p = self.classify_exmaple_without_label(example)

        # find the leaf node
        if label == label_p:
            return True
        else:
            return False

    def get_path_features(self, example):
        """
            print the decision path of each example
            0 -> left
            1 -> right
            -1 -> finished
        """
        if not self.root:
            raise ValueError("The solution file dose not exist the node as root!!")
        current_node = self.root

        example = example[:-1]
        decision_path = []

        while(not current_node.is_leaf()):
            feature_value = current_node.get_feature_value()
            if not feature_value in self.name_feature:
                print("Feature " + feature_value + " is not in the list of features!!")
                exit()

            feature_index = self.name_feature.index(feature_value)
            if example[feature_index] == 1:
                decision_path.append((feature_value, 0))
                current_node = current_node.get_left_child()
            elif example[feature_index] == 0:
                decision_path.append((feature_value, 1))
                current_node = current_node.get_right_child()
            else:
                print("There are third kind of value for a binary feature!!")
                print(exmaple_w_label[feature_index])
                exit()

        decision_path.append((current_node.get_feature_value(), -1))
        return decision_path

    def get_path(self, example):
        """
            Returns an ordered list of the nodes involved in decision for
            an example.
        """
        if not self.root:
            raise ValueError("The solution file dose not exist the node as root!!")
        current_node = self.root

        example = example[:-1]
        nodes = []

        while(not current_node.is_leaf()):
            nodes.append(current_node)

            feature_value = current_node.get_feature_value()
            if not feature_value in self.name_feature:
                raise ValueError("Feature " + feature_value + " is not in the list of features!!")

            feature_index = self.name_feature.index(feature_value)
            if example[feature_index] == 1:
                current_node = current_node.get_left_child()
            elif example[feature_index] == 0:
                current_node = current_node.get_right_child()
            else:
                raise ValueError("There are third kind of value for a binary feature!!\n{}"
                            .format(exmaple_w_label[feature_index]))

        nodes.append(current_node)
        return nodes

    def get_example_leaf(self, example):
        """
            Returns the id of the leaf node where the example got classified.
        """
        return self.get_path(example)[-1].id

    def get_descendant_leaves(self, node_id):
        """
            Returns all the leaves descending from the given node.
        """
        current_nodes = [self.map_nodes[node_id]]
        leave_nodes = []

        while len(current_nodes) != 0:
            next_nodes = []

            for node in current_nodes:
                left_node = node.get_left_child()

                if left_node.is_leaf():
                    leave_nodes.append(left_node.id)
                else:
                    next_nodes.append(left_node)

                right_node = node.get_right_child()

                if right_node.is_leaf():
                    leave_nodes.append(right_node.id)
                else:
                    next_nodes.append(right_node)

                current_nodes = next_nodes

        return leave_nodes

    def get_similar_branches(self, target_branch, removed_feature):
        """
            This function is used for the lazy clause constraints

            Parameters:
                branch: list of feature names
                removed_feature: feature name
        """
        current_branches = []
        final_branches = []

        while len(current_branches) != 0:
            next_branches = []

            for branch in current_branches:
                last_node = branch[-1]

                if last_node.is_leaf():
                    final_branch.append(branch)
                else:
                    if last_node.left_child.feature_value in target_branch: # and has the same direction (0 or 1) !!!
                        next_branches.append(branch + [last_node.left_child])
                    # TODO same
                    next_branches.append(branch + [last_node.right_child])

            current_branches = next_branches

        return final_branches
