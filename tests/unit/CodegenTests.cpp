#include "catch.hpp"

#include "TestSupport.hpp"

TEST_CASE("Code generator emits write instruction") {
  pl0::DiagnosticSink diagnostics;
  pl0::CompilerOptions options;
  auto instructions = pl0::test::compile_source("begin write(1); end.", options, diagnostics);
  REQUIRE(!diagnostics.has_errors());

  bool found_write = false;
  for (const auto& instr : instructions) {
    if (instr.op == pl0::Op::OPR && instr.argument == static_cast<int>(pl0::Opr::WRITE)) {
      found_write = true;
      break;
    }
  }
  REQUIRE(found_write);
}

TEST_CASE("Array assignment emits bounds check when enabled") {
  pl0::DiagnosticSink diagnostics;
  pl0::CompilerOptions options;
  options.enable_bounds_check = true;
  auto instructions = pl0::test::compile_source("var a[2]; begin a[1] := 3; end.", options, diagnostics);
  REQUIRE(!diagnostics.has_errors());

  bool found_chk = false;
  for (const auto& instr : instructions) {
    if (instr.op == pl0::Op::CHK) {
      found_chk = true;
      REQUIRE(instr.argument == 2);
      break;
    }
  }
  REQUIRE(found_chk);
}
