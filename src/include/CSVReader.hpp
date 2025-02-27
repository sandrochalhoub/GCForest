#ifndef __CVS_READER_HH
#define __CVS_READER_HH

#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/lexical_cast.hpp>

namespace csv
{

// template<  >
template <typename header_declaration, typename data_declaration>
void read(const std::string &fn, header_declaration notify_header,
          data_declaration notify_data, std::string delimeter = ",;|\t") {
  using std::cerr;
  try {
    std::ifstream ifs(fn);

    std::vector<std::string> row;

    std::string line = "";

    // header
    getline(ifs, line);
    boost::algorithm::split(row, line, boost::is_any_of(delimeter));

    notify_header(row);

    // Iterate through each line and split the content using delimeter
    while (getline(ifs, line)) {
      row.clear();
      boost::algorithm::split(row, line, boost::is_any_of(delimeter));
      for (auto i{0}; i < row.size(); ++i)
        boost::algorithm::trim(row[i]);
      notify_data(row);
    }

  } catch (std::exception &e) {
    std::cout.flush();
    cerr << "ERROR: " << e.what() << std::endl;
    throw e;
  }
}

template <typename data_declaration>
void read_binary(const std::string &fn, data_declaration notify_data,
                 std::string delimeter = ",; |\t") {
  using std::cerr;
  try {
    std::ifstream ifs(fn);

    std::vector<int> row;
    std::vector<std::string> liner;

    std::string line = "";

    // header
    getline(ifs, line);
    boost::algorithm::split(liner, line, boost::is_any_of(delimeter));

    // row.resize(liner.size());

    while (getline(ifs, line)) {

      boost::algorithm::split(liner, line, boost::is_any_of(delimeter));

      row.clear();
      for (auto v : liner)
        row.push_back(boost::lexical_cast<int>(v));

      notify_data(row);
    }

  } catch (std::exception &e) {
    std::cout.flush();
    cerr << "ERROR: " << e.what() << std::endl;
    throw e;
  }
}

} // namespace dimacs

#endif
