#include "parser.hh"
#include <iostream>

#include "ast.hh"
#include "log.hh"
#include "token.hh"

ASTNode* Parser::parseProgram() {
  // guhhhh
  while ((current = lexer.next_token()).getType() != TokenType::TOKEN_EOF) {
    switch (current.getType()) {
      case TokenType::TOKEN_DATA_TYPE:
        return Parser::parseVarDecl();
      default:
        return nullptr;
    }
  }
  return nullptr;
}

ExprNode* Parser::parseVarDecl() {
  // current must be the keyword
  Token type_token = current;

  current = lexer.next_token();
  // current must be the identifier
  Token identifier = current;

  current = lexer.next_token();
  // current must either be an equals, in which case we parseExpr, or a semi
  // colon, in which case we consume and move on
  if (current.getType() == TokenType::TOKEN_SEMICOLON)
    return new VarDeclNode(type_token, identifier, nullptr);

  if (current.getType() == TokenType::TOKEN_EQUALS) {
    current = lexer.next_token();  // skip equals
    return new VarDeclNode(type_token, identifier, parseBinaryExpr());
  }

  return nullptr;
}

// int a = 5; -> vardecl(int a, expr=(literal(5)))
// int b;

ExprNode* Parser::parseExpr() {
  using Tt = TokenType;
  switch (current.getType()) {
    case Tt::TOKEN_LPAREN: {
      current = lexer.next_token();  // gobble up open parenthesis
      ExprNode* expr = parseBinaryExpr(); // parse the expression inside parentheses
      // current should now be the closing parenthesis
      if (current.getType() == Tt::TOKEN_RPAREN) {
        current = lexer.next_token(); // consume closing parenthesis
      }
      return expr;
    }
    default:
      return parseLiteralExpr();
  }

  return nullptr;
}

ExprNode* Parser::parseBinaryExpr() { return parseBinaryExpr(0); }

ExprNode* Parser::parseBinaryExpr(int parent_precedence) {
  ExprNode* left;
  int unary_precedence = getUnaryPrecedence(current.getType());
  if (unary_precedence != -1 && unary_precedence > parent_precedence) {
    Token unary_op = current;
    current = lexer.next_token();
    Token operand = current;
    current = lexer.next_token();
    left = new UnaryExprNode(unary_op, operand);
  } else {
    left = parseExpr();
  }

  int precedence;
  while ((precedence = getBinaryPrecedence(current.getType())) != -1 && precedence > parent_precedence) {
    Token op_token = current;
    current = lexer.next_token();
    
    ExprNode* right = parseBinaryExpr(precedence);
    
    left = new BinaryExprNode(left, op_token, right);
  }

  return left;
}

ExprNode* Parser::parseLiteralExpr() {
  ExprNode* node = new LiteralExprNode(current, lexer.getLine(), lexer.getCol());
  current = lexer.next_token();
  return node;
}
