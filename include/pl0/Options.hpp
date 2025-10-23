#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace pl0 {

struct CompilerOptions {
  bool dump_tokens = false;
  bool dump_ast = false;
  bool dump_symbols = false;
  bool dump_pcode = false;
  bool enable_bounds_check = false;
};

struct RunnerOptions {
  bool trace_vm = false;
  bool enable_bounds_check = false;
};

struct CliOptions {
  std::string command;
  std::filesystem::path input;
  std::optional<std::filesystem::path> output;
  CompilerOptions compiler;
  RunnerOptions runtime;
};

}  // namespace pl0

