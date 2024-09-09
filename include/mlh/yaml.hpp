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
  // Language Independent Scalar types
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

  // '\n'  : Newline
  Newline,

  // ' '  : If at the start of a line and part of a list, it is an indentation
  //        (TODO: double check when whitespaces count as meaningful indentation)
  WhiteSpaceIndentation,

  // '\t'  : Tab characters are not allowed as part of whitespace indentation.
  //         Tab only indentation should be fine (TODO: double check)
  TabIndentation,
};



class Yaml {
public:
  Yaml() = default;

  static Yaml parse(std::ifstream &InputStream) {
    assert(InputStream.is_open());

    std::string Container = "";

    readStream(InputStream, Container);

    bool Valid = parseString(Container);

    if (!Valid) {
      std::cerr << "YAML file is not valid!" <<  std::endl;
    }

    Yaml NewYaml = Yaml();
    NewYaml.Content = Container;

    return NewYaml;
  }

  std::string Content = "";
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
    // Not yet implemented tokens:
    // - Primary  : '!foo'
    // Reason: If any string can be after the !, what ends the tag property?
    //         I'd assume a whitespace, but double check to be sure.
    // - Secondary  : '!!foo'
    // Reason: See Primary reason
    // - Requires  : '!h!foo'
    // Reason: See Primary reason
    // - VerbatimTag  : '!<foo>'
    // Reason: See Primary reason
    // - Int  : [ 1234, 0x4D2, 02333 ]
    // Reason: What ends an int? I'd assume a whitespace, but double check to be sure.
    // - Float  : [ 1_230.15, 12.3015e+02 ]
    // Reason: See Int
    // - InfOrNAN  : [ .inf, -.Inf, .NAN ]
    // Reason: Does any variation of the characters i, n and f count? Does iNF count?
    //         Also, like Int and such, what ends this tag?
    // - BooleanTrue  : { Y, true, Yes, ON  }
    // Reason: See InfOrNAN
    // - BooleanFalse  : { n, FALSE, No, off }
    // Reason: See InfOrNAN
    // - Base64BinaryValue
    // Reason: See InfOrNAN
    // - Numeric
    // Reason: See InfOrNAN
    // - Protective
    // Reason: See InfOrNAN
    // - C
    // Reason: See InfOrNAN
    // - Additional
    // Reason: See InfOrNAN

    // Removed tokens:
    // - ' #' : Throwaway comment indicator.
    // Reason: Given that comments will be ignored, this token would be useless
    //         A bool to ignore comment content is used instead.
    
    std::string PreviousChars = "";
    bool isComment = false;

    for (const char CurrChar : Str) {
      // End of comment
      if (isComment && CurrChar == '\n') {
        TokenContainer.push_back(YamlTokens::Newline);
        isComment = false;
        continue;
      }

      // Comment continuation
      if (isComment) {
        continue;
      }
      
      // Check for single char tokens
      if (PreviousChars.empty()) {
        if (CurrChar == '"') {
          TokenContainer.push_back(YamlTokens::SurroundInLineEscapedScalar);
          continue;
        }

        if (CurrChar == '|') {
          TokenContainer.push_back(YamlTokens::BlockScalarIndicator);
          continue;
        }

        if (CurrChar == '>') {
          TokenContainer.push_back(YamlTokens::FoldedScalarIndicator);
          continue;
        }

        if (CurrChar == '-') {
          TokenContainer.push_back(YamlTokens::StripChompModifier);
          continue;
        }

        if (CurrChar == '&') {
          TokenContainer.push_back(YamlTokens::AnchorProperty);
          continue;
        }

        if (CurrChar == '*') {
          TokenContainer.push_back(YamlTokens::AliasIndicator);
          continue;
        }

        if (CurrChar == '!') {
          TokenContainer.push_back(YamlTokens::NonSpecificTag);
          continue;
        }

        if (CurrChar == '%') {
          TokenContainer.push_back(YamlTokens::DirectiveIndicator);
          continue;
        }

        if (CurrChar == '=') {
          TokenContainer.push_back(YamlTokens::DefaultValueMappingKey);
          continue;
        }

        // Character behind the hashtag and the hashtag will be ignored
        // until the next newline character
        if (CurrChar == '#') {
          isComment = true;
          continue;
        }

        if (CurrChar == '\n') {
          TokenContainer.push_back(YamlTokens::Newline);
          continue;
        }

        if (CurrChar == ' ') {
          TokenContainer.push_back(YamlTokens::WhiteSpaceIndentation);
          continue;
        }

        if (CurrChar == '\t') {
          TokenContainer.push_back(YamlTokens::TabIndentation);
          continue;
        }

        if (CurrChar == '~') {
          TokenContainer.push_back(YamlTokens::Null);
          continue;
        }
      }

      if (PreviousChars == "?" && CurrChar == ' ') {
        TokenContainer.push_back(YamlTokens::KeyIndicator);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == ":" && CurrChar == ' ') {
        TokenContainer.push_back(YamlTokens::ValueIndicator);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "-" && CurrChar == ' ') {
        TokenContainer.push_back(YamlTokens::NestedSeriesEntryIndicator);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "," && CurrChar == ' ') {
        TokenContainer.push_back(YamlTokens::SeperateInLineBranchEntries);
        TokenContainer.clear();
        continue;
      }

      // TODO: check if this is correct, or if there could be content between the brackets
      if (PreviousChars == "[" && CurrChar == ']') {
        TokenContainer.push_back(YamlTokens::SurroundInLineSeriesBranch);
        TokenContainer.clear();
        continue;
      }

      // TODO: check if this is correct, or if there could be content between the brackets
      if (PreviousChars == "{" && CurrChar == '}') {
        TokenContainer.push_back(YamlTokens::SurroundInLineKeyedBranch);
        TokenContainer.clear();
        continue;
      }

      // TODO: check if this is correct, or if there could be content between the ''s
      if (PreviousChars == "'" && CurrChar == '\'') {
        TokenContainer.push_back(YamlTokens::SurroundInLineKeyedBranch);
        TokenContainer.clear();
        continue;
      }

      // TODO: check if -, + or 1-9 could be a modifier on their own.
      //       I found this to be unclear in the documentation at:
      //       https://yaml.org/refcard.html
      if (PreviousChars == "|" && CurrChar == '-' || PreviousChars == ">" && CurrChar == '-') {
        TokenContainer.push_back(YamlTokens::StripChompModifier);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "|" && CurrChar == '+' || PreviousChars == ">" && CurrChar == '+') {
        TokenContainer.push_back(YamlTokens::KeepChompModifier);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "|" && (CurrChar == '1' || CurrChar == '2' || CurrChar == '3'
                                || CurrChar == '4' || CurrChar == '5' || CurrChar == '6'
                                || CurrChar == '7' || CurrChar == '8' || CurrChar == '9')
       || PreviousChars == ">" && (CurrChar == '1' || CurrChar == '2' || CurrChar == '3'
                                || CurrChar == '4' || CurrChar == '5' || CurrChar == '6'
                                || CurrChar == '7' || CurrChar == '8' || CurrChar == '9')) {
        TokenContainer.push_back(YamlTokens::ExplicitIndentationModifier);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "non" && CurrChar == 'e') {
        TokenContainer.push_back(YamlTokens::UnspecifiedTag);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == ".." && CurrChar == '.') {
        TokenContainer.push_back(YamlTokens::DocumentTerminator);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "`" && CurrChar == '@') {
        TokenContainer.push_back(YamlTokens::BothReservedForFutureUse);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "<" && CurrChar == '<') {
        TokenContainer.push_back(YamlTokens::MergeKeysFromAnotherMapping);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "!!ma" && CurrChar == 'p') {
        TokenContainer.push_back(YamlTokens::Map);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "!!se" && CurrChar == 'q') {
        TokenContainer.push_back(YamlTokens::Seq);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "!!st" && CurrChar == 'r') {
        TokenContainer.push_back(YamlTokens::UnicodeString);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "!!se" && CurrChar == 't') {
        TokenContainer.push_back(YamlTokens::Set);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "!!oma" && CurrChar == 'p') {
        TokenContainer.push_back(YamlTokens::OMap);
        TokenContainer.clear();
        continue;
      }

      if (PreviousChars == "nul" && CurrChar == 'l') {
        TokenContainer.push_back(YamlTokens::Null);
        TokenContainer.clear();
        continue;
      }

      PreviousChars.push_back(CurrChar);
    }
  }

static bool isValid(const std::vector<YamlTokens> &Tokens) {
    bool IsValid = true;
    bool TrackIndentation = true;
    size_t WhiteSpaceIndents = 0;
    size_t TabIndents = 0;

    // TODO: add the rest of the invalid yaml conditions
    for (const auto &Token : Tokens) {
      if (!IsValid) {
        break;
      }

      switch (Token) {
      case YamlTokens::ValueIndicator:
        WhiteSpaceIndents = 0;
        TabIndents = 0;
        TrackIndentation = true;
        break;
      case YamlTokens::WhiteSpaceIndentation:
        WhiteSpaceIndents++;
      case YamlTokens::TabIndentation:
        if (WhiteSpaceIndents) {
          IsValid = false;
        }
        break;
      case YamlTokens::KeyIndicator:
      case YamlTokens::NestedSeriesEntryIndicator:
      case YamlTokens::SeperateInLineBranchEntries:
      case YamlTokens::SurroundInLineSeriesBranch:
      case YamlTokens::SurroundInLineKeyedBranch:
      case YamlTokens::SurroundInLineUnescapedScalar:
      case YamlTokens::SurroundInLineEscapedScalar:
      case YamlTokens::BlockScalarIndicator:
      case YamlTokens::FoldedScalarIndicator:
      case YamlTokens::StripChompModifier:
      case YamlTokens::KeepChompModifier:
      case YamlTokens::ExplicitIndentationModifier:
      case YamlTokens::AnchorProperty:
      case YamlTokens::AliasIndicator:
      case YamlTokens::UnspecifiedTag:
      case YamlTokens::NonSpecificTag:
      case YamlTokens::Primary:
      case YamlTokens::Secondary:
      case YamlTokens::Requires:
      case YamlTokens::VerbatimTag:
      case YamlTokens::DirectiveIndicator:
      case YamlTokens::DocumentHeader:
      case YamlTokens::DocumentTerminator:
      case YamlTokens::BothReservedForFutureUse:
      case YamlTokens::DefaultValueMappingKey:
      case YamlTokens::MergeKeysFromAnotherMapping:
      case YamlTokens::Map:
      case YamlTokens::Seq:
      case YamlTokens::UnicodeString:
      case YamlTokens::Set:
      case YamlTokens::OMap:
      case YamlTokens::Null:
      case YamlTokens::Int:
      case YamlTokens::Float:
      case YamlTokens::InfOrNAN:
      case YamlTokens::BooleanTrue:
      case YamlTokens::BooleanFalse:
      case YamlTokens::Base64BinaryValue:
      case YamlTokens::Numeric:
      case YamlTokens::Protective:
      case YamlTokens::C:
      case YamlTokens::Additional:
      case YamlTokens::Newline:
      default:
        break;
      }
    }

    return IsValid;
  }

  static bool parseString(const std::string &Str) {
    std::vector<std::string> Documents(getNumberOfDocumentHeaders(Str));
    splitStringByDocumentHeaders(Str, Documents);

    bool Valid = true;

    size_t Counter = 0;
    for (const auto &Doc : Documents) {
      std::vector<YamlTokens> Tokens;
      tokenizeDocument(Doc, Tokens);

      if (!isValid(Tokens)) {
        std::cerr << "YAML document " << Counter << " is not valid!" <<  std::endl;
        Valid = false;
      }
      Counter++;
    }

    return Valid;
  }
};

} // namespace mlh

#endif
