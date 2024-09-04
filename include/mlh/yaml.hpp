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

// reference: https://yaml.org/refcard.html
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
};

class Yaml {
public:
  Yaml() = default;

  static Yaml parse(std::ifstream &InputStream) {
    assert(InputStream.is_open());

    Yaml NewYaml = Yaml();
    std::string Container = "";

    while (true) {
      const char CurrChar = InputStream.get();

      if (CurrChar == EOF) {
        break;
      }

      Container += CurrChar;
    }

    if (validateString(Container)) {
      NewYaml.setContent(Container);
    } else {
      std::cerr << "YAML file is not valid!" << std::endl;
    }

    return NewYaml;
  }

  static bool validateString(const std::string &Str) {
    tokenizeString(Str);

    bool Valid = checkTokens();
    Tokens.clear();

    return Valid;
  }

  inline void setContent(const std::string &Str) { Content = Str; }

  inline std::string getContent() { return Content; }

private:
  std::string Content = "";

  static std::vector<YamlTokens> Tokens;

  static void tokenizeString(const std::string &Str) {
    // TODO:
  }

  static bool checkTokens() {
    // TODO:
    return false;
  }
};

} // namespace mlh

#endif
