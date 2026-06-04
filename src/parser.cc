#include "parser.hh"

#include <memory>
#include <optional>

#include "ast.hh"
#include "log.hh"
#include "sourcelocation.hh"
#include "token.hh"

ProgramNode* Parser::parseProgram() {
  LOG_PARSER_ENTER("Program");
  // Create a new program node with empty children
  ProgramNode* program = new ProgramNode();

  advance();
  current.print();
  while (current.getType() != TokenType::TOKEN_EOF) {
    LOG_DEBUG("PARSING PROGRAM");
    current.print();
    std::optional<Token> access_modifier;
    StmtNode* statement = Parser::parseStatement();
    if (!statement) LOG_PARSER_ERROR("Method declaration required", current);

    program->children.emplace_back(statement);
  }

  return program;
}

StmtNode* Parser::parseStatement() {
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
      SourceLocation loc;
      loc.col = current.getCol();
      loc.line = current.getLine();
      ExprNode* ret = Parser::parseVarDecl();
      if (current.getType() != TokenType::TOKEN_SEMICOLON)
        LOG_PARSER_ERROR("Expected semicolon after variable declaration",
                         current);
      advance();
      return new ExprStmtNode(ret, loc);
    }
    case TokenType::KW_IF:
      return Parser::parseIfStmt();
    case TokenType::KW_WHILE:
      return Parser::parseWhileStmt();
    case TokenType::KW_RETURN: {
      StmtNode* ret = Parser::parseReturnStmt();
      if (current.getType() != TokenType::TOKEN_SEMICOLON)
        LOG_PARSER_ERROR("Expected semicolon after return statement", current);
      advance();
      return ret;
    }
    case TokenType::KW_CLASS:
      return Parser::parseClass();
    default: {
      StmtNode* ret = Parser::parseExprStmt();
      if (current.getType() != TokenType::TOKEN_SEMICOLON)
        LOG_PARSER_ERROR("Expected semicolon after expression statement",
                         current);
      advance();
      return ret;
    }
  }
}

StmtNode* Parser::parseClass() {
  // <class_decl> ::= "class" <identifier> "{" { <class_member> } "}"
  if (ret_advance().getType() != TokenType::KW_CLASS)
    LOG_PARSER_ERROR("Expected class keyword, this shouldn't happen", current);
  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected identifier", identifier);

  if (current.getType() != TokenType::TOKEN_LBRACE)
    LOG_PARSER_ERROR("Expected class to have body", current);

  uptr_vector<ClassMemberNode> members;

  advance();  // advance past opening brace
  while (current.getType() != TokenType::TOKEN_RBRACE) {
    current.print();
    ClassMemberNode* member = Parser::parseClassMember();
    // if (!member) LOG_PARSER_ERROR("Expected class member", current);

    members.emplace_back(member);
  }

  if (current.getType() != TokenType::TOKEN_RBRACE)
    LOG_PARSER_ERROR("Expected closing brace", current);

  advance();  // advance past closing brace
  LOG_DEBUG("FINISHED CLASS");
  current.print();

  return new ClassNode(identifier, std::move(members), lexer.getLocation());
}

ClassMemberNode* Parser::parseClassMember() {
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
  } else if (current.getType() == TokenType::KW_CONSTRUCTOR) {
    if (peek(1).getType() == TokenType::TOKEN_LPAREN) {
      return Parser::parseConstructorDecl();
    }
  }

  LOG_PARSER_ERROR("Expected class member", current);

  while (current.getType() != TokenType::TOKEN_SEMICOLON &&
         current.getType() != TokenType::TOKEN_RBRACE &&
         current.getType() != TokenType::TOKEN_EOF)
    advance();

  if (current.getType() == TokenType::TOKEN_SEMICOLON ||
      current.getType() == TokenType::TOKEN_RBRACE)
    advance();

  return nullptr;
}

ClassMemberNode* Parser::parseConstructorDecl() {
  Token identifier = ret_advance();

  uptr_vector<ParamNode> param_list;
  while (advance().getType() != TokenType::TOKEN_RPAREN) {
    if (current.getType() == TokenType::TOKEN_COMMA) continue;
    if (current.getType() != TokenType::TOKEN_DATA_TYPE)
      LOG_PARSER_ERROR("Expected data type for parameter", current);

    const Type* type = parseType();
    advance();

    if (current.getType() != TokenType::TOKEN_ID)
      LOG_PARSER_ERROR("Expected identifier for parameter", current);
    Token param_identifier = current;
    param_list.emplace_back(
        new ParamNode(type, param_identifier, lexer.getLocation()));
  }
  advance();  // skip closing parenthesis
  BlockNode* body = dynamic_cast<BlockNode*>(Parser::parseBlock());
  if (!body) LOG_PARSER_ERROR("Expected body", current);

  return new ConstructorDeclNode(identifier, std::move(param_list),
                                 std::unique_ptr<BlockNode>(body),
                                 lexer.getLocation());
}

ClassMemberNode* Parser::parseMethodDecl(std::optional<Token> access_modifier) {
  LOG_PARSER_ENTER("Method Decl");
  if (current.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected return type", current);

  const Type* type = parseType();
  advance();

  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected identifier", identifier);

  if (current.getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected parameter list", current);

  uptr_vector<ParamNode> param_list;
  while (advance().getType() != TokenType::TOKEN_RPAREN) {
    if (current.getType() == TokenType::TOKEN_COMMA) continue;
    if (current.getType() != TokenType::TOKEN_DATA_TYPE)
      LOG_PARSER_ERROR("Expected data type for parameter", current);
    const Type* param_type = parseType();
    advance();
    if (current.getType() != TokenType::TOKEN_ID)
      LOG_PARSER_ERROR("Expected identifier for parameter", current);
    Token identifier = current;
    param_list.emplace_back(
        new ParamNode(param_type, identifier, lexer.getLocation()));
  }
  advance();  // skip closing parenthesis
  BlockNode* body = dynamic_cast<BlockNode*>(Parser::parseBlock());

  if (!body) LOG_PARSER_ERROR("Expected function body", current);

  if (!access_modifier.has_value())
    access_modifier =
        Token(TokenType::KW_ACCESS_MODIFIER, "public", lexer.getLocation());

  return new MethodDeclNode(
      access_modifier.value(), false, type, identifier, std::move(param_list),
      std::unique_ptr<BlockNode>(body), lexer.getLocation());
}

ClassMemberNode* Parser::parseFieldDecl(std::optional<Token> access_modifier) {
  // <field_decl> ::= <access_modifier> [ "static" ] <type> <identifier> ";"
  if (current.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected field type", current);

  const Type* type = parseType();
  advance();

  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected field identifier", identifier);

  if (!access_modifier.has_value())
    access_modifier =
        Token(TokenType::KW_ACCESS_MODIFIER, "public", lexer.getLocation());

  return new FieldDeclNode(access_modifier.value(), false, type, identifier,
                           lexer.getLocation());
}

StmtNode* Parser::parseBlock() {
  // <block> ::= "{" { <statement> } "}"

  LOG_PARSER_ENTER("Block");

  SourceLocation block_loc;
  block_loc.line = current.getLine();
  block_loc.col = current.getCol();

  if (current.getType() != TokenType::TOKEN_LBRACE) {
    LOG_PARSER_ERROR("Expected opening brace '{'", current);
  }

  std::vector<std::unique_ptr<StmtNode>> statements;
  advance();  // consume '{'
  while (current.getType() != TokenType::TOKEN_RBRACE) {
    if (current.getType() == TokenType::TOKEN_EOF) {
      LOG_PARSER_ERROR("Expected closing brace", current);
      return new BlockNode(std::move(statements), lexer.getLocation());
    }
    statements.emplace_back(Parser::parseStatement());
  }

  advance();  // consume '}'

  return new BlockNode(std::move(statements), block_loc);
}

StmtNode* Parser::parseIfStmt() {
  // <if_stmt> ::= "if" "(" <expression> ")" <statement> [ "else" <statement> ]
  SourceLocation if_loc;
  if_loc.line = current.getLine();
  if_loc.col = current.getCol();

  if (advance().getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  ExprNode* condition = Parser::parseBinaryExpr();

  LOG_DEBUG("PARSING IFSTMT STMT, current");
  current.print();
  StmtNode* statement = Parser::parseStatement();
  LOG_DEBUG("AFTER STMT");
  current.print();

  // [ "else" <statement> ]

  StmtNode* else_statement = nullptr;

  if (current.getType() == TokenType::KW_ELSE) {
    advance();  // consume else
    else_statement = Parser::parseStatement();
  }

  return new IfStmtNode(condition, statement, else_statement, if_loc);
}

ExprNode* Parser::parseVarDecl() {
  LOG_PARSER_ENTER("VarDecl");
  // current must be the keyword
  if (current.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected data type", current);

  SourceLocation decl_loc;
  decl_loc.line = current.getLine();
  decl_loc.col = current.getCol();

  auto type = parseType();
  advance();

  // check if function decl
  // if (peek(1).getType() == TokenType::TOKEN_LPAREN)
  // return Parser::parseMethodDecl();

  // current must be the identifier
  Token identifier = ret_advance();
  if (identifier.getType() != TokenType::TOKEN_ID)
    LOG_PARSER_ERROR("Expected identifier", identifier);

  if (current.getType() == TokenType::TOKEN_EQUALS) {
    advance();  // skip equals
    ExprNode* init = parseBinaryExpr();
    return new VarDeclNode(type, identifier, init, decl_loc);
  } else {
    return new VarDeclNode(type, identifier, nullptr, decl_loc);
  }

  return nullptr;
}

StmtNode* Parser::parseWhileStmt() {
  // <while_stmt> ::= "while" "(" <expression> ")" <statement>

  SourceLocation while_loc;
  while_loc.line = current.getLine();
  while_loc.col = current.getCol();

  if (advance().getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  ExprNode* condition = Parser::parseBinaryExpr();

  StmtNode* statement = Parser::parseStatement();

  return new WhileStmtNode(condition, statement, while_loc);
}

StmtNode* Parser::parseReturnStmt() {
  // <return_stmt> ::= "return" <expression> ";"
  SourceLocation return_loc;
  return_loc.line = current.getLine();
  return_loc.col = current.getCol();

  advance();
  ExprNode* expression = Parser::parseBinaryExpr();

  return new ReturnStmtNode(expression, return_loc);
}

StmtNode* Parser::parseExprStmt() {
  SourceLocation expr_loc;
  expr_loc.line = current.getLine();
  expr_loc.col = current.getCol();

  ExprStmtNode* expr;
  if (current.getType() == TokenType::TOKEN_ID &&
      peek(1).getType() == TokenType::TOKEN_LPAREN)
    expr = new ExprStmtNode(parseMethodCall(), expr_loc);
  else {
    expr = new ExprStmtNode(parseBinaryExpr(), expr_loc);
  }

  return expr;
}

ExprNode* Parser::parseExpr() {
  using Tk = TokenType;
  switch (current.getType()) {
    case Tk::TOKEN_LPAREN: {
      advance();  // gobble up open parenthesis
      ExprNode* expr =
          parseBinaryExpr();  // parse the expression inside parentheses
      // current should now be the closing parenthesis
      if (current.getType() == Tk::TOKEN_RPAREN) {
        advance();  // consume closing parenthesis
      }
      return expr;
    }
    case Tk::TOKEN_DATA_TYPE:
      return parseVarDecl();
    case Tk::TOKEN_ID:
      return parseIdentifierExpr();
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
    left = parseUnaryExpr();
  } else {
    left = parseExpr();
  }

  int precedence;
  LOG_DEBUG("Current: {}", current.to_string());
  while ((precedence = getBinaryPrecedence(current.getType())) != -1 &&
         precedence > parent_precedence) {
    Token op_token = ret_advance();

    SourceLocation expr_loc;
    if (left) {
      expr_loc = left->location;
    } else {
      expr_loc = lexer.getLocation();
    }

    ExprNode* right = parseBinaryExpr(precedence);

    if (op_token.getType() == TokenType::TOKEN_EQUALS) {
      left = new AssignmentExprNode(left, op_token, right, expr_loc);
    } else {
      left = new BinaryExprNode(left, op_token, right, expr_loc);
    }
  }

  return left;
}

ExprNode* Parser::parseUnaryExpr() {
  Token unary_op = current;
  int unary_prec = getUnaryPrecedence(unary_op.getType());

  SourceLocation expr_loc;
  expr_loc.line = unary_op.getLine();
  expr_loc.col = unary_op.getCol();

  advance();
  ExprNode* operand = parseBinaryExpr(unary_prec);

  return new UnaryExprNode(unary_op, operand, expr_loc);
}

ExprNode* Parser::parseLiteralExpr() {
  SourceLocation expr_loc;
  expr_loc.line = current.getLine();
  expr_loc.col = current.getCol();

  ExprNode* node = new LiteralExprNode(current, expr_loc);
  advance();
  return node;
}

ExprNode* Parser::parseIdentifierExpr() {
  // Check if this is a function call (identifier followed by parenthesis)
  if (peek(1).getType() == TokenType::TOKEN_LPAREN) {
    return parseMethodCall();
  }

  // Simple identifier
  SourceLocation expr_loc;
  expr_loc.line = current.getLine();
  expr_loc.col = current.getCol();

  ExprNode* node = new IdentifierExprNode(current, expr_loc);
  advance();
  return node;
}

ExprNode* Parser::parseMethodCall() {
  LOG_PARSER_ENTER("Method Call");
  // <method_call> ::= <expression> "." <identifier> "(" [ <argument_list> ] ")"
  // <function_call> ::= <identifier> "(" [ <argument_list> ] ")"

  ExprNode* expr = nullptr;
  Token identifier;
  SourceLocation call_loc;

  if (current.getType() == TokenType::TOKEN_ID &&
      peek(1).getType() == TokenType::TOKEN_LPAREN) {
    LOG_DEBUG("IDENTIFIER");
    current.print();
    identifier = ret_advance();
    call_loc.line = identifier.getLine();
    call_loc.col = identifier.getCol();
  } else {
    expr = Parser::parseExpr();
    if (expr) {
      call_loc = expr->location;
    }
    identifier = ret_advance();
    if (identifier.getType() != TokenType::TOKEN_ID)
      LOG_PARSER_ERROR("Expected method identifier", identifier);
  }

  if (current.getType() != TokenType::TOKEN_LPAREN)
    LOG_PARSER_ERROR("Expected opening parenthesis", current);

  uptr_vector<ArgumentNode> arg_list;
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

    ExprNode* arg_expr = Parser::parseExpr();

    SourceLocation arg_loc;
    if (arg_expr) {
      arg_loc = arg_expr->location;
    } else {
      arg_loc = lexer.getLocation();
    }

    arg_list.emplace_back(
        new ArgumentNode(std::unique_ptr<ExprNode>(arg_expr), arg_loc));

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
    return new MethodCallNode(std::unique_ptr<ExprNode>(expr), identifier,
                              std::move(arg_list), call_loc);
  } else {
    return new MethodCallNode(nullptr, identifier, std::move(arg_list),
                              call_loc);
  }
}

const Type* Parser::parseType() {
  if (current.getType() != TokenType::TOKEN_DATA_TYPE)
    LOG_PARSER_ERROR("Expected type token", current);

  if (match(ctx.get_int32_type()))
    return ctx.get_int32_type();
  else if (match(ctx.get_bool_type()))
    return ctx.get_bool_type();
  else if (match(ctx.get_char_type()))
    return ctx.get_char_type();

  return ctx.get_void_type();
}
