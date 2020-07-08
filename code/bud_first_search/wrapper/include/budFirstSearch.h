#include <vector>
#include <string>

#include "Tree.hpp"
#include "Backtrack.hpp"
#include "CmdLine.hpp"
#include "Adaboost.hpp"

#define SWIG_FILE_WITH_INIT

extern DTOptions parse(std::vector<std::string> params);
extern void read_binary(primer::BacktrackingAlgorithm<> &A, DTOptions &opt);
