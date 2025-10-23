#include "catch.hpp"

#include "pl0/Lexer.hpp"
#include "pl0/Parser.hpp"

TEST_CASE("Parser handles if-then-else statements") {
  const char* source = "if x then write(1) else write(0).";
  pl0::DiagnosticSink diagnostics;
  pl0::Lexer lexer(source, diagnostics);
  pl0::Parser parser(lexer, diagnostics);

  auto program = parser.parse_program();
  REQUIRE(program != nullptr);
  REQUIRE(!diagnostics.has_errors());
  REQUIRE(program->block.statements.size() == 1);

  const auto& stmt = *program->block.statements.front();
  const auto* if_stmt = std::get_if<pl0::IfStmt>(&stmt.value);
  REQUIRE(if_stmt != nullptr);
  REQUIRE(if_stmt->then_branch.size() == 1);
  REQUIRE(if_stmt->else_branch.size() == 1);
}

TEST_CASE("Parser understands repeat-until loop") {
  const char* source = "repeat write(i); i := i + 1; until i > 10.";
  pl0::DiagnosticSink diagnostics;
  pl0::Lexer lexer(source, diagnostics);
  pl0::Parser parser(lexer, diagnostics);

  auto program = parser.parse_program();
  REQUIRE(program != nullptr);
  REQUIRE(!diagnostics.has_errors());
  REQUIRE(program->block.statements.size() == 1);

  const auto& stmt = *program->block.statements.front();
  const auto* repeat_stmt = std::get_if<pl0::RepeatStmt>(&stmt.value);
  REQUIRE(repeat_stmt != nullptr);
  REQUIRE(repeat_stmt->body.size() == 2);
}

