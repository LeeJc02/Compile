#pragma once

#include <cstdint>
#include <vector>

#include "pl0/Diagnostics.hpp"
#include "pl0/Options.hpp"
#include "pl0/PCode.hpp"

namespace pl0 {

class VirtualMachine {
 public:
  struct Result {
    bool success = true;
    std::int64_t last_value = 0;
  };

  VirtualMachine(DiagnosticSink& diagnostics, const RunnerOptions& options);

  Result execute(const InstructionSequence& code);

 private:
  void push(std::int64_t value);
  std::int64_t pop();
  std::int64_t& at(int index);
  int base(int level, int b) const;

  DiagnosticSink& diagnostics_;
  const RunnerOptions& options_;
  std::vector<std::int64_t> stack_;
  int stack_top_ = 0;
  int base_pointer_ = 0;
  int program_counter_ = 0;
};

}  // namespace pl0

