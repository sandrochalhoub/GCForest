#include <vector>
#include <string>

#include "Tree.hpp"
#include "Backtrack.hpp"
#include "CmdLine.hpp"
#include "WeightedDataset.hpp"
#include "Adaboost.hpp"

#define SWIG_FILE_WITH_INIT

extern blossom::DTOptions parse(std::vector<std::string> params);
extern void read_binary(blossom::WeightedDataset<int> &input, blossom::DTOptions &opt);
