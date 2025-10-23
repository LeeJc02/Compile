#pragma once

#include <cstdint>
#include <vector>

#include "pl0/AST.hpp"
#include "pl0/Diagnostics.hpp"
#include "pl0/Options.hpp"
#include "pl0/PCode.hpp"
#include "pl0/SymbolTable.hpp"

namespace pl0 {

class CodeGenerator {
 public:
  CodeGenerator(SymbolTable& symbols, InstructionSequence& output,
                DiagnosticSink& diagnostics, const CompilerOptions& options);

  void emit_program(const Program& program);

  [[nodiscard]] const std::vector<Symbol>& symbols() const { return exported_symbols_; }

 private:
  int emit_instruction(const Instruction& instr);
  void patch(int index, int target);
  void emit_block(const Block& block);
  void emit_statement(const Statement& stmt);
  void emit_statements(const std::vector<StmtPtr>& stmts);
  void emit_assignment(const AssignmentStmt& stmt, const SourceRange& range);
  void emit_call(const std::string& callee,
                 const std::vector<ExprPtr>& arguments,
                 const SourceRange& range);
  void emit_if(const IfStmt& stmt);
  void emit_while(const WhileStmt& stmt);
  void emit_repeat(const RepeatStmt& stmt);
  void emit_read(const ReadStmt& stmt, const SourceRange& range);
  void emit_write(const WriteStmt& stmt);
  void emit_expression(const Expression& expr);
  void emit_binary(const BinaryExpr& expr);
  void emit_unary(const UnaryExpr& expr);
  void emit_identifier(const IdentifierExpr& expr, const SourceRange& range);
  void emit_array_access(const ArrayAccessExpr& expr, const SourceRange& range,
                         bool load_value);
  void emit_const(const ConstDecl& decl);
  void emit_var(const VarDecl& decl);
  void emit_procedure(const ProcedureDecl& decl, Symbol& symbol);

  const Symbol* resolve(const std::string& name, const SourceRange& range) const;

  SymbolTable& symbols_;
  InstructionSequence& output_;
  DiagnosticSink& diagnostics_;
  const CompilerOptions& options_;
  std::vector<Symbol> exported_symbols_;
};

}  // namespace pl0
