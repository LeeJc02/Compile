#include "pl0/Token.hpp"

#include <array>
#include <stdexcept>

namespace pl0 {

std::string to_string(TokenKind kind) {
  switch (kind) {
    case TokenKind::EndOfFile:
      return "eof";
    case TokenKind::Identifier:
      return "identifier";
    case TokenKind::Number:
      return "number";
    case TokenKind::Boolean:
      return "boolean";
    case TokenKind::Plus:
      return "+";
    case TokenKind::Minus:
      return "-";
    case TokenKind::Star:
      return "*";
    case TokenKind::Slash:
      return "/";
    case TokenKind::Percent:
      return "%";
    case TokenKind::Assign:
      return ":=";
    case TokenKind::Equal:
      return "=";
    case TokenKind::NotEqual:
      return "#";
    case TokenKind::Less:
      return "<";
    case TokenKind::LessEqual:
      return "<=";
    case TokenKind::Greater:
      return ">";
    case TokenKind::GreaterEqual:
      return ">=";
    case TokenKind::LParen:
      return "(";
    case TokenKind::RParen:
      return ")";
    case TokenKind::LBracket:
      return "[";
    case TokenKind::RBracket:
      return "]";
    case TokenKind::Comma:
      return ",";
    case TokenKind::Semicolon:
      return ";";
    case TokenKind::Period:
      return ".";
    case TokenKind::Colon:
      return ":";
    case TokenKind::Begin:
      return "begin";
    case TokenKind::Call:
      return "call";
    case TokenKind::Const:
      return "const";
    case TokenKind::Do:
      return "do";
    case TokenKind::Else:
      return "else";
    case TokenKind::End:
      return "end";
    case TokenKind::If:
      return "if";
    case TokenKind::Odd:
      return "odd";
    case TokenKind::Procedure:
      return "procedure";
    case TokenKind::Then:
      return "then";
    case TokenKind::Var:
      return "var";
    case TokenKind::While:
      return "while";
    case TokenKind::Repeat:
      return "repeat";
    case TokenKind::Until:
      return "until";
    case TokenKind::Read:
      return "read";
    case TokenKind::Write:
      return "write";
    case TokenKind::Writeln:
      return "writeln";
    case TokenKind::True:
      return "true";
    case TokenKind::False:
      return "false";
    case TokenKind::And:
      return "and";
    case TokenKind::Or:
      return "or";
    case TokenKind::Not:
      return "not";
  }
  return "unknown";
}

}  // namespace pl0

