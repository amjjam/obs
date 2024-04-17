#include "config.H"

#include <iostream>
#include <fstream>
#include <cctype>

Config::Config(const std::string &filename){
  load(filename);  
}

bool Config::find(const std::string &k, std::vector<std::string> &c) const{
  for(unsigned int i=0;i<key.size();i++)
    if(key[i]==k){
      c=content[i];
      return true;
    }
  return false;
}

void Config::print() const{
  for(unsigned int i=0;i<key.size();i++){
    std::cout << key[i] << "=";
    for(unsigned int j=0;j<content[i].size();j++){
      if(j>0)
	std::cout << "/";
      std::cout << content[i][j];
    }
    std::cout << std::endl;
  }
}

void Config::load(const std::string &filename){
  std::fstream file;
  file.open(filename,std::ios::in);
  if(!file.is_open()){
    std::cout << "Config: unable to open file: " << filename << std::endl;
    abort();
  }
  std::string line;
  int n=0;
  std::string s;
  std::vector<std::string> ss;
  while(getline(file,line)){
    s=compress(before(line,'#'));
    n++;
    if(s.length()==0)
      continue;
    ss=split(s,'=');
    if(ss.size()==1){
      std::cout << "Config: error: not '=' on line " << n << ": "
		<< line << std::endl;
      abort();
    }
    else if(ss.size()>2){
      std::cout << "Config: error: multiple '=' on line " << n << ": "
		<< line << std::endl;
      abort();
    }

    key.push_back(compress(ss[0]));
    content.push_back(split(compress(ss[1]),' '));
  }
  
  file.close();
}

std::vector<std::string> Config::split(const std::string &input, char delimiter) const{
  std::vector<std::string> substrings;
  size_t start = 0;
  size_t end = input.find(delimiter);
  
  while (end != std::string::npos) {
    substrings.push_back(input.substr(start, end - start));
    start = end + 1;
    end = input.find(delimiter, start);
  }
  
  // Push the substring after the last delimiter (or the entire string if no delimiter found)
  substrings.push_back(input.substr(start));
  
  return substrings;
}

std::string Config::before(const std::string &s, char c) const{
  return s.substr(0,s.find(c));
}

// std::string Confiig::after(std::string &s, char c){
  
// }

std::string Config::compress(const std::string &input) const{
  std::string result;
  bool spaceFound = false;
  
  // Iterate through each character in the input string
  for (char c : input) {
    if (std::isspace(c)) {
      if (!spaceFound) {
	// If the current character is a space and it's the first space encountered,
	// add it to the result string.
	result.push_back(' ');
	spaceFound = true;
      }
    } else {
      // If the current character is not a space, add it to the result string.
      result.push_back(c);
      spaceFound = false;
    }
  }
  
  // Trim leading and trailing spaces
  size_t start = result.find_first_not_of(" ");
  size_t end = result.find_last_not_of(" ");
  if (start != std::string::npos && end != std::string::npos)
    result = result.substr(start, end - start + 1);
  else
    result.clear();
  
  return result;
}
