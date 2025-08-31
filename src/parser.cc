#include "parser.hh"

#include "ast.hh"
#include "token.hh"

ASTNode* Parser::parseProgram() {
  // guhhh

  std::optional<Token> tok_opt = lexer.next_token();
  while (tok_opt.has_value() &&
         tok_opt.value().getType() != TokenType::TOKEN_EOF) {
    current = tok_opt.value();

    switch (current.getType()) {
      case TokenType::KW_INT:
        return Parser::parseVarDecl();
      default:
        return nullptr;
    }

  }
  return nullptr;
}

VarDeclNode* Parser::parseVarDecl() {
  // current must be the keyword
  auto* node = new VarDeclNode;
  node->type = current.getType();

  std::optional<Token> tok_opt = lexer.next_token();
  if (!tok_opt.has_value() || tok_opt.value().getType() != TokenType::TOKEN_ID)
    exit(1);
  current = tok_opt.value();

  // current must be the identifier
  node->name = current.getValue();

  tok_opt = lexer.next_token();
  if (!tok_opt.has_value()) exit(1);
  current = tok_opt.value();

  // current must either be an equals, in which case we parseExpr, or a semi
  // colon, in which case we consume and move on
  if (current.getType() == TokenType::TOKEN_SEMICOLON) return node;

  if (current.getType() == TokenType::TOKEN_EQUALS) {
    node->initializer = parseExpr();
    return node;
  }

  delete node;
  return nullptr;
}

// int a = 5; -> vardecl(int a, expr=(literal(5)))
// int b;

ExprNode* Parser::parseExpr() { return new ExprNode; }
