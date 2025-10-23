#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace pl0 {

struct SourceLoc {
  std::size_t line = 1;
  std::size_t column = 1;
};

struct SourceRange {
  SourceLoc begin;
  SourceLoc end;
};

enum class DiagnosticLevel {
  Error,
  Warning,
  Note,
};

enum class DiagnosticCode : std::uint16_t {
  UnexpectedToken,
  UnterminatedComment,
  InvalidNumber,
  Redeclaration,
  UndeclaredIdentifier,
  ExpectedIdentifier,
  ExpectedSymbol,
  InvalidAssignmentTarget,
  InvalidArraySubscript,
  StackOverflow,
  StackUnderflow,
  DivisionByZero,
  RuntimeError,
  IOError,
  InternalError,
};

struct Diagnostic {
  DiagnosticLevel level = DiagnosticLevel::Error;
  DiagnosticCode code = DiagnosticCode::InternalError;
  std::string message;
  SourceRange range;
};

class DiagnosticSink {
 public:
  void report(Diagnostic diagnostic);

  [[nodiscard]] bool has_errors() const;

  [[nodiscard]] const std::vector<Diagnostic>& diagnostics() const;

  void clear();

 private:
  std::vector<Diagnostic> diagnostics_;
};

std::ostream& operator<<(std::ostream& os, const Diagnostic& diagnostic);

}  // namespace pl0

