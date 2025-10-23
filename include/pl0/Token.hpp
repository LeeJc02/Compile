#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "pl0/Diagnostics.hpp"

namespace pl0 {

enum class TokenKind : std::uint16_t {
  EndOfFile,
  Identifier,
  Number,
  Boolean,
  Plus,
  Minus,
  Star,
  Slash,
  Percent,
  Assign,
  Equal,
  NotEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  LParen,
  RParen,
  LBracket,
  RBracket,
  Comma,
  Semicolon,
  Period,
  Colon,
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

struct Token {
  TokenKind kind = TokenKind::EndOfFile;
  std::string lexeme;
  SourceRange range;
  std::optional<std::int64_t> number;
  std::optional<bool> boolean;
};

std::string to_string(TokenKind kind);

}  // namespace pl0

