#ifndef __TXT_READER_HH
#define __TXT_READER_HH

#include <fstream>
#include <sstream>

#include <boost/algorithm/string.hpp>


namespace txt
{

// template<  >
template <typename data_declaration>
void read(const std::string &fn, data_declaration notify_data,
          std::string delimeter = "\t ") {
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

                        int start = 0;
                        while (line[start] == ' ')
                          ++start;

                        if (start > 0)
                          line = line.substr(start, line.size());

                        row.clear();
                        boost::algorithm::split(row, line,
                                                boost::is_any_of(delimeter),
                                                boost::token_compress_on);

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
                 std::string delimeter = " \n") {
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
			for(auto c : line) {
				if(c == '0')
					row.push_back(0);
				else if(c == '1')
					row.push_back(1);
			}
      notify_data(row);
    }

  } catch (std::exception &e) {
    std::cout.flush();
    cerr << "ERROR: " << e.what() << std::endl;
    exit(1);
  }
}

} // namespace dimacs

#endif
