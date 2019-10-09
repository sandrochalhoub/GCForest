
#include <iostream>
#include <vector>


#ifndef _PRIMER_TYPEDDATASET_HPP
#define _PRIMER_TYPEDDATASET_HPP

using namespace boost;


typedef int dtype;

#define NUMBER 0
#define SYMBOLIC 1
	
	
namespace primer {
	

	
/**********************************************
* TypedDataSet
**********************************************/
/// Representation of a list of examples
	class TypedDataSet {



public:
  /*!@name Parameters*/
  //@{
  /// list of values
  std::vector<std::string> feature_label;
	std::vector<dtype> feature_type;
	std::vector<int> feature_rank;

	std::vector<std::vector<double>> number_value;
	std::vector<std::vector<std::string>> string_value;
	
	std::vector<string> label;
  //@}

public:
  /*!@name Constructors*/
  //@{
  explicit TypedDataSet() {}
  //@}

  /*!@name Accessors*/
  //@{
	size_t numFeature() const { return feature_label.size(); }
	bool typed() const { return feature_type.size() == numFeature(); }
  template <typename RandomIt> void setFeatures(RandomIt beg, RandomIt end) {
    auto n_feature = (end - beg);
    feature_label.reserve(n_feature);
    for (auto f{beg}; f != end; ++f)
      addFeature(*f);
  }
  void addFeature(string &f) { feature_label.push_back(f); }
	
template <typename RandomIt> void addExample(RandomIt beg, RandomIt end, string& l) {
  auto n_feature = (end - beg);
	// assert(n_feature = numFeature());
	while(n_feature > numFeature())
	{
		string s("f" + to_string(numFeature() + 1));
		addFeature(s);
	}
	
	
		if(! typed())
		for (auto f{beg}; f != end; ++f) {
				typeFeature(*f);
			}
	
	
		for (auto f{beg}; f != end; ++f) {
			auto j{f-beg};
			std::stringstream convert(*f);
			switch(feature_type[j]) {
				case NUMBER:
				double d;
				convert >> d;
				number_value[feature_rank[j]].push_back(d);
				break;
				case SYMBOLIC:
				string_value[feature_rank[j]].push_back(*f);
				break;
			}
		}
		
		label.push_back(l);
}
	
	// guess the type of the k-th feature, given a value v
	void typeFeature(string &v) { 
	
		std::stringstream convert(v);
		double d;
		string s;
			
			try {
	      convert >> d;
				
				feature_type.push_back(NUMBER);
				feature_rank.push_back(number_value.size());
				number_value.resize(number_value.size()+1);
			} catch (const std::invalid_argument& ia) {
				// std::cerr << "Invalid argument: " << ia.what() << std::endl;
        
				feature_type.push_back(SYMBOLIC);
				feature_rank.push_back(string_value.size());
				string_value.resize(string_value.size()+1);
	    } 
    }
	
	
	
  std::ostream &display(std::ostream &os) const {

    os << "features:";
    // for (auto l : feature_label)
    for (auto i{0}; i < numFeature(); ++i)
      os << " " << feature_label[i];
    os << endl;


		for(auto e{0}; e<label.size(); ++e) {
			for(auto f{0}; f<numFeature(); ++f)
			{
				dtype t{feature_type[f]};
				int r{feature_rank[f]};
				switch(t) {
					case NUMBER:
					os << " " << number_value[r][e];
					break;
					case SYMBOLIC:
					os << " " << string_value[r][e];
					break;
				}   
			}
			os << endl;
		}

    return os;
  }
	
};

std::ostream &operator<<(std::ostream &os, const TypedDataSet &x) {
  return x.display(os);
}

}

#endif // _PRIMER_TYPEDDATASET_HPP
