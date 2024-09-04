//
//  \ /  /\   |\  /| |    YAML for Modern C++
//   |  /__\  | \/ | |    version 0.0.1
//   | /    \ |    | |___ TODO: add github link
//

#ifndef INCLUDE_MLH_YAML_HPP
#define INCLUDE_MLH_YAML_HPP

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace mlh {

// Reference: https://yaml.org/refcard.html
// %YAML 1.1   # Reference card
enum class YamlTokens {
  //
  // Collection indicators:
  //

  // '? '
  KeyIndicator,

  // ': '
  ValueIndicator,

  // '- '
  NestedSeriesEntryIndicator,

  // ', '
  SeperateInLineBranchEntries,

  // '[]'
  SurroundInLineSeriesBranch,

  // '{}'
  SurroundInLineKeyedBranch,

  //
  // Scalar indicators:
  //

  // ''''
  SurroundInLineUnescapedScalar,

  // '"'
  SurroundInLineEscapedScalar,

  // '|'
  BlockScalarIndicator,

  // '>'
  FoldedScalarIndicator,

  // '-'  : ('|-' or '>-')
  StripChompModifier,

  // '+'  : ('|+' or '>+')
  KeepChompModifier,

  // 1-9  : ('|1' or '>2')
  //      # Modifiers can be combined ('|2-', '>+1')
  ExplicitIndentationModifier,

  //
  // Alias indicators:
  //

  // '&'
  AnchorProperty,

  // '*'
  AliasIndicator,

  //
  // Tag property: # Usually unspecified
  //

  // none
  UnspecifiedTag,

  // '!'
  NonSpecificTag,

  // '!foo'
  Primary,

  // '!!foo'
  Secondary,
  
  // '!h!foo'
  Requires,
  
  // '!<foo>'
  VerbatimTag,

  //
  // Document indicators:
  //

  // '%'
  DirectiveIndicator,

  // '---'
  DocumentHeader,

  // '...'
  DocumentTerminator,

  //
  // Misc indicators:
  //

  // ' #'
  ThrowAwayCommentIndicator,

  // '`@'
  BothReservedForFutureUse,

  //
  // Special keys:
  //

  // '='
  DefaultValueMappingKey,

  // '<<'
  MergeKeysFromAnotherMapping,

  //
  // Core types:
  //

  // '!!map'
  Map,

  // '!!seq'
  Seq,

  // '!!str'
  UnicodeString,

  //
  // More types:
  //

  // '!!set'
  Set,

  // '!!omap'
  OMap,

  //
  // Language Indipendent Scalar types
  //

  // { ~, null }
  Null,

  // [ 1234, 0x4D2, 02333 ]
  Int,

  // [ 1_230.15, 12.3015e+02 ]
  Float,

  // [ .inf, -.Inf, .NAN ]
  InfOrNAN,

  // { Y, true, Yes, ON  }
  BooleanTrue,

  // { n, FALSE, No, off }
  BooleanFalse,

  // ? !!binary >
  //     R0lG...BADS=
  // : >-
  Base64BinaryValue,

  //
  // Escape codes:
  //

  // { "\x12": 8-bit, "\u1234": 16-bit, "\U00102030": 32-bit }
  Numeric,

  // { "\\": '\', "\"": '"', "\ ": ' ', "\<TAB>": TAB }
  Protective,

  // { "\0": NUL, "\a": BEL, "\b": BS, "\f": FF, "\n": LF, "\r": CR,
  //   "\t": TAB, "\v": VTAB }
  C,

  // { "\e": ESC, "\_": NBSP, "\N": NEL, "\L": LS, "\P": PS }
  Additional,

  //
  // Additional tokens:
  //

  // '#'  : Comment, that starts with the # character anywhere 
  //        on a line and ends with the end of the line
  Comment,

  // ' '  : If at the start of a line and part of a list, it is an indentation
  //        (TODO: double check when whitespaces count as meaningful indentation)
  WhiteSpaceIndentation,

  // '\t'  : Tab characters are not allowed as part of whitespace indentation.
  //         Tab only indentation should be fine (TODO: double check)
  TabIndentation,
};

enum class ASTElements {
  // TODO:
};

class YamlAST {
public:
  YamlAST(std::vector<YamlTokens>);
  bool isValid();
  void print();
private:
  // TODO: implement Abstract Syntax Tree
};

class Yaml {
public:
  Yaml() = default;

  static Yaml parse(std::ifstream &InputStream, bool PrintASTs = false) {
    assert(InputStream.is_open());

    std::string Container = "";

    readStream(InputStream, Container);

    bool Valid = parseString(Container, PrintASTs);

    if (!Valid) {
      std::cerr << "YAML file is not valid!" <<  std::endl;
    }

    Yaml NewYaml = Yaml();
    NewYaml.setContent(Container);

    return NewYaml;
  }

  inline void setContent(const std::string &Str) { Content = Str; }

  inline std::string getContent() { return Content; }

private:
  std::string Content = "";
 
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

  static void getIndicesOfNewDocuments(const std::string &Str, std::vector<size_t> &Container) {
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

  static void splitStringByDocumentHeaders(const std::string &Str, std::vector<std::string> &Container) {
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

  static void tokenizeDocument(const std::string &Str, std::vector<YamlTokens> &TokenContainer) {

  }

  static bool parseString(const std::string &Str, bool PrintASTs) {
    std::vector<std::string> Documents(getNumberOfDocumentHeaders(Str));
    splitStringByDocumentHeaders(Str, Documents);

    bool Valid = true;

    for (const auto &Doc : Documents) {
      std::vector<YamlTokens> Tokens;
      tokenizeDocument(Doc, Tokens);

      YamlAST CurrDocAST(Tokens);
      if (!CurrDocAST.isValid()) {
        Valid = false;
      }

      if (PrintASTs) {
        CurrDocAST.print();
      }
    }

    return Valid;
  }
};

} // namespace mlh

#endif
