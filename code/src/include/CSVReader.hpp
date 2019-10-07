#ifndef __CVS_READER_HH
#define __CVS_READER_HH

#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>


namespace csv
{

	// template<  >
template <typename data_type, typename feature_declaration,
          typename value_declaration, typename example_declaration>
void read1(const std::string &fn, feature_declaration add_feature,
           value_declaration add_value, example_declaration add_example,
           std::string delimeter = ",") {
  using std::cerr;
  try {
    std::ifstream ifs(fn);

    std::vector<std::string> row;
    std::vector<data_type> data_point;

    std::string line = "";

    // header
    getline(ifs, line);
    boost::algorithm::split(row, line, boost::is_any_of(delimeter));

    for (auto str{row.begin()}; str != row.end() - 1; ++str) {
      add_feature(*str);
    }
    int n = row.size() - 1;

    // Iterate through each line and split the content using delimeter
    while (getline(ifs, line)) {
      data_type y;
      row.clear();
      boost::algorithm::split(row, line, boost::is_any_of(delimeter));

      int e = 0;
      for (auto str : row) {
        std::stringstream convert(str);

        data_type v;
        convert >> v;

        if (e < n)
          add_value(data_point, e, v);
        else
          y = v;
        // data_point.push_back(v);

        // std::cout << " " << v;

        ++e;
      }
      // std::cout << std::endl;

      // // exit(1);
      // auto y{data_point.back();}
      // data_point.pop_back();

      add_example(data_point, y);
      data_point.clear();
    }

  } catch (std::exception &e) {
    std::cout.flush();
    cerr << "ERROR: " << e.what() << std::endl;
    exit(1);
  }
}

// template<  >
template <typename data_type, typename header_declaration,
          typename data_declaration>
void read(const std::string &fn, header_declaration notify_header,
          data_declaration notify_data, std::string delimeter = ",") {
  using std::cerr;
  try {
    std::ifstream ifs(fn);

    std::vector<std::string> row;
    std::vector<data_type> data_point;

    std::string line = "";

    // header
    getline(ifs, line);
    boost::algorithm::split(row, line, boost::is_any_of(delimeter));

    notify_header(row);

    // Iterate through each line and split the content using delimeter
    while (getline(ifs, line)) {
      row.clear();

      boost::algorithm::split(row, line, boost::is_any_of(delimeter));

      for (auto str : row) {
        std::stringstream convert(str);

        data_type v;
        convert >> v;

        data_point.push_back(v);
      }

      notify_data(data_point);
      data_point.clear();
    }

  } catch (std::exception &e) {
    std::cout.flush();
    cerr << "ERROR: " << e.what() << std::endl;
    exit(1);
  }
}

} // namespace dimacs

#endif
