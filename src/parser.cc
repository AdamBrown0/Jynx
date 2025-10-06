#include "parser.hh"

#include "ast.hh"
#include "log.hh"

ProgramNode<ParseExtra>* Parser::parseProgram() {
  // Create a new program node with empty children
  ProgramNode<ParseExtra>* program = new ProgramNode<ParseExtra>();

  while (advance().getType() != TokenType::TOKEN_EOF) {
    _StmtNode* statement = Parser::parseStatement();
    if (!statement) LOG_PARSER_ERROR("Not implemented", current);

    program->children.emplace_back(statement);
  }

  return program;
}

StmtNode<ParseExtra>* Parser::parseStatement() {
  switch (current.getType()) {
    case TokenType::TOKEN_LBRACE:
      return Parser::parseBlock();
    case TokenType::TOKEN_DATA_TYPE:
      return Parser::parseVarDecl();
    case TokenType::KW_IF:
      return Parser::parseIfStmt();
    default:
      return Parser::parseExprStmt();
  }
}

StmtNode<ParseExtra>* Parser::parseBlock() {
  // <block> ::= "{" { <statement> } "}"

  std::vector<std::unique_ptr<_StmtNode>> statements;
  while (advance().getType() != TokenType::TOKEN_RBRACE) {
    statements.emplace_back(Parser::parseStatement());
  }

  return new BlockNode<ParseExtra>(std::move(statements), lexer.getLocation());
}

StmtNode<ParseExtra>* Parser::parseIfStmt() {
  // <if_stmt> ::= "if" "(" <expression> ")" <statement> [ "else" <statement> ]

  if (current.getType() == TokenType::KW_ELSE) {
    advance();
    return Parser::parseStatement();
  }

  if (advance().getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  _ExprNode* condition = Parser::parseBinaryExpr();

  _StmtNode* statement = Parser::parseStatement();

  // [ "else" <statement> ]

  IfStmtNode<ParseExtra>* else_statement = nullptr;

  if (peek(1).getType() == TokenType::KW_ELSE) {
    advance();
    else_statement =
        reinterpret_cast<IfStmtNode<ParseExtra>*>(Parser::parseIfStmt());
  }

  return new IfStmtNode<ParseExtra>(condition, statement, else_statement,
                                    lexer.getLocation());
}

StmtNode<ParseExtra>* Parser::parseVarDecl() {
  // current must be the keyword
  Token type_token = ret_advance();

  // current must be the identifier
  Token identifier = ret_advance();

  // current must either be an equals, in which case we parseExpr, or a semi
  // colon, in which case we consume and move on
  if (current.getType() == TokenType::TOKEN_SEMICOLON)
    return new VarDeclNode<ParseExtra>(type_token, identifier, nullptr,
                                       lexer.getLocation());

  if (current.getType() == TokenType::TOKEN_EQUALS) {
    advance();  // skip equals
    return new VarDeclNode<ParseExtra>(type_token, identifier,
                                       parseBinaryExpr(), lexer.getLocation());
  }

  return nullptr;
}

StmtNode<ParseExtra>* Parser::parseExprStmt() { return nullptr; }

ExprNode<ParseExtra>* Parser::parseExpr() {
  using Tk = TokenType;
  switch (current.getType()) {
    case Tk::TOKEN_LPAREN: {
      advance();  // gobble up open parenthesis
      _ExprNode* expr =
          parseBinaryExpr();  // parse the expression inside parentheses
      // current should now be the closing parenthesis
      if (current.getType() == Tk::TOKEN_RPAREN) {
        advance();  // consume closing parenthesis
      }
      return expr;
    }
    default:
      return parseLiteralExpr();
  }

  return nullptr;
}

ExprNode<ParseExtra>* Parser::parseBinaryExpr() { return parseBinaryExpr(0); }

ExprNode<ParseExtra>* Parser::parseBinaryExpr(int parent_precedence) {
  _ExprNode* left;
  int unary_precedence = getUnaryPrecedence(current.getType());
  if (unary_precedence != -1 && unary_precedence > parent_precedence) {
    left = parseUnaryExpr();
  } else {
    left = parseExpr();
  }

  int precedence;
  while ((precedence = getBinaryPrecedence(current.getType())) != -1 &&
         precedence > parent_precedence) {
    Token op_token = ret_advance();

    _ExprNode* right = parseBinaryExpr(precedence);

    left = new BinaryExprNode<ParseExtra>(left, op_token, right,
                                          lexer.getLocation());
  }

  return left;
}

ExprNode<ParseExtra>* Parser::parseUnaryExpr() {
  Token unary_op = current;
  int unary_prec = getUnaryPrecedence(unary_op.getType());
  advance();
  _ExprNode* operand = parseBinaryExpr(unary_prec);

  return new UnaryExprNode<ParseExtra>(unary_op, operand, lexer.getLocation());
}

ExprNode<ParseExtra>* Parser::parseLiteralExpr() {
  _ExprNode* node =
      new LiteralExprNode<ParseExtra>(current, lexer.getLocation());
  advance();
  return node;
}
