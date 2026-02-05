#include "log.hh"
#include "diagnostics.hh"

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <system_error>

#include "ast.hh"
#include "ast_utils.hh"

using namespace Log;

// Static member definitions
Log::Level Logger::current_level = Level::DEBUG;
bool Logger::show_timestamps = true;
bool Logger::show_colors = true;
std::ostream* Logger::output_stream = &std::cout;

// Logger configuration methods
void Logger::set_level(Level level) { current_level = level; }

Log::Level Logger::get_level() { return current_level; }

void Logger::enable_timestamps(bool enable) { show_timestamps = enable; }

void Logger::enable_colors(bool enable) { show_colors = enable; }

void Logger::set_output(std::ostream& stream) { output_stream = &stream; }

// Core logging methods
void Logger::log(Level level, const std::string& message) {
  if (level < current_level) return;

  std::string output;

  if (show_colors) {
    output += get_level_color(level);
  }

  if (show_timestamps) {
    output += "[" + get_timestamp() + "] ";
  }

  output += "[" + get_level_string(level) + "] " + message;

  if (show_colors) {
    output += "\033[0m";  // Reset color
  }

  *output_stream << output << std::endl;
}

void Logger::debug(const std::string& message) { log(Level::DEBUG, message); }

void Logger::info(const std::string& message) { log(Level::INFO, message); }

void Logger::warn(const std::string& message) { log(Level::WARN, message); }

void Logger::error(const std::string& message) { log(Level::ERROR, message); }

void Logger::fatal(const std::string& message) { log(Level::FATAL, message); }

// Helper methods
std::string Logger::get_timestamp() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()) %
            1000;

  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
  ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
  return ss.str();
}

std::string Logger::get_level_string(Level level) {
  switch (level) {
    case Level::DEBUG:
      return "DEBUG";
    case Level::INFO:
      return "INFO ";
    case Level::WARN:
      return "WARN ";
    case Level::ERROR:
      return "ERROR";
    case Level::FATAL:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

std::string Logger::get_level_color(Level level) {
  if (!show_colors) return "";

  switch (level) {
    case Level::DEBUG:
      return "\033[36m";  // Cyan
    case Level::INFO:
      return "\033[32m";  // Green
    case Level::WARN:
      return "\033[33m";  // Yellow
    case Level::ERROR:
      return "\033[31m";  // Red
    case Level::FATAL:
      return "\033[35m";  // Magenta
    default:
      return "\033[0m";  // Reset
  }
}

std::string Logger::format_message(const std::string& format) { return format; }

// Compiler-specific logging functions
namespace Log::Compiler {
void lexer_token(const Token& token) {
  if (Logger::get_level() <= Level::DEBUG) {
    std::stringstream ss;
    ss << "Token: " << static_cast<int>(token.getType()) << " ('"
       << token.getValue() << "') at line " << token.getLine() << ", col "
       << token.getCol();
    Logger::debug(ss.str());
  }
}

void lexer_error(const std::string& message, SourceLocation loc) {
  std::stringstream ss;
  ss << "Lexer error at " << loc.to_string() << ": " << message;
  Logger::error(ss.str());
  Diagnostics::instance().report_error(ss.str());
}

void parser_enter(const std::string& rule) {
  if (Logger::get_level() <= Level::DEBUG) {
    Logger::debug("Entering parser rule: " + rule);
  }
}

void parser_exit(const std::string& rule, bool success) {
  if (Logger::get_level() <= Level::DEBUG) {
    std::string result = success ? "SUCCESS" : "FAILED";
    Logger::debug("Exiting parser rule: " + rule + " (" + result + ")");
  }
}

// // Helper function to get token type string
// const char* token_type_to_string(TokenType type) {
//   switch (type) {
// #define X(name)         \
//   case TokenType::name: \
//     return #name;
//     TOKEN_LIST
// #undef X
//     default:
//       return "UNKNOWN";
//   }
// }

void parser_error(const std::string& message, const Token& token) {
  std::stringstream ss;
  ss << "Parser error at line " << token.getLine() << ", col " << token.getCol()
     << ": " << message << " (found: '" << token.getValue()
     << "', type: " << token_type_to_string(token.getType()) << ")";
  Logger::error(ss.str());
  Diagnostics::instance().report_error(ss.str());
}

void semantic_error(const std::string& message, int line, int col) {
  std::stringstream ss;
  ss << "Semantic error at line " << line << ", col " << col << ": " << message;
  Logger::error(ss.str());
}
}  // namespace Log::Compiler

// AST printing functions using ast_utils
template <typename Extra>
void print_ast_templated(ASTNode<Extra>* root, std::string indent, bool isFirst,
                         bool isLast) {
  if (!root) {
    std::cout << indent << "null" << std::endl;
    return;
  }

  const std::string RESET = "\033[0m";
  const std::string VAR_DECL_COLOR = "\033[1;34m";
  const std::string BINARY_EXPR_COLOR = "\033[1;32m";
  const std::string UNARY_EXPR_COLOR = "\033[1;33m";
  const std::string LITERAL_COLOR = "\033[1;36m";
  const std::string OPERATOR_COLOR = "\033[1;31m";
  const std::string TREE_COLOR = "\033[90m";
  const std::string BOLD_WHITE = "\033[1;37m";

  std::string marker = isFirst ? "" : isLast ? "└── " : "├── ";

  // Print the current node with tree structure in gray
  std::cout << TREE_COLOR << indent << marker << RESET;

  // Get node type and details using ASTStringBuilder
  std::string node_type = ASTStringBuilder::node_type_name(root);
  std::string node_details = ASTStringBuilder::node_to_string(root);

  // Choose color based on node type
  std::string color;
  if (node_type == "VarDecl" || node_type == "Block" || node_type == "IfStmt" ||
      node_type == "ReturnStmt" || node_type == "Class" ||
      node_type == "FieldDecl" || node_type == "MethodDecl" ||
      node_type == "ConstructorDecl" || node_type == "Param") {
    color = VAR_DECL_COLOR;
  } else if (node_type == "BinaryExpr" || node_type == "Assignment" ||
             node_type == "MethodCall") {
    color = BINARY_EXPR_COLOR;
  } else if (node_type == "UnaryExpr") {
    color = UNARY_EXPR_COLOR;
  } else if (node_type == "Literal" || node_type == "Identifier" ||
             node_type == "Argument") {
    color = LITERAL_COLOR;
  } else {
    color = BOLD_WHITE;  // Default color for unknown types
  }

  std::cout << color << node_type << ": " << RESET << node_details << std::endl;

  // Print children based on node type
  std::string newIndent = indent + (isLast ? "    " : "│   ");
  std::vector<ASTNode<Extra>*> children;

  // Collect children based on node type
  if (auto program = dynamic_cast<ProgramNode<Extra>*>(root)) {
    for (const auto& child : program->children) {
      if (child) children.push_back(child.get());
    }
  } else if (auto varDecl = dynamic_cast<VarDeclNode<Extra>*>(root)) {
    if (varDecl->initializer) children.push_back(varDecl->initializer.get());
  } else if (auto binaryExpr = dynamic_cast<BinaryExprNode<Extra>*>(root)) {
    if (binaryExpr->left) children.push_back(binaryExpr->left.get());
    if (binaryExpr->right) children.push_back(binaryExpr->right.get());
  } else if (auto unaryExpr = dynamic_cast<UnaryExprNode<Extra>*>(root)) {
    if (unaryExpr->operand) children.push_back(unaryExpr->operand.get());
  } else if (auto assignment = dynamic_cast<AssignmentExprNode<Extra>*>(root)) {
    if (assignment->left) children.push_back(assignment->left.get());
    if (assignment->right) children.push_back(assignment->right.get());
  } else if (auto methodCall = dynamic_cast<MethodCallNode<Extra>*>(root)) {
    if (methodCall->expr) children.push_back(methodCall->expr.get());
    for (const auto& arg : methodCall->arg_list) {
      if (arg) children.push_back(arg.get());
    }
  } else if (auto block = dynamic_cast<BlockNode<Extra>*>(root)) {
    for (const auto& stmt : block->statements) {
      if (stmt) children.push_back(stmt.get());
    }
  } else if (auto ifStmt = dynamic_cast<IfStmtNode<Extra>*>(root)) {
    if (ifStmt->condition) children.push_back(ifStmt->condition.get());
    if (ifStmt->statement) children.push_back(ifStmt->statement.get());
    if (ifStmt->else_stmt) children.push_back(ifStmt->else_stmt.get());
  } else if (auto whileStmt = dynamic_cast<WhileStmtNode<Extra>*>(root)) {
    if (whileStmt->condition) children.push_back(whileStmt->condition.get());
    if (whileStmt->statement) children.push_back(whileStmt->statement.get());
  } else if (auto returnStmt = dynamic_cast<ReturnStmtNode<Extra>*>(root)) {
    if (returnStmt->ret) children.push_back(returnStmt->ret.get());
  } else if (auto exprStmt = dynamic_cast<ExprStmtNode<Extra>*>(root)) {
    if (exprStmt->expr) children.push_back(exprStmt->expr.get());
  } else if (auto classNode = dynamic_cast<ClassNode<Extra>*>(root)) {
    for (const auto& member : classNode->members) {
      if (member) children.push_back(member.get());
    }
  } else if (auto methodDecl = dynamic_cast<MethodDeclNode<Extra>*>(root)) {
    for (const auto& param : methodDecl->param_list) {
      if (param) children.push_back(param.get());
    }
    if (methodDecl->body) children.push_back(methodDecl->body.get());
  } else if (auto constructorDecl =
                 dynamic_cast<ConstructorDeclNode<Extra>*>(root)) {
    for (const auto& param : constructorDecl->param_list) {
      if (param) children.push_back(param.get());
    }
    if (constructorDecl->body) children.push_back(constructorDecl->body.get());
  } else if (auto arg = dynamic_cast<ArgumentNode<Extra>*>(root)) {
    if (arg->expr) children.push_back(arg->expr.get());
  }

  // Print all children
  for (size_t i = 0; i < children.size(); ++i) {
    bool isLastChild = (i == children.size() - 1);
    print_ast_templated(children[i], newIndent, false, isLastChild);
  }
}

// Wrapper functions for backwards compatibility
void Log::print_ast(ASTNode<NodeInfo>* root) {
  print_ast_templated(root, "", true, false);
}

void Log::print_ast(ASTNode<NodeInfo>* root, std::string indent, bool isFirst,
                    bool isLast) {
  print_ast_templated(root, indent, isFirst, isLast);
}

// Simplified reflection-based AST Printer using ast_utils
template <typename Extra>
class SimpleASTPrinter {
 private:
  std::ostream& output;

  // Color constants
  static const std::string RESET;
  static const std::string VAR_DECL_COLOR;
  static const std::string BINARY_EXPR_COLOR;
  static const std::string UNARY_EXPR_COLOR;
  static const std::string LITERAL_COLOR;
  static const std::string TREE_COLOR;
  static const std::string PROGRAM_COLOR;

 public:
  SimpleASTPrinter(std::ostream& os = std::cout) : output(os) {}

  void print(ASTNode<Extra>* root, const std::string& indent = "",
             bool isFirst = true, bool isLast = true) {
    if (!root) {
      output << indent << "null" << std::endl;
      return;
    }

    std::string marker = isFirst ? "" : isLast ? "└── " : "├── ";
    std::string node_type = ASTStringBuilder::node_type_name(root);
    std::string node_details = ASTStringBuilder::detailed_node_info(root);

    // Choose color based on node type
    std::string color = get_color_for_node_type(node_type);

    // Print the node
    output << TREE_COLOR << indent << marker << RESET << color << node_type
           << ": " << RESET << node_details << std::endl;

    // Print children
    std::string newIndent = indent + (isLast ? "    " : "│   ");
    auto children = get_children(root);

    for (size_t i = 0; i < children.size(); ++i) {
      bool isLastChild = (i == children.size() - 1);
      if (children[i]) {
        print(children[i], newIndent, false, isLastChild);
      }
    }
  }

 private:
  std::string get_color_for_node_type(const std::string& node_type) {
    if (node_type == "VarDecl" || node_type == "Block" ||
        node_type == "IfStmt" || node_type == "WhileStmt" ||
        node_type == "ReturnStmt" || node_type == "FieldDecl" ||
        node_type == "Param" || node_type == "ExprStmt") {
      return VAR_DECL_COLOR;
    } else if (node_type == "BinaryExpr" || node_type == "Assignment" ||
               node_type == "MethodCall" || node_type == "MethodDecl" ||
               node_type == "ConstructorDecl") {
      return BINARY_EXPR_COLOR;
    } else if (node_type == "UnaryExpr") {
      return UNARY_EXPR_COLOR;
    } else if (node_type == "Literal" || node_type == "Identifier" ||
               node_type == "Argument") {
      return LITERAL_COLOR;
    } else if (node_type == "Program" || node_type == "Class") {
      return PROGRAM_COLOR;
    }
    return RESET;  // Default
  }

  std::vector<ASTNode<Extra>*> get_children(ASTNode<Extra>* node) {
    std::vector<ASTNode<Extra>*> children;

    if (auto program = dynamic_cast<ProgramNode<Extra>*>(node)) {
      for (const auto& child : program->children) {
        if (child) children.push_back(child.get());
      }
    } else if (auto varDecl = dynamic_cast<VarDeclNode<Extra>*>(node)) {
      if (varDecl->initializer) children.push_back(varDecl->initializer.get());
    } else if (auto binaryExpr = dynamic_cast<BinaryExprNode<Extra>*>(node)) {
      if (binaryExpr->left) children.push_back(binaryExpr->left.get());
      if (binaryExpr->right) children.push_back(binaryExpr->right.get());
    } else if (auto unaryExpr = dynamic_cast<UnaryExprNode<Extra>*>(node)) {
      if (unaryExpr->operand) children.push_back(unaryExpr->operand.get());
    } else if (auto assignment =
                   dynamic_cast<AssignmentExprNode<Extra>*>(node)) {
      if (assignment->left) children.push_back(assignment->left.get());
      if (assignment->right) children.push_back(assignment->right.get());
    } else if (auto methodCall = dynamic_cast<MethodCallNode<Extra>*>(node)) {
      if (methodCall->expr) children.push_back(methodCall->expr.get());
      for (const auto& arg : methodCall->arg_list) {
        if (arg) children.push_back(arg.get());
      }
    } else if (auto block = dynamic_cast<BlockNode<Extra>*>(node)) {
      for (const auto& stmt : block->statements) {
        if (stmt) children.push_back(stmt.get());
      }
    } else if (auto ifStmt = dynamic_cast<IfStmtNode<Extra>*>(node)) {
      if (ifStmt->condition) children.push_back(ifStmt->condition.get());
      if (ifStmt->statement) children.push_back(ifStmt->statement.get());
      if (ifStmt->else_stmt) children.push_back(ifStmt->else_stmt.get());
    } else if (auto whileStmt = dynamic_cast<WhileStmtNode<Extra>*>(node)) {
      if (whileStmt->condition) children.push_back(whileStmt->condition.get());
      if (whileStmt->statement) children.push_back(whileStmt->statement.get());
    } else if (auto returnStmt = dynamic_cast<ReturnStmtNode<Extra>*>(node)) {
      if (returnStmt->ret) children.push_back(returnStmt->ret.get());
    } else if (auto exprStmt = dynamic_cast<ExprStmtNode<Extra>*>(node)) {
      if (exprStmt->expr) children.push_back(exprStmt->expr.get());
    } else if (auto classNode = dynamic_cast<ClassNode<Extra>*>(node)) {
      for (const auto& member : classNode->members) {
        if (member) children.push_back(member.get());
      }
    } else if (auto methodDecl = dynamic_cast<MethodDeclNode<Extra>*>(node)) {
      for (const auto& param : methodDecl->param_list) {
        if (param) children.push_back(param.get());
      }
      if (methodDecl->body) children.push_back(methodDecl->body.get());
    } else if (auto constructorDecl =
                   dynamic_cast<ConstructorDeclNode<Extra>*>(node)) {
      for (const auto& param : constructorDecl->param_list) {
        if (param) children.push_back(param.get());
      }
      if (constructorDecl->body)
        children.push_back(constructorDecl->body.get());
    } else if (auto arg = dynamic_cast<ArgumentNode<Extra>*>(node)) {
      if (arg->expr) children.push_back(arg->expr.get());
    }

    return children;
  }
};

// Static color definitions
template <typename Extra>
const std::string SimpleASTPrinter<Extra>::RESET = "\033[0m";
template <typename Extra>
const std::string SimpleASTPrinter<Extra>::VAR_DECL_COLOR = "\033[1;34m";
template <typename Extra>
const std::string SimpleASTPrinter<Extra>::BINARY_EXPR_COLOR = "\033[1;32m";
template <typename Extra>
const std::string SimpleASTPrinter<Extra>::UNARY_EXPR_COLOR = "\033[1;33m";
template <typename Extra>
const std::string SimpleASTPrinter<Extra>::LITERAL_COLOR = "\033[1;36m";
template <typename Extra>
const std::string SimpleASTPrinter<Extra>::TREE_COLOR = "\033[90m";
template <typename Extra>
const std::string SimpleASTPrinter<Extra>::PROGRAM_COLOR = "\033[1;35m";

// New simplified reflection-based print function
void Log::print_ast_reflection(ASTNode<NodeInfo>* root) {
  SimpleASTPrinter<NodeInfo> printer;
  printer.print(root);
}

// Utility functions
void Log::print_separator(const std::string& title) {
  std::string sep(60, '=');
  if (!title.empty()) {
    std::cout << sep << std::endl;
    std::cout << "  " << title << std::endl;
    std::cout << sep << std::endl;
  } else {
    std::cout << sep << std::endl;
  }
}

void Log::print_header(const std::string& title) {
  std::cout << std::endl;
  print_separator(title);
}

void Log::print_tokens(const std::vector<Token>& tokens) {
  Logger::info("Token stream (" + std::to_string(tokens.size()) + " tokens):");
  for (size_t i = 0; i < tokens.size(); ++i) {
    const auto& token = tokens[i];
    std::stringstream ss;
    ss << "  [" << std::setw(3) << i << "] " << std::setw(15) << std::left
       << static_cast<int>(token.getType()) << " '" << token.getValue() << "' "
       << "(" << token.getLine() << ":" << token.getCol() << ")";
    std::cout << ss.str() << std::endl;
  }
}
