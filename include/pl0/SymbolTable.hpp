#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "pl0/AST.hpp"

namespace pl0 {

enum class SymbolKind {
  Constant,
  Variable,
  Procedure,
  Parameter,
  Array,
};

struct Symbol {
  std::string name;
  SymbolKind kind = SymbolKind::Variable;
  VarType type = VarType::Integer;
  int level = 0;
  int address = 0;
  std::size_t size = 1;
  bool by_value = true;
  std::int64_t constant_value = 0;
};

class SymbolTable {
 public:
  struct ScopeInfo {
    int level = 0;
    int data_offset = 0;
  };

  SymbolTable();

  void enter_scope();
  void leave_scope();

  [[nodiscard]] ScopeInfo& current_scope();
  [[nodiscard]] const ScopeInfo& current_scope() const;

  Symbol& add_symbol(Symbol symbol);

  [[nodiscard]] const Symbol* lookup(const std::string& name) const;
  [[nodiscard]] const Symbol* lookup_in_current_scope(const std::string& name) const;

  [[nodiscard]] const std::vector<Symbol>& symbols() const { return symbols_; }

 private:
  struct ScopeFrame {
    std::size_t start_index = 0;
    ScopeInfo info;
  };

  std::vector<Symbol> symbols_;
  std::vector<ScopeFrame> scopes_;
};

}  // namespace pl0

