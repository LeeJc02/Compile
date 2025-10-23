#include "pl0/Lexer.hpp"

#include <cctype>
#include <charconv>
#include <system_error>

#include "pl0/Symbol.hpp"
#include "pl0/Token.hpp"

namespace pl0 {

namespace {

bool is_identifier_start(char ch) {
  return std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

bool is_identifier_part(char ch) {
  return std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_';
}

}  // namespace

Lexer::Lexer(std::string source, DiagnosticSink& diagnostics)
    : source_(std::move(source)), diagnostics_(diagnostics) {}

const Token& Lexer::peek(std::size_t lookahead) {
  if (!buffer_valid_) {
    buffer_.clear();
    buffer_valid_ = true;
  }
  while (buffer_.size() <= lookahead) {
    const auto start = location_;
    skip_whitespace_and_comments();
    token_start_ = location_;

    if (is_end()) {
      buffer_.push_back(make_token(TokenKind::EndOfFile, "", start, location_));
      break;
    }

    char ch = current();
    Token token;
    if (std::isdigit(static_cast<unsigned char>(ch)) != 0) {
      token = lex_number(start);
    } else if (is_identifier_start(ch)) {
      token = lex_identifier_or_keyword(start);
    } else {
      token = lex_symbol(start);
    }
    buffer_.push_back(std::move(token));
  }
  return buffer_[lookahead];
}

Token Lexer::next() {
  const Token& token = peek(0);
  Token result = token;
  if (!buffer_.empty()) {
    buffer_.erase(buffer_.begin());
  }
  return result;
}

void Lexer::reset() {
  index_ = 0;
  location_ = SourceLoc{1, 1};
  token_start_ = location_;
  buffer_.clear();
  buffer_valid_ = false;
}

Token Lexer::make_token(TokenKind kind, std::string_view lexeme,
                        SourceLoc start, SourceLoc end) {
  Token token;
  token.kind = kind;
  token.lexeme = std::string(lexeme);
  token.range = SourceRange{start, end};
  return token;
}

void Lexer::skip_whitespace_and_comments() {
  bool advancing = true;
  while (advancing) {
    advancing = false;
    while (!is_end()) {
      char ch = current();
      if (ch == ' ' || ch == '\t' || ch == '\r') {
        advance();
        advancing = true;
        continue;
      }
      if (ch == '\n') {
        advance();
        advancing = true;
        continue;
      }
      break;
    }

    if (is_end()) {
      break;
    }

    if (current() == '/') {
      if (peek_char(1) == '/') {
        // Line comment
        advance();
        advance();
        while (!is_end() && current() != '\n') {
          advance();
        }
        advancing = true;
        continue;
      }
      if (peek_char(1) == '*') {
        // Block comment
        advance();
        advance();
        SourceLoc start = location_;
        bool terminated = false;
        while (!is_end()) {
          if (current() == '*' && peek_char(1) == '/') {
            advance();
            advance();
            terminated = true;
            break;
          }
          advance();
        }
        if (!terminated) {
          report_unterminated_comment(start);
        }
        advancing = true;
        continue;
      }
    }
  }
}

Token Lexer::lex_number(SourceLoc start) {
  std::size_t begin = index_;
  while (!is_end() &&
         std::isdigit(static_cast<unsigned char>(current())) != 0) {
    advance();
  }
  std::string_view text(source_.data() + begin, index_ - begin);
  std::int64_t value = 0;
  auto result =
      std::from_chars(text.begin(), text.end(), value, 10);
  if (result.ec != std::errc()) {
    diagnostics_.report(
        {DiagnosticLevel::Error, DiagnosticCode::InvalidNumber,
         "invalid integer literal", {start, location_}});
  }
  Token token = make_token(TokenKind::Number, text, start, location_);
  token.number = value;
  return token;
}

Token Lexer::lex_identifier_or_keyword(SourceLoc start) {
  std::size_t begin = index_;
  advance();
  while (!is_end() && is_identifier_part(current())) {
    advance();
  }
  std::string_view text(source_.data() + begin, index_ - begin);
  auto lower = std::string(text);
  for (auto& c : lower) {
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
  }
  if (auto keyword = lookup_keyword(lower)) {
    Token token = make_token(TokenKind::Identifier, text, start, location_);
    if (*keyword == Keyword::True) {
      token.kind = TokenKind::Boolean;
      token.boolean = true;
    } else if (*keyword == Keyword::False) {
      token.kind = TokenKind::Boolean;
      token.boolean = false;
    } else {
      if (auto token_kind = keyword_token(*keyword)) {
        token.kind = *token_kind;
      }
    }
    return token;
  }
  Token token = make_token(TokenKind::Identifier, text, start, location_);
  return token;
}

Token Lexer::lex_symbol(SourceLoc start) {
  char ch = current();
  advance();
  switch (ch) {
    case '+':
      return make_token(TokenKind::Plus, "+", start, location_);
    case '-':
      return make_token(TokenKind::Minus, "-", start, location_);
    case '*':
      return make_token(TokenKind::Star, "*", start, location_);
    case '/':
      return make_token(TokenKind::Slash, "/", start, location_);
    case '%':
      return make_token(TokenKind::Percent, "%", start, location_);
    case '(':
      return make_token(TokenKind::LParen, "(", start, location_);
    case ')':
      return make_token(TokenKind::RParen, ")", start, location_);
    case '[':
      return make_token(TokenKind::LBracket, "[", start, location_);
    case ']':
      return make_token(TokenKind::RBracket, "]", start, location_);
    case ',':
      return make_token(TokenKind::Comma, ",", start, location_);
    case ';':
      return make_token(TokenKind::Semicolon, ";", start, location_);
    case '.':
      return make_token(TokenKind::Period, ".", start, location_);
    case ':':
      if (current() == '=') {
        advance();
        return make_token(TokenKind::Assign, ":=", start, location_);
      }
      return make_token(TokenKind::Colon, ":", start, location_);
    case '=':
      return make_token(TokenKind::Equal, "=", start, location_);
    case '#':
      return make_token(TokenKind::NotEqual, "#", start, location_);
    case '!':
      if (current() == '=') {
        advance();
        return make_token(TokenKind::NotEqual, "!=", start, location_);
      }
      diagnostics_.report({DiagnosticLevel::Error,
                           DiagnosticCode::UnexpectedToken,
                           "unexpected '!'", {start, location_}});
      break;
    case '<':
      if (current() == '=') {
        advance();
        return make_token(TokenKind::LessEqual, "<=", start, location_);
      }
      if (current() == '>') {
        advance();
        return make_token(TokenKind::NotEqual, "<>", start, location_);
      }
      return make_token(TokenKind::Less, "<", start, location_);
    case '>':
      if (current() == '=') {
        advance();
        return make_token(TokenKind::GreaterEqual, ">=", start, location_);
      }
      return make_token(TokenKind::Greater, ">", start, location_);
    default:
      diagnostics_.report({DiagnosticLevel::Error,
                           DiagnosticCode::UnexpectedToken,
                           std::string("unexpected character '") + ch + "'",
                           {start, location_}});
      break;
  }
  return make_token(TokenKind::EndOfFile, "", start, location_);
}

char Lexer::current() const {
  if (is_end()) {
    return '\0';
  }
  return source_[index_];
}

char Lexer::peek_char(std::size_t offset) const {
  if (index_ + offset >= source_.size()) {
    return '\0';
  }
  return source_[index_ + offset];
}

char Lexer::advance() {
  if (is_end()) {
    return '\0';
  }
  char ch = source_[index_++];
  if (ch == '\n') {
    location_.line += 1;
    location_.column = 1;
  } else {
    location_.column += 1;
  }
  return ch;
}

bool Lexer::is_end() const {
  return index_ >= source_.size();
}

void Lexer::report_unterminated_comment(SourceLoc start) {
  diagnostics_.report(
      {DiagnosticLevel::Error, DiagnosticCode::UnterminatedComment,
       "unterminated block comment", {start, location_}});
}

}  // namespace pl0
