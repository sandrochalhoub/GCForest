#ifndef __TXT_READER_HH
#define __TXT_READER_HH

#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>


namespace txt
{

// template<  >
template <typename data_declaration>
void read(const std::string &fn, 
          data_declaration notify_data, std::string delimeter = " ") {
  using std::cerr;
  try {
    std::ifstream ifs(fn);
    std::vector<std::string> row;
    std::string line = "";

    // Iterate through each line and split the content using delimeter
    while (getline(ifs, line)) {			
			if(line[0]=='@' or line[0]=='\n')
				continue;
			
			auto emptyline{true};
			for(auto c : line)
				if(c != ' ' or c != '\t' or c != '\n')
			{
				emptyline = false;
				break;
			}

			if(emptyline)
				continue;

			
      row.clear();
      boost::algorithm::split(row, line, boost::is_any_of(delimeter));
			
      notify_data(row);
    }

  } catch (std::exception &e) {
    std::cout.flush();
    cerr << "ERROR: " << e.what() << std::endl;
    exit(1);
  }
}

template <typename data_declaration>
void read_binary(const std::string &fn, data_declaration notify_data,
                 std::string delimeter = " ") {
  using std::cerr;
  try {
    std::ifstream ifs(fn);
    std::vector<std::string> string_row;
    std::string line = "";

    std::vector<int> row;

    // Iterate through each line and split the content using delimeter
    while (getline(ifs, line)) {
      if (line[0] == '@' or line[0] == '\n')
        continue;

      auto emptyline{true};
      for (auto c : line)
        if (c != ' ' or c != '\t' or c != '\n') {
          emptyline = false;
          break;
        }

      if (emptyline)
        continue;

      row.clear();
      boost::algorithm::split(string_row, line, boost::is_any_of(delimeter));

      for (auto r : string_row) {
        if (r == "1")
          row.push_back(1);
        else
          row.push_back(0);
        // cout << row.back() << " ";
      }

      // notify_data(row);
      break;
    }

    // cout << row.size() << endl;

    auto ok{true};
    while (ok) {
      notify_data(row);
      for (auto i{0}; i < row.size() and ok; ++i) {
        ifs >> row[i];
        if (not ifs.good()) {
          ok = false;
          // cout << "stop!\n";
        }
      }
    }

  } catch (std::exception &e) {
    std::cout.flush();
    cerr << "ERROR: " << e.what() << std::endl;
    exit(1);
  }
}

} // namespace dimacs

#endif
