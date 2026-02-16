#include <catch2/catch_test_macros.hpp>

#include "ast.hh"
#include "test_utils.hh"

// TEST_CASE("Parser parses variable declaration", "[parser]") {
//   auto program = parse_program("int x = 1;");
//   REQUIRE(program);
//   REQUIRE(program->children.size() == 1);

//   auto* decl =
//   dynamic_cast<VarDeclNode<NodeInfo>*>(program->children[0].get());
//   REQUIRE(decl);
//   CHECK(decl->identifier.getValue() == "x");
//   CHECK(decl->type_token.getValue() == "int");
//   REQUIRE(decl->initializer);

//   auto* literal =
//       dynamic_cast<LiteralExprNode<NodeInfo>*>(decl->initializer.get());
//   REQUIRE(literal);
//   CHECK(literal->literal_token.getValue() == "1");
// }
