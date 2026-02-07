#include <catch2/catch_test_macros.hpp>

#include "test_utils.hh"

TEST_CASE("Sema succeeds on valid program", "[sema]") {
  auto program = parse_program("int x = 1;");
  REQUIRE(program);

  auto* analyzed = analyze_program(*program);
  REQUIRE(analyzed != nullptr);
}

TEST_CASE("Sema fails on invalid assignment", "[sema]") {
  auto program = parse_program("int x = \"hi\";");
  REQUIRE(program);

  auto* analyzed = analyze_program(*program);
  REQUIRE(analyzed == nullptr);
}
