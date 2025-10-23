#include "pl0/Diagnostics.hpp"

#include <iostream>
#include <sstream>

namespace pl0 {

void DiagnosticSink::report(Diagnostic diagnostic) {
  diagnostics_.push_back(std::move(diagnostic));
}

bool DiagnosticSink::has_errors() const {
  for (const auto& diag : diagnostics_) {
    if (diag.level == DiagnosticLevel::Error) {
      return true;
    }
  }
  return false;
}

const std::vector<Diagnostic>& DiagnosticSink::diagnostics() const {
  return diagnostics_;
}

void DiagnosticSink::clear() {
  diagnostics_.clear();
}

static std::string_view level_to_string(DiagnosticLevel level) {
  switch (level) {
    case DiagnosticLevel::Error:
      return "error";
    case DiagnosticLevel::Warning:
      return "warning";
    case DiagnosticLevel::Note:
      return "note";
  }
  return "unknown";
}

std::ostream& operator<<(std::ostream& os, const Diagnostic& diagnostic) {
  os << level_to_string(diagnostic.level) << " ";
  os << static_cast<int>(diagnostic.code) << ": ";
  os << diagnostic.message;
  os << " (";
  os << diagnostic.range.begin.line << ":" << diagnostic.range.begin.column;
  os << "-";
  os << diagnostic.range.end.line << ":" << diagnostic.range.end.column;
  os << ")";
  return os;
}

}  // namespace pl0

