#include "catch.hpp"

#include "pl0/Lexer.hpp"

TEST_CASE("Lexer tokenizes keywords, identifiers, and numbers") {
  pl0::DiagnosticSink diagnostics;
  pl0::Lexer lexer("var answer := 42;", diagnostics);

  auto token1 = lexer.next();
  REQUIRE(token1.kind == pl0::TokenKind::Var);

  auto token2 = lexer.next();
  REQUIRE(token2.kind == pl0::TokenKind::Identifier);
  REQUIRE(token2.lexeme == "answer");

  auto token3 = lexer.next();
  REQUIRE(token3.kind == pl0::TokenKind::Assign);

  auto token4 = lexer.next();
  REQUIRE(token4.kind == pl0::TokenKind::Number);
  REQUIRE(token4.number.value() == 42);
}

TEST_CASE("Lexer skips comments and recognizes booleans") {
  const char* source = "// comment\nvar flag := true; /* block */ flag := false;";
  pl0::DiagnosticSink diagnostics;
  pl0::Lexer lexer(source, diagnostics);

  REQUIRE(lexer.next().kind == pl0::TokenKind::Var);
  REQUIRE(lexer.next().lexeme == "flag");
  REQUIRE(lexer.next().kind == pl0::TokenKind::Assign);
  auto true_token = lexer.next();
  REQUIRE(true_token.kind == pl0::TokenKind::Boolean);
  REQUIRE(true_token.boolean.value());

  // skip semicolon
  lexer.next();

  auto ident = lexer.next();
  REQUIRE(ident.lexeme == "flag");
  auto assign = lexer.next();
  REQUIRE(assign.kind == pl0::TokenKind::Assign);
  auto false_token = lexer.next();
  REQUIRE(false_token.kind == pl0::TokenKind::Boolean);
  REQUIRE(!false_token.boolean.value());
}

