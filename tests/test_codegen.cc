#include <catch2/catch_test_macros.hpp>

#include "gen.hh"
#include "test_utils.hh"

TEST_CASE("Codegen emits text section", "[codegen]") {
  auto program = parse_program("int x = 1;");
  REQUIRE(program);

  auto* analyzed = analyze_program(*program);
  REQUIRE(analyzed != nullptr);

  CodeGenerator gen;
  std::string output = gen.generate(*analyzed);

  REQUIRE(output.find(".section .text") != std::string::npos);
  REQUIRE(output.find("_jynx_main") != std::string::npos);
  REQUIRE(output.find("_start") != std::string::npos);
}
