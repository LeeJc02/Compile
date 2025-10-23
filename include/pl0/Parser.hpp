#pragma once

#include <memory>

#include "pl0/AST.hpp"
#include "pl0/Diagnostics.hpp"
#include "pl0/Lexer.hpp"
#include "pl0/SymbolTable.hpp"

namespace pl0 {

class Parser {
 public:
  Parser(Lexer& lexer, DiagnosticSink& diagnostics);

  std::unique_ptr<Program> parse_program();

 private:
  const Token& peek(std::size_t lookahead = 0);
  bool match(TokenKind kind);
  Token expect(TokenKind kind, DiagnosticCode code,
               std::string_view message);

  void synchronize(const std::vector<TokenKind>& sync_tokens);

  std::unique_ptr<Block> parse_block();
  void parse_const_declarations(Block& block);
  void parse_var_declarations(Block& block);
  void parse_procedure_declarations(Block& block);
  StmtPtr parse_statement();
  StmtPtr parse_assignment();
  StmtPtr parse_call();
  StmtPtr parse_begin_end();
  StmtPtr parse_if();
  StmtPtr parse_while();
  StmtPtr parse_repeat();
  StmtPtr parse_read();
  StmtPtr parse_write(bool newline);

  ExprPtr parse_expression();
  ExprPtr parse_logic_term();
  ExprPtr parse_logic_factor();
  ExprPtr parse_relation();
  ExprPtr parse_term();
  ExprPtr parse_factor();
  ExprPtr parse_primary();

  std::vector<std::string> parse_identifier_list();

  Lexer& lexer_;
  DiagnosticSink& diagnostics_;
  bool panic_mode_ = false;
};

}  // namespace pl0
