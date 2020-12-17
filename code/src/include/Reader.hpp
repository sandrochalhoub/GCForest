#ifndef __BLOSSOM_READER_HH
#define __BLOSSOM_READER_HH

#include "CSVReader.hpp"
#include "TXTReader.hpp"
#include "TypedDataSet.hpp"
#include "WeightedDataset.hpp"

namespace blossom
{

	template <typename E_t>
	void read_non_binary(WeightedDataset<E_t> &base, DTOptions &opt) {

	  TypedDataSet input;

	  string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

	  auto target_column{-1};

	  if (opt.format != "guess")
	    target_column = opt.intarget;

	  if (opt.format == "csv" or (opt.format == "guess" and ext == "csv"))
	    csv::read(opt.instance_file,
	              [&](vector<string> &f) {
	                input.setFeatures(f.begin(), f.end(), target_column);
	              },
	              [&](vector<string> &data) {
	                input.addExample(data.begin(), data.end(), target_column);
	              });
	  else {

	    if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
	      target_column = 0;
	    }

	    txt::read(opt.instance_file, [&](vector<string> &data) {
	      input.addExample(data.begin(), data.end(), target_column);
	    });
	  }

	  // WeightedDataset base;

	  input.binarize(base);

	  // base.toInc(A);
	}

	template <typename E_t>
	void read_binary(WeightedDataset<E_t> &input, DTOptions &opt) {
	  // WeightedDataset input;

	  string ext{opt.instance_file.substr(opt.instance_file.find_last_of(".") + 1)};

	  auto target_column{-1};

	  if (opt.format != "guess")
	    target_column = opt.intarget;

	  if (opt.format == "csv" or (opt.format == "guess" and ext == "csv")) {
	    csv::read_binary(opt.instance_file, [&](vector<int> &data) {
	      input.addExample(data.begin(), data.end(), target_column);
	    });
	  } else {

	    if (opt.format == "dl8" or (opt.format == "guess" and ext == "dl8")) {
	      target_column = 0;
	    }

	    txt::read_binary(opt.instance_file, [&](vector<int> &data) {
	      input.addExample(data.begin(), data.end(), target_column);
	    });
	  }

	  // input.toInc(A);
	}
	
} // namespace blossom

#endif
