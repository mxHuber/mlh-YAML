//
//  \ /  /\   |\  /| |    YAML 1.2.2 for Modern C++
//   |  /__\  | \/ | |    version 0.0.1
//   | /    \ |    | |___ https://github.com/mxHuber/mlh-YAML
//

#ifndef INCLUDE_MLH_YAML_HPP
#define INCLUDE_MLH_YAML_HPP

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace mlh {

class Yaml {
public:
  Yaml() = default;

  static Yaml parse(std::ifstream &InputStream) {
    assert(InputStream.is_open());

    std::string Container = "";
    Yaml NewYaml = Yaml();

    readStream(InputStream, Container);
    parseString(Container, NewYaml.Documents);

    return std::move(NewYaml);
  }

  //
  // 2.1 Collections
  //

  struct Data {
    Data() = default;
    Data(const std::string &Str) {
      std::string PreviousChars = "";
      bool isComment = false;

      for (const char CurrChar : Str) {
        // End of comment
        if (isComment && CurrChar == '\n') {
          isComment = false;
          continue;
        }

        // Comment continuation
        if (isComment) {
          continue;
        }

        // Check for single char tokens
        if (PreviousChars.empty()) {
          PreviousChars += CurrChar;
          continue;
        }

        if (PreviousChars == "-" && CurrChar == ' ') {
          SubData.push_back(std::unique_ptr<Data>(new Data(Str)));
        }

        PreviousChars += CurrChar;
      }
    }
    std::vector<std::unique_ptr<Data>> SubData;
  };

  template <typename V> struct Value : public Data {
    Value() = default;
  };

  template <typename V> struct ValueVector : public Data {
    std::vector<Value<V>> Values;
  };

  struct Document {
    Document() = default;
    Document(const std::string &Str)
        : DocumentData(std::unique_ptr<Data>(new Data(Str))){};
    std::unique_ptr<Data> DocumentData = nullptr;
  };

  //
  // 2.2 Structures
  //

  // TODO:

  //
  // Yaml member variables
  //

  std::string Content = "";
  // One yaml file can contain multiple documents
  std::vector<Document> Documents{};

private:
  //
  // Functions for parsing
  //

  static void readStream(std::ifstream &InputStream, std::string &Container) {
    while (true) {
      const char CurrChar = InputStream.get();

      if (CurrChar == EOF) {
        break;
      }

      Container += CurrChar;
    }
  }

  static size_t getNumberOfDocumentHeaders(const std::string &Str) {
    size_t NumberOfDocumentHeaders = 0;
    size_t MinusCounter = 0;

    for (const char CurrChar : Str) {
      if (CurrChar == '-') {
        MinusCounter++;
      }
      if (MinusCounter == 2) {
        NumberOfDocumentHeaders++;
        MinusCounter = 0;
      }
    }

    return NumberOfDocumentHeaders;
  }

  static void getIndicesOfNewDocuments(const std::string &Str,
                                       std::vector<size_t> &Container) {
    size_t MinusCounter = 0;
    size_t Index = 0;

    for (const char CurrChar : Str) {
      if (CurrChar == '-') {
        MinusCounter++;
      }
      if (MinusCounter == 2) {
        MinusCounter = 0;
        Container.push_back(Index - 2);
      }

      Index++;
    }
  }

  static void
  splitStringByDocumentHeaders(const std::string &Str,
                               std::vector<std::string> &Container) {
    std::vector<size_t> DocumentHeaderIndices;
    getIndicesOfNewDocuments(Str, DocumentHeaderIndices);

    size_t Index = 0;
    size_t DocumentIndex = 0;
    for (char CurrChar : Str) {
      Container[DocumentIndex] += CurrChar;

      Index++;
      if (Index == DocumentHeaderIndices[DocumentIndex]) {
        DocumentIndex++;
      }
    }
  }

  static void parseString(const std::string &Str,
                          std::vector<Document> &Documents) {
    std::vector<std::string> DocumentStrings(getNumberOfDocumentHeaders(Str));
    splitStringByDocumentHeaders(Str, DocumentStrings);

    for (const auto &DocStr : DocumentStrings) {
      Documents.push_back(Document(DocStr));
    }
  }
};

} // namespace mlh

#endif
