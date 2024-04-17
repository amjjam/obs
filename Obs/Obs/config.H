
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <vector>
#include <string>

class Config{
public:
  Config(){};
  Config(const std::string &filename);
  ~Config(){};
  bool find(const std::string &, std::vector<std::string> &) const;
  void print() const;
private:
  void load(const std::string &filename);
  std::vector<std::string> split(const std::string &,char) const;
  std::string before(const std::string &,char) const;
  // std::string after(std::string &,char);
  std::string compress(const std::string &) const;
  std::vector<std::string> key;
  std::vector<std::vector<std::string> > content;
};

#endif
