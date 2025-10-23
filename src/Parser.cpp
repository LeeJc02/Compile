#include "pl0/Parser.hpp"

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <utility>

#include "pl0/Symbol.hpp"
#include "pl0/Token.hpp"

namespace pl0 {

namespace {

StmtPtr make_statement(SourceRange range, auto value) {
  auto stmt = std::make_unique<Statement>();
  stmt->range = range;
  stmt->value = std::move(value);
  return stmt;
}

ExprPtr make_expression(SourceRange range, auto value) {
  auto expr = std::make_unique<Expression>();
  expr->range = range;
  expr->value = std::move(value);
  return expr;
}

}  // namespace

Parser::Parser(Lexer& lexer, DiagnosticSink& diagnostics)
    : lexer_(lexer), diagnostics_(diagnostics) {}

std::unique_ptr<Program> Parser::parse_program() {
  auto program = std::make_unique<Program>();
  auto block = parse_block();
  if (!block) {
    return nullptr;
  }
  program->block = std::move(*block);
  expect(TokenKind::Period, DiagnosticCode::ExpectedSymbol,
         "expected '.' at end of program");
  return program;
}

const Token& Parser::peek(std::size_t lookahead) {
  return lexer_.peek(lookahead);
}

bool Parser::match(TokenKind kind) {
  if (peek(0).kind == kind) {
    lexer_.next();
    panic_mode_ = false;
    return true;
  }
  return false;
}

Token Parser::expect(TokenKind kind, DiagnosticCode code,
                     std::string_view message) {
  const Token& token = peek(0);
  if (token.kind != kind) {
    diagnostics_.report({DiagnosticLevel::Error, code, std::string(message),
                         token.range});
    panic_mode_ = true;
    return lexer_.next();
  }
  panic_mode_ = false;
  return lexer_.next();
}

void Parser::synchronize(const std::vector<TokenKind>& sync_tokens) {
  if (!panic_mode_) {
    return;
  }
  auto is_sync = [&](TokenKind kind) {
    return std::find(sync_tokens.begin(), sync_tokens.end(), kind) !=
           sync_tokens.end();
  };
  while (peek(0).kind != TokenKind::EndOfFile && !is_sync(peek(0).kind)) {
    lexer_.next();
  }
  panic_mode_ = false;
}

std::unique_ptr<Block> Parser::parse_block() {
  auto block = std::make_unique<Block>();
  parse_const_declarations(*block);
  parse_var_declarations(*block);
  parse_procedure_declarations(*block);

  if (auto stmt = parse_statement()) {
    block->statements.push_back(std::move(stmt));
  }
  return block;
}

void Parser::parse_const_declarations(Block& block) {
  if (!match(TokenKind::Const)) {
    return;
  }
  while (true) {
    auto name_token = expect(TokenKind::Identifier,
                             DiagnosticCode::ExpectedIdentifier,
                             "expected identifier in const declaration");
    auto equals_token =
        expect(TokenKind::Equal, DiagnosticCode::ExpectedSymbol,
               "expected '=' in const declaration");
    auto value_token = peek(0);
    ConstDecl decl;
    decl.range.begin = name_token.range.begin;
    decl.name = name_token.lexeme;
    if (match(TokenKind::Number)) {
      decl.value = *value_token.number;
      decl.range.end = value_token.range.end;
    } else if (match(TokenKind::Boolean)) {
      decl.value = value_token.boolean.value() ? 1 : 0;
      decl.range.end = value_token.range.end;
    } else {
      diagnostics_.report({DiagnosticLevel::Error,
                           DiagnosticCode::ExpectedSymbol,
                           "expected number or boolean literal in const declaration",
                           value_token.range});
    }
    block.consts.push_back(std::move(decl));

    if (!match(TokenKind::Comma)) {
      break;
    }
  }
  expect(TokenKind::Semicolon, DiagnosticCode::ExpectedSymbol,
         "expected ';' after const declarations");
}

void Parser::parse_var_declarations(Block& block) {
  if (!match(TokenKind::Var)) {
    return;
  }
  while (true) {
    auto name_token = expect(TokenKind::Identifier,
                             DiagnosticCode::ExpectedIdentifier,
                             "expected identifier in var declaration");
    VarDecl decl;
    decl.range.begin = name_token.range.begin;
    decl.name = name_token.lexeme;
    if (match(TokenKind::LBracket)) {
      auto size_token = expect(TokenKind::Number, DiagnosticCode::ExpectedSymbol,
                               "expected array size");
      if (size_token.number && *size_token.number <= 0) {
        diagnostics_.report({DiagnosticLevel::Error,
                             DiagnosticCode::InvalidArraySubscript,
                             "array size must be positive", size_token.range});
      }
      decl.array_size = size_token.number.value_or(0);
      decl.range.end = size_token.range.end;
      expect(TokenKind::RBracket, DiagnosticCode::ExpectedSymbol,
             "expected ']' after array size");
    } else {
      decl.range.end = name_token.range.end;
    }
    block.vars.push_back(std::move(decl));
    if (!match(TokenKind::Comma)) {
      break;
    }
  }
  expect(TokenKind::Semicolon, DiagnosticCode::ExpectedSymbol,
         "expected ';' after var declarations");
}

void Parser::parse_procedure_declarations(Block& block) {
  while (match(TokenKind::Procedure)) {
    auto proc_token = expect(TokenKind::Identifier,
                             DiagnosticCode::ExpectedIdentifier,
                             "expected procedure name");
    ProcedureDecl decl;
    decl.range.begin = proc_token.range.begin;
    decl.name = proc_token.lexeme;
    expect(TokenKind::Semicolon, DiagnosticCode::ExpectedSymbol,
           "expected ';' before procedure body");
    auto body = parse_block();
    if (!body) {
      body = std::make_unique<Block>();
    }
    decl.body = std::move(body);
    decl.range.end = peek(0).range.begin;
    expect(TokenKind::Semicolon, DiagnosticCode::ExpectedSymbol,
           "expected ';' after procedure body");
    block.procedures.push_back(std::move(decl));
  }
}

StmtPtr Parser::parse_statement() {
  const auto token = peek(0);
  switch (token.kind) {
    case TokenKind::Identifier:
      return parse_assignment();
    case TokenKind::Call:
      return parse_call();
    case TokenKind::Begin:
      return parse_begin_end();
    case TokenKind::If:
      return parse_if();
    case TokenKind::While:
      return parse_while();
    case TokenKind::Repeat:
      return parse_repeat();
    case TokenKind::Read:
      return parse_read();
    case TokenKind::Write:
      return parse_write(false);
    case TokenKind::Writeln:
      return parse_write(true);
    default:
      return nullptr;
  }
}

StmtPtr Parser::parse_assignment() {
  auto identifier = expect(TokenKind::Identifier,
                           DiagnosticCode::ExpectedIdentifier,
                           "expected assignment target");
  AssignmentStmt stmt;
  stmt.target = identifier.lexeme;
  stmt.index = nullptr;
  SourceLoc begin = identifier.range.begin;
  if (match(TokenKind::LBracket)) {
    auto index_expr = parse_expression();
    if (!index_expr) {
      index_expr = make_expression(identifier.range, NumberLiteral{0});
    }
    stmt.index = std::move(index_expr);
    expect(TokenKind::RBracket, DiagnosticCode::ExpectedSymbol,
           "expected ']' after subscript");
  }
  expect(TokenKind::Assign, DiagnosticCode::ExpectedSymbol,
         "expected ':=' in assignment");
  auto value_expr = parse_expression();
  if (!value_expr) {
    value_expr = make_expression(identifier.range, NumberLiteral{0});
  }
  stmt.value = std::move(value_expr);
  SourceRange range{begin, stmt.value->range.end};
  return make_statement(range, std::move(stmt));
}

StmtPtr Parser::parse_call() {
  auto call_token = expect(TokenKind::Call, DiagnosticCode::ExpectedSymbol,
                           "expected 'call'");
  auto name_token = expect(TokenKind::Identifier,
                           DiagnosticCode::ExpectedIdentifier,
                           "expected procedure name after call");
  CallStmt stmt;
  stmt.callee = name_token.lexeme;
  SourceLoc end = name_token.range.end;
  if (peek(0).kind == TokenKind::LParen) {
    auto lparen = lexer_.next();
    end = lparen.range.end;
    if (peek(0).kind == TokenKind::RParen) {
      auto rparen = lexer_.next();
      end = rparen.range.end;
    } else {
      while (true) {
        auto arg = parse_expression();
        if (arg) {
          end = arg->range.end;
          stmt.arguments.push_back(std::move(arg));
        }
        if (peek(0).kind == TokenKind::Comma) {
          lexer_.next();
          continue;
        }
        auto rparen = expect(TokenKind::RParen, DiagnosticCode::ExpectedSymbol,
                             "expected ')' after arguments");
        end = rparen.range.end;
        break;
      }
    }
  }
  SourceRange range{call_token.range.begin, end};
  return make_statement(range, std::move(stmt));
}

StmtPtr Parser::parse_begin_end() {
  auto begin_token = expect(TokenKind::Begin, DiagnosticCode::ExpectedSymbol,
                            "expected 'begin'");
  std::vector<StmtPtr> statements;
  while (peek(0).kind != TokenKind::End &&
         peek(0).kind != TokenKind::EndOfFile) {
    if (auto stmt = parse_statement()) {
      statements.push_back(std::move(stmt));
    }
    if (!match(TokenKind::Semicolon)) {
      break;
    }
  }
  auto end_token = expect(TokenKind::End, DiagnosticCode::ExpectedSymbol,
                          "expected 'end'");
  SourceRange range{begin_token.range.begin, end_token.range.end};
  return make_statement(range, std::move(statements));
}

StmtPtr Parser::parse_if() {
  auto if_token = expect(TokenKind::If, DiagnosticCode::ExpectedSymbol,
                         "expected 'if'");
  auto condition = parse_expression();
  expect(TokenKind::Then, DiagnosticCode::ExpectedSymbol,
         "expected 'then'");
  auto then_branch = parse_statement();
  if (!then_branch) {
    then_branch = make_statement(if_token.range, std::vector<StmtPtr>{});
  }
  std::vector<StmtPtr> else_branch_nodes;
  if (match(TokenKind::Else)) {
    auto else_stmt = parse_statement();
    if (else_stmt) {
      else_branch_nodes.push_back(std::move(else_stmt));
    }
  }
  IfStmt stmt;
  if (!condition) {
    condition = make_expression(if_token.range, BooleanLiteral{false});
  }
  stmt.condition = std::move(condition);
  stmt.then_branch.push_back(std::move(then_branch));
  stmt.else_branch = std::move(else_branch_nodes);
  SourceRange range{if_token.range.begin,
                    stmt.then_branch.back()->range.end};
  if (!stmt.else_branch.empty()) {
    range.end = stmt.else_branch.back()->range.end;
  }
  return make_statement(range, std::move(stmt));
}

StmtPtr Parser::parse_while() {
  auto while_token = expect(TokenKind::While, DiagnosticCode::ExpectedSymbol,
                            "expected 'while'");
  auto condition = parse_expression();
  expect(TokenKind::Do, DiagnosticCode::ExpectedSymbol,
         "expected 'do'");
  auto body = parse_statement();
  if (!body) {
    body = make_statement(while_token.range, std::vector<StmtPtr>{});
  }
  WhileStmt stmt;
  if (!condition) {
    condition = make_expression(while_token.range, BooleanLiteral{false});
  }
  stmt.condition = std::move(condition);
  stmt.body.push_back(std::move(body));
  SourceRange range{while_token.range.begin, stmt.body.back()->range.end};
  return make_statement(range, std::move(stmt));
}

StmtPtr Parser::parse_repeat() {
  auto repeat_token = expect(TokenKind::Repeat, DiagnosticCode::ExpectedSymbol,
                             "expected 'repeat'");
  RepeatStmt stmt;
  while (true) {
    if (auto body_stmt = parse_statement()) {
      stmt.body.push_back(std::move(body_stmt));
    }
    if (match(TokenKind::Semicolon)) {
      continue;
    }
    break;
  }
  expect(TokenKind::Until, DiagnosticCode::ExpectedSymbol,
         "expected 'until'");
  auto condition = parse_expression();
  if (!condition) {
    condition = make_expression(repeat_token.range, BooleanLiteral{false});
  }
  stmt.condition = std::move(condition);
  SourceRange range{repeat_token.range.begin, stmt.condition->range.end};
  return make_statement(range, std::move(stmt));
}

StmtPtr Parser::parse_read() {
  auto read_token = expect(TokenKind::Read, DiagnosticCode::ExpectedSymbol,
                           "expected 'read'");
  ReadStmt stmt;
  SourceLoc end = read_token.range.end;
  if (peek(0).kind == TokenKind::LParen) {
    auto lparen = lexer_.next();
    end = lparen.range.end;
    while (true) {
      auto target = expect(TokenKind::Identifier,
                           DiagnosticCode::ExpectedIdentifier,
                           "expected identifier in read");
      stmt.targets.push_back(target.lexeme);
      end = target.range.end;
      if (peek(0).kind != TokenKind::Comma) {
        break;
      }
      lexer_.next();
    }
    auto rparen = expect(TokenKind::RParen, DiagnosticCode::ExpectedSymbol,
                         "expected ')' after read arguments");
    end = rparen.range.end;
  } else {
    auto target = expect(TokenKind::Identifier,
                         DiagnosticCode::ExpectedIdentifier,
                         "expected identifier in read");
    stmt.targets.push_back(target.lexeme);
    end = target.range.end;
  }
  SourceRange range{read_token.range.begin, end};
  return make_statement(range, std::move(stmt));
}

StmtPtr Parser::parse_write(bool newline) {
  auto write_token = expect(newline ? TokenKind::Writeln : TokenKind::Write,
                            DiagnosticCode::ExpectedSymbol,
                            newline ? "expected 'writeln'" : "expected 'write'");
  WriteStmt stmt;
  stmt.newline = newline;
  SourceLoc end = write_token.range.end;
  if (peek(0).kind == TokenKind::LParen) {
    auto lparen = lexer_.next();
    end = lparen.range.end;
    if (peek(0).kind == TokenKind::RParen) {
      auto rparen = lexer_.next();
      end = rparen.range.end;
    } else {
      while (true) {
        auto value = parse_expression();
        if (value) {
          end = value->range.end;
          stmt.values.push_back(std::move(value));
        }
        if (peek(0).kind == TokenKind::Comma) {
          lexer_.next();
          continue;
        }
        auto rparen = expect(TokenKind::RParen, DiagnosticCode::ExpectedSymbol,
                             "expected ')' after write arguments");
        end = rparen.range.end;
        break;
      }
    }
  } else {
    auto value = parse_expression();
    if (value) {
      end = value->range.end;
      stmt.values.push_back(std::move(value));
    }
  }
  SourceRange range{write_token.range.begin, end};
  return make_statement(range, std::move(stmt));
}

ExprPtr Parser::parse_expression() {
  auto expr = parse_logic_term();
  while (match(TokenKind::Or)) {
    auto rhs = parse_logic_term();
    if (!rhs) {
      rhs = make_expression(expr->range, BooleanLiteral{false});
    }
    SourceRange range{expr->range.begin, rhs->range.end};
    BinaryExpr binary{BinaryOp::Or, std::move(expr), std::move(rhs)};
    expr = make_expression(range, std::move(binary));
  }
  return expr;
}

ExprPtr Parser::parse_logic_term() {
  auto expr = parse_logic_factor();
  while (match(TokenKind::And)) {
    auto rhs = parse_logic_factor();
    if (!rhs) {
      rhs = make_expression(expr->range, BooleanLiteral{false});
    }
    SourceRange range{expr->range.begin, rhs->range.end};
    BinaryExpr binary{BinaryOp::And, std::move(expr), std::move(rhs)};
    expr = make_expression(range, std::move(binary));
  }
  return expr;
}

ExprPtr Parser::parse_logic_factor() {
  auto expr = parse_relation();
  return expr;
}

ExprPtr Parser::parse_relation() {
  auto left = parse_term();
  const auto op_token = peek(0);
  BinaryOp op;
  bool has_op = true;
  switch (op_token.kind) {
    case TokenKind::Equal:
      op = BinaryOp::Equal;
      break;
    case TokenKind::NotEqual:
      op = BinaryOp::NotEqual;
      break;
    case TokenKind::Less:
      op = BinaryOp::Less;
      break;
    case TokenKind::LessEqual:
      op = BinaryOp::LessEqual;
      break;
    case TokenKind::Greater:
      op = BinaryOp::Greater;
      break;
    case TokenKind::GreaterEqual:
      op = BinaryOp::GreaterEqual;
      break;
    default:
      has_op = false;
      break;
  }
  if (!has_op) {
    return left;
  }
  lexer_.next();
  auto right = parse_term();
  if (!right) {
    right = make_expression(op_token.range, NumberLiteral{0});
  }
  SourceRange range{left->range.begin, right->range.end};
  BinaryExpr binary{op, std::move(left), std::move(right)};
  return make_expression(range, std::move(binary));
}

ExprPtr Parser::parse_term() {
  auto expr = parse_factor();
  while (true) {
    TokenKind kind = peek(0).kind;
    BinaryOp op;
    if (kind == TokenKind::Plus) {
      op = BinaryOp::Add;
    } else if (kind == TokenKind::Minus) {
      op = BinaryOp::Subtract;
    } else {
      break;
    }
    lexer_.next();
    auto rhs = parse_factor();
    if (!rhs) {
      rhs = make_expression(expr->range, NumberLiteral{0});
    }
    SourceRange range{expr->range.begin, rhs->range.end};
    BinaryExpr binary{op, std::move(expr), std::move(rhs)};
    expr = make_expression(range, std::move(binary));
  }
  return expr;
}

ExprPtr Parser::parse_factor() {
  const auto token = peek(0);
  if (match(TokenKind::Plus)) {
    auto operand = parse_factor();
    if (!operand) {
      operand = make_expression(token.range, NumberLiteral{0});
    }
    return operand;
  }
  if (match(TokenKind::Minus)) {
    auto operand = parse_factor();
    if (!operand) {
      operand = make_expression(token.range, NumberLiteral{0});
    }
    SourceRange range{token.range.begin, operand->range.end};
    UnaryExpr unary{UnaryOp::Negative, std::move(operand)};
    return make_expression(range, std::move(unary));
  }
  if (match(TokenKind::Not)) {
    auto operand = parse_factor();
    if (!operand) {
      operand = make_expression(token.range, BooleanLiteral{false});
    }
    SourceRange range{token.range.begin, operand->range.end};
    UnaryExpr unary{UnaryOp::Not, std::move(operand)};
    return make_expression(range, std::move(unary));
  }
  if (match(TokenKind::Odd)) {
    auto operand = parse_factor();
    if (!operand) {
      operand = make_expression(token.range, NumberLiteral{0});
    }
    SourceRange range{token.range.begin, operand->range.end};
    UnaryExpr unary{UnaryOp::Odd, std::move(operand)};
    return make_expression(range, std::move(unary));
  }

  auto expr = parse_primary();

  while (true) {
    TokenKind kind = peek(0).kind;
    BinaryOp op;
    if (kind == TokenKind::Star) {
      op = BinaryOp::Multiply;
    } else if (kind == TokenKind::Slash) {
      op = BinaryOp::Divide;
    } else if (kind == TokenKind::Percent) {
      op = BinaryOp::Modulo;
    } else {
      break;
    }
    lexer_.next();
    auto rhs = parse_primary();
    if (!rhs) {
      rhs = make_expression(expr->range, NumberLiteral{1});
    }
    SourceRange range{expr->range.begin, rhs->range.end};
    BinaryExpr binary{op, std::move(expr), std::move(rhs)};
    expr = make_expression(range, std::move(binary));
  }

  return expr;
}

ExprPtr Parser::parse_primary() {
  const auto token = peek(0);
  switch (token.kind) {
    case TokenKind::Number: {
      auto number_token = lexer_.next();
      NumberLiteral literal{number_token.number.value_or(0)};
      return make_expression(number_token.range, std::move(literal));
    }
    case TokenKind::Boolean: {
      auto bool_token = lexer_.next();
      BooleanLiteral literal{bool_token.boolean.value_or(false)};
      return make_expression(bool_token.range, std::move(literal));
    }
    case TokenKind::Identifier: {
      auto ident_token = lexer_.next();
      if (peek(0).kind == TokenKind::LParen) {
        CallExpr call;
        call.callee = ident_token.lexeme;
        auto lparen = lexer_.next();
        SourceLoc end = lparen.range.end;
        if (peek(0).kind == TokenKind::RParen) {
          auto rparen = lexer_.next();
          end = rparen.range.end;
        } else {
          while (true) {
            auto arg = parse_expression();
            if (arg) {
              end = arg->range.end;
              call.arguments.push_back(std::move(arg));
            }
            if (peek(0).kind == TokenKind::Comma) {
              lexer_.next();
              continue;
            }
            auto rparen = expect(TokenKind::RParen, DiagnosticCode::ExpectedSymbol,
                                 "expected ')' after arguments");
            end = rparen.range.end;
            break;
          }
        }
        SourceRange range{ident_token.range.begin, end};
        return make_expression(range, std::move(call));
      }
      if (peek(0).kind == TokenKind::LBracket) {
        lexer_.next();
        auto index = parse_expression();
        auto rbracket = expect(TokenKind::RBracket, DiagnosticCode::ExpectedSymbol,
                               "expected ']' after subscript");
        if (!index) {
          index = make_expression(ident_token.range, NumberLiteral{0});
        }
        ArrayAccessExpr access{ident_token.lexeme, std::move(index)};
        SourceRange range{ident_token.range.begin, rbracket.range.end};
        return make_expression(range, std::move(access));
      }
      IdentifierExpr ident{ident_token.lexeme};
      return make_expression(ident_token.range, std::move(ident));
    }
    case TokenKind::LParen: {
      auto lparen = lexer_.next();
      auto expr = parse_expression();
      expect(TokenKind::RParen, DiagnosticCode::ExpectedSymbol,
             "expected ')' after expression");
      if (!expr) {
        expr = make_expression(lparen.range, NumberLiteral{0});
      }
      return expr;
    }
    default:
      diagnostics_.report({DiagnosticLevel::Error,
                           DiagnosticCode::UnexpectedToken,
                           "unexpected token in expression", token.range});
      lexer_.next();
      return nullptr;
  }
}

std::vector<std::string> Parser::parse_identifier_list() {
  std::vector<std::string> names;
  auto first = expect(TokenKind::Identifier, DiagnosticCode::ExpectedIdentifier,
                      "expected identifier");
  names.push_back(first.lexeme);
  while (match(TokenKind::Comma)) {
    auto next_token = expect(TokenKind::Identifier,
                             DiagnosticCode::ExpectedIdentifier,
                             "expected identifier");
    names.push_back(next_token.lexeme);
  }
  return names;
}

}  // namespace pl0
