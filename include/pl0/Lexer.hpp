#pragma once

#include <string>
#include <vector>

#include "pl0/Diagnostics.hpp"
#include "pl0/Token.hpp"

namespace pl0 {

class Lexer {
 public:
  Lexer(std::string source, DiagnosticSink& diagnostics);

  [[nodiscard]] const Token& peek(std::size_t lookahead = 0);
  Token next();

  void reset();

 private:
  Token make_token(TokenKind kind, std::string_view lexeme,
                   SourceLoc start, SourceLoc end);

  void skip_whitespace_and_comments();

  Token lex_number(SourceLoc start);
  Token lex_identifier_or_keyword(SourceLoc start);
  Token lex_symbol(SourceLoc start);

  char current() const;
  char peek_char(std::size_t offset) const;
  char advance();
  bool is_end() const;

  void report_unterminated_comment(SourceLoc start);

  std::string source_;
  DiagnosticSink& diagnostics_;
  std::size_t index_ = 0;
  SourceLoc location_{1, 1};
  SourceLoc token_start_{1, 1};
  std::vector<Token> buffer_;
  bool buffer_valid_ = false;
};

}  // namespace pl0

