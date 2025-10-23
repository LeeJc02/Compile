#include "pl0/SymbolTable.hpp"

#include <algorithm>

namespace pl0 {

SymbolTable::SymbolTable() {
  scopes_.push_back({0, ScopeInfo{0, 0}});
}

void SymbolTable::enter_scope() {
  ScopeInfo info;
  if (!scopes_.empty()) {
    info.level = scopes_.back().info.level + 1;
  }
  scopes_.push_back({symbols_.size(), info});
}

void SymbolTable::leave_scope() {
  if (scopes_.empty()) {
    return;
  }
  auto frame = scopes_.back();
  if (symbols_.size() > frame.start_index) {
    symbols_.resize(frame.start_index);
  }
  scopes_.pop_back();
  if (scopes_.empty()) {
    scopes_.push_back({0, ScopeInfo{0, 0}});
  }
}

SymbolTable::ScopeInfo& SymbolTable::current_scope() {
  return scopes_.back().info;
}

const SymbolTable::ScopeInfo& SymbolTable::current_scope() const {
  return scopes_.back().info;
}

Symbol& SymbolTable::add_symbol(Symbol symbol) {
  symbol.level = current_scope().level;
  symbols_.push_back(std::move(symbol));
  return symbols_.back();
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
  for (auto it = symbols_.rbegin(); it != symbols_.rend(); ++it) {
    if (it->name == name) {
      return &*it;
    }
  }
  return nullptr;
}

const Symbol* SymbolTable::lookup_in_current_scope(const std::string& name) const {
  if (scopes_.empty()) {
    return nullptr;
  }
  const auto frame = scopes_.back();
  for (std::size_t index = symbols_.size(); index > frame.start_index; --index) {
    const auto& symbol = symbols_[index - 1];
    if (symbol.name == name) {
      return &symbol;
    }
  }
  return nullptr;
}

}  // namespace pl0

