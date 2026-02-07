#include "parser.hh"

#include <memory>
#include <optional>

#include "ast.hh"
#include "log.hh"
#include "token.hh"

ProgramNode<NodeInfo>* Parser::parseProgram() {
  LOG_PARSER_ENTER("Program");
  // Create a new program node with empty children
  ProgramNode<NodeInfo>* program = new ProgramNode<NodeInfo>();

  advance();
  current.print();
  while (current.getType() != TokenType::TOKEN_EOF) {
    LOG_DEBUG("PARSING PROGRAM");
    current.print();
    _StmtNode* statement = Parser::parseStatement();
    if (!statement) LOG_PARSER_ERROR("Not implemented", current);

    program->children.emplace_back(statement);
  }

  return program;
}

StmtNode<NodeInfo>* Parser::parseStatement() {
  LOG_PARSER_ENTER("Statement");
  current.print();
  switch (current.getType()) {
    case TokenType::TOKEN_LBRACE:
      return Parser::parseBlock();
    case TokenType::TOKEN_DATA_TYPE: {
      if (peek(2).getType() ==
          TokenType::TOKEN_LPAREN) {  // temporary whilst making code gen for
                                      // functions
        std::optional<Token> access_modifier;
        return Parser::parseMethodDecl(access_modifier);
      }
      return Parser::parseVarDecl();
    }
    case TokenType::KW_IF:
      return Parser::parseIfStmt();
    case TokenType::KW_WHILE:
      return Parser::parseWhileStmt();
    case TokenType::KW_RETURN:
      return Parser::parseReturnStmt();
    case TokenType::KW_CLASS:
      return Parser::parseClass();
    default:
      return Parser::parseExprStmt();
  }
}

StmtNode<NodeInfo>* Parser::parseClass() {
  // <class_decl> ::= "class" <identifier> "{" { <class_member> } "}"
  if (ret_advance().getType() != TokenType::KW_CLASS)
    LOG_PARSER_ERROR("Expected class keyword, this shouldn't happen", current);
  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected identifier", identifier);

  if (current.getType() != TokenType::TOKEN_LBRACE)
    LOG_PARSER_ERROR("Expected class to have body", current);

  uptr_vector<ClassMemberNode<NodeInfo>> members;

  advance();  // advance past opening brace
  while (current.getType() != TokenType::TOKEN_RBRACE) {
    current.print();
    ClassMemberNode<NodeInfo>* member = Parser::parseClassMember();
    if (!member) LOG_PARSER_ERROR("Expected class member", current);

    members.emplace_back(member);
  }

  if (current.getType() != TokenType::TOKEN_RBRACE)
    LOG_PARSER_ERROR("Expected closing brace", current);

  advance();  // advance past closing brace
  LOG_DEBUG("FINISHED CLASS");
  current.print();

  return new ClassNode<NodeInfo>(identifier, std::move(members),
                                   lexer.getLocation());
}

ClassMemberNode<NodeInfo>* Parser::parseClassMember() {
  LOG_PARSER_ENTER("Class Member");
  std::optional<Token> access_modifier;
  if (current.getType() == TokenType::KW_ACCESS_MODIFIER)
    access_modifier = ret_advance();

  current.print();
  if (current.getType() == TokenType::TOKEN_DATA_TYPE) {
    // is method?
    if (peek(2).getType() == TokenType::TOKEN_LPAREN)
      return Parser::parseMethodDecl(access_modifier);
    else
      return Parser::parseFieldDecl(access_modifier);
  } else if (current.getType() == TokenType::TOKEN_ID) {
    if (peek(1).getType() == TokenType::TOKEN_LPAREN) {
      return Parser::parseConstructorDecl();
    }
  }

  LOG_PARSER_ERROR("Expected class member", current);
  return nullptr;
}

ClassMemberNode<NodeInfo>* Parser::parseConstructorDecl() {
  Token identifier = ret_advance();

  uptr_vector<ParamNode<NodeInfo>> param_list;
  while (advance().getType() != TokenType::TOKEN_RPAREN) {
    if (current.getType() == TokenType::TOKEN_COMMA) continue;
    if (current.getType() != TokenType::TOKEN_DATA_TYPE)
      LOG_PARSER_ERROR("Expected data type for parameter", current);
    Token type = ret_advance();
    if (current.getType() != TokenType::TOKEN_ID)
      LOG_PARSER_ERROR("Expected identifier for parameter", current);
    Token param_identifier = current;
    param_list.emplace_back(
        new ParamNode<NodeInfo>(type, param_identifier, lexer.getLocation()));
  }
  advance();  // skip closing parenthesis
  BlockNode<NodeInfo>* body =
      dynamic_cast<BlockNode<NodeInfo>*>(Parser::parseBlock());
  if (!body) LOG_PARSER_ERROR("Expected body", current);

  return new ConstructorDeclNode<NodeInfo>(
      identifier, std::move(param_list),
      std::unique_ptr<BlockNode<NodeInfo>>(body), lexer.getLocation());
}

ClassMemberNode<NodeInfo>* Parser::parseMethodDecl(
    std::optional<Token> access_modifier) {
  LOG_PARSER_ENTER("Method Decl");
  Token type = ret_advance();
  if (type.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected return type", type);

  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected identifier", identifier);

  if (current.getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected parameter list", current);

  uptr_vector<ParamNode<NodeInfo>> param_list;
  LOG_DEBUG("OUTSIDE");
  current.print();
  while (advance().getType() != TokenType::TOKEN_RPAREN) {
    LOG_DEBUG("INSIDE");
    current.print();
    if (current.getType() == TokenType::TOKEN_COMMA) continue;
    if (current.getType() != TokenType::TOKEN_DATA_TYPE)
      LOG_PARSER_ERROR("Expected data type for parameter", current);
    Token type = ret_advance();
    if (current.getType() != TokenType::TOKEN_ID)
      LOG_PARSER_ERROR("Expected identifier for parameter", current);
    Token identifier = current;
    param_list.emplace_back(
        new ParamNode<NodeInfo>(type, identifier, lexer.getLocation()));
  }
  current.print();
  advance();  // skip closing parenthesis
  BlockNode<NodeInfo>* body =
      dynamic_cast<BlockNode<NodeInfo>*>(Parser::parseBlock());

  if (!body) LOG_PARSER_ERROR("Expected function body", current);

  if (!access_modifier.has_value())
    access_modifier =
        Token(TokenType::KW_ACCESS_MODIFIER, "public", lexer.getLocation());

  return new MethodDeclNode<NodeInfo>(
      access_modifier.value(), false, type, identifier, std::move(param_list),
      std::unique_ptr<BlockNode<NodeInfo>>(body), lexer.getLocation());
}

ClassMemberNode<NodeInfo>* Parser::parseFieldDecl(
    std::optional<Token> access_modifier) {
  // <field_decl> ::= <access_modifier> [ "static" ] <type> <identifier> ";"
  Token type = ret_advance();
  if (type.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected field type", type);

  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected field identifier", identifier);

  if (current.getType() != TokenType::TOKEN_SEMICOLON)
    LOG_PARSER_ERROR("Expected semicolon after field declaration", current);
  advance();  // consume semicolon

  if (!access_modifier.has_value())
    access_modifier =
        Token(TokenType::KW_ACCESS_MODIFIER, "public", lexer.getLocation());

  return new FieldDeclNode<NodeInfo>(access_modifier.value(), false, type,
                                       identifier, lexer.getLocation());
}

StmtNode<NodeInfo>* Parser::parseBlock() {
  // <block> ::= "{" { <statement> } "}"

  LOG_PARSER_ENTER("Block");

  if (current.getType() != TokenType::TOKEN_LBRACE) {
    LOG_PARSER_ERROR("Expected opening brace '{'", current);
  }

  std::vector<std::unique_ptr<_StmtNode>> statements;
  advance();  // consume '{'
  while (current.getType() != TokenType::TOKEN_RBRACE) {
    if (current.getType() == TokenType::TOKEN_EOF) {
      LOG_PARSER_ERROR("Expected closing brace", current);
      return new BlockNode<NodeInfo>(std::move(statements),
                                     lexer.getLocation());
    }
    statements.emplace_back(Parser::parseStatement());
  }

  advance();  // consume '}'

  return new BlockNode<NodeInfo>(std::move(statements), lexer.getLocation());
}

StmtNode<NodeInfo>* Parser::parseIfStmt() {
  // <if_stmt> ::= "if" "(" <expression> ")" <statement> [ "else" <statement> ]
  if (advance().getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  _ExprNode* condition = Parser::parseBinaryExpr();

  LOG_DEBUG("PARSING IFSTMT STMT, current");
  current.print();
  _StmtNode *statement = Parser::parseStatement();
  LOG_DEBUG("AFTER STMT");
  current.print();

  // [ "else" <statement> ]

  StmtNode<NodeInfo>* else_statement = nullptr;

  if (current.getType() == TokenType::KW_ELSE) {
    advance();  // consume else
    else_statement = Parser::parseStatement();
  }

  return new IfStmtNode<NodeInfo>(condition, statement, else_statement,
                                  lexer.getLocation());
}

StmtNode<NodeInfo>* Parser::parseVarDecl() {
  LOG_PARSER_ENTER("VarDecl");
  // current must be the keyword
  Token type_token = ret_advance();
  if (type_token.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected data type", type_token);

  // check if function decl
  // if (peek(1).getType() == TokenType::TOKEN_LPAREN)
  // return Parser::parseMethodDecl();

  // current must be the identifier
  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected identifier", identifier);

  // current must either be an equals, in which case we parseExpr, or a semi
  // colon, in which case we consume and move on
  if (current.getType() == TokenType::TOKEN_SEMICOLON) {
    advance();  // consume semicolon
    return new VarDeclNode<NodeInfo>(type_token, identifier, nullptr,
                                       lexer.getLocation());
  } else if (current.getType() == TokenType::TOKEN_EQUALS) {
    advance();  // skip equals
    _ExprNode* init = parseBinaryExpr();
    if (current.getType() != TokenType::TOKEN_SEMICOLON)
      LOG_PARSER_ERROR("Expected semicolon after declaration", current);
    advance();  // consume semicolon
    return new VarDeclNode<NodeInfo>(type_token, identifier, init,
                                       lexer.getLocation());
  } else {
    LOG_PARSER_ERROR("Expected semi-colon or initializer", current);
  }

  return nullptr;
}

StmtNode<NodeInfo>* Parser::parseWhileStmt() {
  // <while_stmt> ::= "while" "(" <expression> ")" <statement>

  // advance();
  if (advance().getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  _ExprNode* condition = Parser::parseBinaryExpr();

  _StmtNode* statement = Parser::parseStatement();

  return new WhileStmtNode<NodeInfo>(condition, statement,
                                       lexer.getLocation());
}

StmtNode<NodeInfo>* Parser::parseReturnStmt() {
  // <return_stmt> ::= "return" <expression> ";"
  advance();
  _ExprNode* expression = Parser::parseBinaryExpr();
  if (current.getType() != TokenType::TOKEN_SEMICOLON)
    LOG_PARSER_ERROR("Expected semicolon after return", current);
  advance();
  return new ReturnStmtNode<NodeInfo>(expression, lexer.getLocation());
}

StmtNode<NodeInfo>* Parser::parseExprStmt() {
  ExprStmtNode<NodeInfo>* expr;
  if (current.getType() == TokenType::TOKEN_ID &&
      peek(1).getType() == TokenType::TOKEN_LPAREN)
    expr = new ExprStmtNode<NodeInfo>(parseMethodCall(), lexer.getLocation());
  else {
    expr = new ExprStmtNode<NodeInfo>(parseBinaryExpr(), lexer.getLocation());
  }
  if (current.getType() != TokenType::TOKEN_SEMICOLON)
    LOG_PARSER_ERROR("Expected semicolon after expression", current);
  advance();
  return expr;
}

ExprNode<NodeInfo>* Parser::parseExpr() {
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

ExprNode<NodeInfo>* Parser::parseBinaryExpr() { return parseBinaryExpr(0); }

ExprNode<NodeInfo>* Parser::parseBinaryExpr(int parent_precedence) {
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

    if (op_token.getType() == TokenType::TOKEN_EQUALS) {
      left = new AssignmentExprNode<NodeInfo>(left, op_token, right,
                                              lexer.getLocation());
    } else {
      left = new BinaryExprNode<NodeInfo>(left, op_token, right,
                                            lexer.getLocation());
    }
  }

  return left;
}

ExprNode<NodeInfo>* Parser::parseUnaryExpr() {
  Token unary_op = current;
  int unary_prec = getUnaryPrecedence(unary_op.getType());
  advance();
  _ExprNode* operand = parseBinaryExpr(unary_prec);

  return new UnaryExprNode<NodeInfo>(unary_op, operand, lexer.getLocation());
}

ExprNode<NodeInfo>* Parser::parseLiteralExpr() {
  _ExprNode* node =
      new LiteralExprNode<NodeInfo>(current, lexer.getLocation());
  advance();
  return node;
}

ExprNode<NodeInfo>* Parser::parseIdentifierExpr() {
  // Check if this is a function call (identifier followed by parenthesis)
  if (peek(1).getType() == TokenType::TOKEN_LPAREN) {
    return parseMethodCall();
  }

  // Simple identifier
  _ExprNode* node =
      new IdentifierExprNode<NodeInfo>(current, lexer.getLocation());
  advance();
  return node;
}

ExprNode<NodeInfo>* Parser::parseMethodCall() {
  LOG_PARSER_ENTER("Method Call");
  // <method_call> ::= <expression> "." <identifier> "(" [ <argument_list> ] ")"
  // <function_call> ::= <identifier> "(" [ <argument_list> ] ")"

  _ExprNode* expr = nullptr;
  Token identifier;

  if (current.getType() == TokenType::TOKEN_ID &&
      peek(1).getType() == TokenType::TOKEN_LPAREN) {
    LOG_DEBUG("IDENTIFIER");
    current.print();
    identifier = ret_advance();
  } else {
    expr = Parser::parseExpr();
    identifier = ret_advance();
    if (identifier.getType() != TokenType::TOKEN_ID)
      LOG_PARSER_ERROR("Expected method identifier", identifier);
  }

  if (current.getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  uptr_vector<ArgumentNode<NodeInfo>> arg_list;
  LOG_DEBUG("OUTSIDE CALL");
  current.print();
  advance();  // advance past opening parenthesis
  while (current.getType() != TokenType::TOKEN_RPAREN) {
    LOG_DEBUG("INSIDE CALL");
    current.print();
    if (current.getType() == TokenType::TOKEN_EOF) {
      LOG_PARSER_ERROR("Unexpected end of file in function call", current);
      break;
    }
    if (current.getType() == TokenType::TOKEN_COMMA) {
      advance();
      continue;
    }

    _ExprNode* arg_expr = Parser::parseExpr();

    arg_list.emplace_back(new ArgumentNode<NodeInfo>(
        std::unique_ptr<ExprNode<NodeInfo>>(arg_expr), lexer.getLocation()));

    // After parsing an expression, we should either see a comma or closing
    // parenthesis
    if (current.getType() != TokenType::TOKEN_COMMA &&
        current.getType() != TokenType::TOKEN_RPAREN) {
      advance();  // advance to the next token
    }
  }

  if (current.getType() != TokenType::TOKEN_RPAREN)
    LOG_PARSER_ERROR("Expected closing parenthesis", current);
  advance();

  if (expr) {
    return new MethodCallNode<NodeInfo>(
        std::unique_ptr<ExprNode<NodeInfo>>(expr), identifier,
        std::move(arg_list), lexer.getLocation());
  } else {
    return new MethodCallNode<NodeInfo>(
        nullptr, identifier, std::move(arg_list), lexer.getLocation());
  }
}
