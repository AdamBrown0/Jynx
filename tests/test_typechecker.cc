// #include <catch2/catch_test_macros.hpp>

// #include "ast.hh"
// #include "test_utils.hh"
// #include "visitor/symbolcollector.hh"
// #include "visitor/typechecker.hh"

// TEST_CASE("TypeChecker sets binary expr type", "[typechecker]") {
//   auto program = parse_program("int x = 1 + 2;");
//   REQUIRE(program);

//   SymbolCollectorVisitor symbol_collector;
//   program->accept(symbol_collector);
//   REQUIRE_FALSE(symbol_collector.has_errors());

//   TypeCheckerVisitor type_checker(symbol_collector.get_global_symbols());
//   program->accept(type_checker);
//   REQUIRE_FALSE(type_checker.has_errors());

//   auto* decl = dynamic_cast<VarDeclNode<NodeInfo>*>(program->children[0].get());
//   REQUIRE(decl);
//   REQUIRE(decl->initializer);

//   auto* binary =
//       dynamic_cast<BinaryExprNode<NodeInfo>*>(decl->initializer.get());
//   REQUIRE(binary);
//   CHECK(binary->extra.resolved_type == TokenType::TOKEN_INT);
// }

// TEST_CASE("TypeChecker reports incompatible assignment", "[typechecker]") {
//   auto program = parse_program("int x = \"hi\";");
//   REQUIRE(program);

//   SymbolCollectorVisitor symbol_collector;
//   program->accept(symbol_collector);
//   REQUIRE_FALSE(symbol_collector.has_errors());

//   TypeCheckerVisitor type_checker(symbol_collector.get_global_symbols());
//   program->accept(type_checker);
//   REQUIRE(type_checker.has_errors());
// }
