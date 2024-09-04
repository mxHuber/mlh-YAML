#include "../include/mlh/yaml.hpp"

int main() {
  std::ifstream FileStream("test/test_file_1.yaml");

  mlh::Yaml TestYaml = mlh::Yaml::parse(FileStream);

  return 0;
}