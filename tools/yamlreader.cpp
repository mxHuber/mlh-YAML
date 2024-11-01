#include "../include/mlh/yaml.hpp"
#include <fstream>
#include <iostream>

int main() {
  std::ifstream FileStream("../test/test_file_1.yaml");

  mlh::Yaml TestYaml = mlh::Yaml::parse(FileStream);

  std::cout << TestYaml.Content << std::endl;

  return 0;
}
