#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "pl0/Diagnostics.hpp"

namespace pl0 {

enum class BinaryOp {
  Add,
  Subtract,
  Multiply,
  Divide,
  Modulo,
  Equal,
  NotEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  And,
  Or,
};

enum class UnaryOp {
  Positive,
  Negative,
  Not,
  Odd,
};

struct Expression;
struct Statement;
struct Block;

using ExprPtr = std::unique_ptr<Expression>;
using StmtPtr = std::unique_ptr<Statement>;

struct NumberLiteral {
  std::int64_t value = 0;
};

struct BooleanLiteral {
  bool value = false;
};

struct IdentifierExpr {
  std::string name;
};

struct ArrayAccessExpr {
  std::string name;
  ExprPtr index;
};

struct BinaryExpr {
  BinaryOp op;
  ExprPtr lhs;
  ExprPtr rhs;
};

struct UnaryExpr {
  UnaryOp op;
  ExprPtr operand;
};

struct CallExpr {
  std::string callee;
  std::vector<ExprPtr> arguments;
};

struct Expression {
  SourceRange range;
  std::variant<NumberLiteral, BooleanLiteral, IdentifierExpr, ArrayAccessExpr,
               BinaryExpr, UnaryExpr, CallExpr>
      value;
};

struct AssignmentStmt {
  std::string target;
  ExprPtr index;
  ExprPtr value;
};

struct CallStmt {
  std::string callee;
  std::vector<ExprPtr> arguments;
};

struct BlockStmt;

struct IfStmt {
  ExprPtr condition;
  std::vector<StmtPtr> then_branch;
  std::vector<StmtPtr> else_branch;
};

struct WhileStmt {
  ExprPtr condition;
  std::vector<StmtPtr> body;
};

struct RepeatStmt {
  std::vector<StmtPtr> body;
  ExprPtr condition;
};

struct ReadStmt {
  std::vector<std::string> targets;
};

struct WriteStmt {
  std::vector<ExprPtr> values;
  bool newline = false;
};

struct Statement {
  SourceRange range;
  std::variant<AssignmentStmt, CallStmt, IfStmt, WhileStmt, RepeatStmt, ReadStmt,
               WriteStmt, std::vector<StmtPtr>>
      value;
};

struct ConstDecl {
  SourceRange range;
  std::string name;
  std::int64_t value = 0;
};

enum class VarType {
  Integer,
  Boolean,
};

struct VarDecl {
  SourceRange range;
  std::string name;
  VarType type = VarType::Integer;
  std::optional<std::size_t> array_size;
};

struct ProcedureDecl {
  SourceRange range;
  std::string name;
  std::vector<VarDecl> parameters;
  std::unique_ptr<Block> body;
};

struct Block {
  std::vector<ConstDecl> consts;
  std::vector<VarDecl> vars;
  std::vector<ProcedureDecl> procedures;
  std::vector<StmtPtr> statements;
};

struct Program {
  Block block;
};

}  // namespace pl0

