#pragma once

#include <cstdint>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace pl0 {

enum class Op : std::uint8_t {
  LIT,
  OPR,
  LOD,
  STO,
  CAL,
  INT,
  JMP,
  JPC,
  LDA,
  IDX,
  LDI,
  STI,
  CHK,
  NOP,
};

enum class Opr : std::uint8_t {
  RET = 0,
  NEG = 1,
  ADD = 2,
  SUB = 3,
  MUL = 4,
  DIV = 5,
  ODD = 6,
  MOD = 7,
  EQ = 8,
  NE = 9,
  LT = 10,
  GE = 11,
  GT = 12,
  LE = 13,
  WRITE = 14,
  WRITELN = 15,
  READ = 16,
  AND = 17,
  OR = 18,
  NOT = 19,
};

struct Instruction {
  Op op = Op::NOP;
  std::int32_t level = 0;
  std::int32_t argument = 0;
};

using InstructionSequence = std::vector<Instruction>;

std::string to_string(Op op);
std::string to_string(Opr opr);
std::string to_string(const Instruction& instr);

Instruction parse_instruction(const std::string& text);

void serialize_instructions(const InstructionSequence& instructions,
                            std::ostream& out);

InstructionSequence deserialize_instructions(std::istream& in);

}  // namespace pl0
