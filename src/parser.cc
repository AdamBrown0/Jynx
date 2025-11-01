#include "parser.hh"

#include "ast.hh"
#include "ast_utils.hh"
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
    case TokenType::KW_WHILE:
      return Parser::parseWhileStmt();
    case TokenType::KW_RETURN:
      return Parser::parseReturnStmt();
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

  StmtNode<ParseExtra>* else_statement = nullptr;

  if (peek(1).getType() == TokenType::KW_ELSE) {
    advance();
    else_statement = Parser::parseIfStmt();
  }

  return new IfStmtNode<ParseExtra>(condition, statement, else_statement,
                                    lexer.getLocation());
}

StmtNode<ParseExtra>* Parser::parseVarDecl() {
  // current must be the keyword
  Token type_token = ret_advance();
  if (type_token.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected data type", type_token);

  // current must be the identifier
  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected identifier", identifier);

  // current must either be an equals, in which case we parseExpr, or a semi
  // colon, in which case we consume and move on
  if (current.getType() == TokenType::TOKEN_SEMICOLON) {
    return new VarDeclNode<ParseExtra>(type_token, identifier, nullptr,
                                       lexer.getLocation());
  } else if (current.getType() == TokenType::TOKEN_EQUALS) {
    advance();  // skip equals
    return new VarDeclNode<ParseExtra>(type_token, identifier,
                                       parseBinaryExpr(), lexer.getLocation());
  } else {
    LOG_PARSER_ERROR("Expected semi-colon or initializer", current);
  }

  return nullptr;
}

StmtNode<ParseExtra>* Parser::parseWhileStmt() {
  // <while_stmt> ::= "while" "(" <expression> ")" <statement>

  // advance();
  if (advance().getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  _ExprNode* condition = Parser::parseBinaryExpr();

  _StmtNode* statement = Parser::parseStatement();

  return new WhileStmtNode<ParseExtra>(condition, statement,
                                       lexer.getLocation());
}

StmtNode<ParseExtra>* Parser::parseReturnStmt() {
  // <return_stmt> ::= "return" <expression> ";"
  LOG_PARSER_ENTER("Return stmt");
  advance();
  LOG_DEBUG("[PARSE] Parsing binary expr");
  _ExprNode* expression = Parser::parseBinaryExpr();
  return new ReturnStmtNode<ParseExtra>(expression, lexer.getLocation());
}

StmtNode<ParseExtra>* Parser::parseExprStmt() {
  ExprStmtNode<ParseExtra>* expr =
      new ExprStmtNode<ParseExtra>(parseBinaryExpr(), lexer.getLocation());

  return expr;
}

ExprNode<ParseExtra>* Parser::parseExpr() {
  LOG_DEBUG("[PARSE] ParseExpr current: {}", current.to_string());
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
    case Tk::TOKEN_ID:
      return parseIdentifierExpr();
    default:
      return parseLiteralExpr();
  }

  return nullptr;
}

ExprNode<ParseExtra>* Parser::parseBinaryExpr() { return parseBinaryExpr(0); }

ExprNode<ParseExtra>* Parser::parseBinaryExpr(int parent_precedence) {
  LOG_DEBUG("[PARSE] Binary expr entered");
  _ExprNode* left;
  int unary_precedence = getUnaryPrecedence(current.getType());
  if (unary_precedence != -1 && unary_precedence > parent_precedence) {
    left = parseUnaryExpr();
  } else {
    left = parseExpr();
  }

  LOG_DEBUG("[PARSE] While loop entering");
  int precedence;
  while ((precedence = getBinaryPrecedence(current.getType())) != -1 &&
         precedence > parent_precedence) {
    LOG_DEBUG("[PARSE] precedence: {}, parent_precedence: {}", precedence,
              parent_precedence);
    Token op_token = ret_advance();

    _ExprNode* right = parseBinaryExpr(precedence);

    if (op_token.getType() == TokenType::TOKEN_EQUALS) {
      left = new AssignmentExprNode<ParseExtra>(left, op_token, right,
                                                lexer.getLocation());
    } else {
      left = new BinaryExprNode<ParseExtra>(left, op_token, right,
                                            lexer.getLocation());
    }
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

ExprNode<ParseExtra>* Parser::parseIdentifierExpr() {
  _ExprNode* node =
      new IdentifierExprNode<ParseExtra>(current, lexer.getLocation());
  advance();
  return node;
}
