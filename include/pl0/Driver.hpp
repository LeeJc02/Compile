#pragma once

#include <filesystem>
#include <iosfwd>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "pl0/AST.hpp"
#include "pl0/Codegen.hpp"
#include "pl0/Diagnostics.hpp"
#include "pl0/Options.hpp"
#include "pl0/PCode.hpp"
#include "pl0/Token.hpp"
#include "pl0/VM.hpp"

namespace pl0 {

struct DumpOptions {
  bool tokens = false;
  bool ast = false;
  bool symbols = false;
  bool pcode = false;
};

struct CompileResult {
  InstructionSequence code;
  std::vector<Symbol> symbols;
  std::vector<Token> tokens;
  std::unique_ptr<Program> program;
  std::string source_name;
};

CompileResult compile_file(const std::filesystem::path& input,
                           const CompilerOptions& options,
                           const DumpOptions& dumps,
                           DiagnosticSink& diagnostics,
                           std::ostream& dump_stream);

CompileResult compile_source_text(std::string_view source_name,
                                  const std::string& source,
                                  const CompilerOptions& options,
                                  DiagnosticSink& diagnostics);

InstructionSequence load_pcode_file(const std::filesystem::path& input);

void save_pcode_file(const std::filesystem::path& output,
                     const InstructionSequence& instructions);

VirtualMachine::Result run_instructions(const InstructionSequence& code,
                                        DiagnosticSink& diagnostics,
                                        const RunnerOptions& options);

void print_diagnostics(const DiagnosticSink& diagnostics, std::ostream& out);

}  // namespace pl0
