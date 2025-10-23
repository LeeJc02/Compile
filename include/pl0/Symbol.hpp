#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace pl0 {

enum class TokenKind : std::uint16_t;

enum class Keyword : std::uint16_t {
  Begin,
  Call,
  Const,
  Do,
  Else,
  End,
  If,
  Odd,
  Procedure,
  Then,
  Var,
  While,
  Repeat,
  Until,
  Read,
  Write,
  Writeln,
  True,
  False,
  And,
  Or,
  Not,
};

const std::unordered_map<std::string_view, Keyword>& keyword_table();

std::optional<Keyword> lookup_keyword(std::string_view lexeme);

std::optional<TokenKind> keyword_token(Keyword keyword);

}  // namespace pl0
