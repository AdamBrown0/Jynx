#include "log.hh"
#include <iostream>
#include <iomanip>
#include <ctime>
#include "ast.hh"

using namespace Log;

// Static member definitions
Log::Level Logger::current_level = Level::DEBUG;
bool Logger::show_timestamps = true;
bool Logger::show_colors = true;
std::ostream* Logger::output_stream = &std::cout;

// Logger configuration methods
void Logger::set_level(Level level) {
    current_level = level;
}

Log::Level Logger::get_level() {
    return current_level;
}

void Logger::enable_timestamps(bool enable) {
    show_timestamps = enable;
}

void Logger::enable_colors(bool enable) {
    show_colors = enable;
}

void Logger::set_output(std::ostream& stream) {
    output_stream = &stream;
}

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
        output += "\033[0m"; // Reset color
    }
    
    *output_stream << output << std::endl;
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warn(const std::string& message) {
    log(Level::WARN, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::fatal(const std::string& message) {
    log(Level::FATAL, message);
}

// Helper methods
std::string Logger::get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::get_level_string(Level level) {
    switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO:  return "INFO ";
        case Level::WARN:  return "WARN ";
        case Level::ERROR: return "ERROR";
        case Level::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::get_level_color(Level level) {
    if (!show_colors) return "";
    
    switch (level) {
        case Level::DEBUG: return "\033[36m"; // Cyan
        case Level::INFO:  return "\033[32m"; // Green
        case Level::WARN:  return "\033[33m"; // Yellow
        case Level::ERROR: return "\033[31m"; // Red
        case Level::FATAL: return "\033[35m"; // Magenta
        default: return "\033[0m";            // Reset
    }
}

std::string Logger::format_message(const std::string& format) {
    return format;
}

// Compiler-specific logging functions
namespace Log::Compiler {
    void lexer_token(const Token& token) {
        if (Logger::get_level() <= Level::DEBUG) {
            std::stringstream ss;
            ss << "Token: " << static_cast<int>(token.getType()) 
               << " ('" << token.getValue() << "') at line " << token.getLine() 
               << ", col " << token.getCol();
            Logger::debug(ss.str());
        }
    }
    
    void lexer_error(const std::string& message, SourceLocation loc) {
        std::stringstream ss;
        ss << "Lexer error at " << loc.to_string() << ": " << message;
        Logger::error(ss.str());
        exit(1); // token not found
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
    
    // Helper function to get token type string
    const char* token_type_to_string(TokenType type) {
        switch (type) {
#define X(name) case TokenType::name: return #name;
            TOKEN_LIST
#undef X
            default: return "UNKNOWN";
        }
    }
    
    void parser_error(const std::string& message, const Token& token) {
        std::stringstream ss;
        ss << "Parser error at line " << token.getLine() << ", col " << token.getCol() 
           << ": " << message << " (found: '" << token.getValue() << "', type: " << token_type_to_string(token.getType()) << ")";
        Logger::error(ss.str());
        exit(1); // syntax error
    }
    
    void semantic_error(const std::string& message, int line, int col) {
        std::stringstream ss;
        ss << "Semantic error at line " << line << ", col " << col << ": " << message;
        Logger::error(ss.str());
    }
}

// AST printing functions (enhanced from existing)
template<typename Extra>
void print_ast_templated(ASTNode<Extra>* root, std::string indent, bool isFirst, bool isLast) {
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
    
    // Print node type and details with colors
    if (auto program = dynamic_cast<ProgramNode<Extra>*>(root)) {
        std::cout << VAR_DECL_COLOR << "Program" << RESET << std::endl;
        
        // Print all children with proper tree structure
        // Use base indent, no additional spacing since Program is typically the root
        for (size_t i = 0; i < program->children.size(); ++i) {
            bool isLastChild = (i == program->children.size() - 1);
            print_ast_templated(program->children[i].get(), indent, false, isLastChild);
        }
    } else if (auto varDecl = dynamic_cast<VarDeclNode<Extra>*>(root)) {
        std::cout << VAR_DECL_COLOR << "VarDecl: " << RESET 
                  << BOLD_WHITE << varDecl->identifier.getValue() << RESET << std::endl;
        
        // Print initializer if it exists
        if (varDecl->initializer) {
            std::string newIndent = indent + (isLast ? "    " : "│   ");
            print_ast_templated(varDecl->initializer.get(), newIndent, false, true);
        }
    } else if (auto binaryExpr = dynamic_cast<BinaryExprNode<Extra>*>(root)) {
        std::cout << BINARY_EXPR_COLOR << "BinaryExpr: " << RESET 
                  << OPERATOR_COLOR << "'" << binaryExpr->op.getValue() << "'" << RESET << std::endl;
        
        // Calculate new indent based on current position
        std::string newIndent = indent + (isLast ? "    " : "│   ");
        
        if (binaryExpr->left && binaryExpr->right) {
            // Both children exist
            print_ast_templated(binaryExpr->left.get(), newIndent, false, false);
            print_ast_templated(binaryExpr->right.get(), newIndent, false, true);
        } else if (binaryExpr->left) {
            // Only left child
            print_ast_templated(binaryExpr->left.get(), newIndent, false, true);
        } else if (binaryExpr->right) {
            // Only right child
            print_ast_templated(binaryExpr->right.get(), newIndent, false, true);
        }
    } else if (auto unaryExpr = dynamic_cast<UnaryExprNode<Extra>*>(root)) {
        std::cout << UNARY_EXPR_COLOR << "UnaryExpr: " << RESET 
                  << OPERATOR_COLOR << "'" << unaryExpr->op.getValue() << "'" << RESET << std::endl;
        
        // Calculate new indent based on current position
        std::string newIndent = indent + (isLast ? "    " : "│   ");
        
        // Print the operand as a proper AST node
        if (unaryExpr->operand) {
            print_ast_templated(unaryExpr->operand.get(), newIndent, false, true);
        }
    } else if (auto literalExpr = dynamic_cast<LiteralExprNode<Extra>*>(root)) {
        std::cout << LITERAL_COLOR << "Literal: " << RESET 
                  << "'" << BOLD_WHITE << literalExpr->literal_token.getValue() << RESET << "'" << std::endl;
    } else if (auto expr = dynamic_cast<ExprNode<Extra>*>(root)) {
        std::cout << "Expression (generic)" << std::endl;
    } else {
        std::cout << "ASTNode (unknown type)" << std::endl;
    }
}

// Wrapper functions for backwards compatibility
void Log::print_ast(ASTNode<ParseExtra>* root) {
    print_ast_templated(root, "", true, false);
}

void Log::print_ast(ASTNode<ParseExtra>* root, std::string indent, bool isFirst, bool isLast) {
    print_ast_templated(root, indent, isFirst, isLast);
}

// Reflection-based AST Printer
#include <typeindex>
#include <functional>
#include <unordered_map>

template<typename Extra>
class ReflectionASTPrinter {
private:
    using NodeHandler = std::function<void(ASTNode<Extra>*, const std::string&, bool, bool, std::ostream&)>;
    std::unordered_map<std::type_index, NodeHandler> handlers;
    std::ostream& output;
    
    // Color constants
    static const std::string RESET;
    static const std::string VAR_DECL_COLOR;
    static const std::string BINARY_EXPR_COLOR;
    static const std::string UNARY_EXPR_COLOR;
    static const std::string LITERAL_COLOR;
    static const std::string OPERATOR_COLOR;
    static const std::string TREE_COLOR;
    static const std::string BOLD_WHITE;
    static const std::string PROGRAM_COLOR;
    
public:
    ReflectionASTPrinter(std::ostream& os = std::cout) : output(os) {
        register_all_handlers();
    }
    
    void print(ASTNode<Extra>* root, const std::string& indent = "", bool isFirst = true, bool isLast = true) {
        if (!root) {
            output << indent << "null" << std::endl;
            return;
        }
        
        std::type_index nodeType = std::type_index(typeid(*root));
        
        if (handlers.count(nodeType)) {
            handlers[nodeType](root, indent, isFirst, isLast, output);
        } else {
            // Fallback for unknown types
            std::string marker = isFirst ? "" : isLast ? "└── " : "├── ";
            output << TREE_COLOR << indent << marker << RESET 
                   << "Unknown[" << nodeType.name() << "]" << std::endl;
        }
    }
    
private:
    void register_all_handlers() {
        // ============ Expression Nodes ============
        
        // Register BinaryExprNode handler
        register_handler<BinaryExprNode<Extra>>("BinaryExpr", BINARY_EXPR_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << OPERATOR_COLOR << "'" << node->op.getValue() << "'" << RESET;
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                if (node->left) children.push_back(node->left.get());
                if (node->right) children.push_back(node->right.get());
                return children;
            });
        
        // Register UnaryExprNode handler
        register_handler<UnaryExprNode<Extra>>("UnaryExpr", UNARY_EXPR_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << OPERATOR_COLOR << "'" << node->op.getValue() << "'" << RESET;
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                if (node->operand) {
                    return {node->operand.get()};
                }
                return {};
            });
        
        // Register LiteralExprNode handler
        register_handler<LiteralExprNode<Extra>>("Literal", LITERAL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": '" << BOLD_WHITE << node->literal_token.getValue() << RESET << "'";
            });
            
        // Register IdentifierExprNode handler
        register_handler<IdentifierExprNode<Extra>>("Identifier", LITERAL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << BOLD_WHITE << node->identifier.getValue() << RESET;
            });
            
        // Register AssignmentExprNode handler
        register_handler<AssignmentExprNode<Extra>>("Assignment", BINARY_EXPR_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << OPERATOR_COLOR << node->op.getValue() << RESET;
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                if (node->left) children.push_back(node->left.get());
                if (node->right) children.push_back(node->right.get());
                return children;
            });
            
        // Register MethodCallNode handler
        register_handler<MethodCallNode<Extra>>("MethodCall", BINARY_EXPR_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << BOLD_WHITE << node->identifier.getValue() << RESET << "()";
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                if (node->expr) children.push_back(node->expr.get());
                for (const auto& arg : node->arg_list) {
                    if (arg) children.push_back(arg.get());
                }
                return children;
            });

        // ============ Statement Nodes ============
        
        // Register ProgramNode handler
        register_handler<ProgramNode<Extra>>("Program", PROGRAM_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)node; (void)indent; (void)isFirst; (void)isLast;
                // Special handling for ProgramNode - print children directly
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                for (const auto& child : node->children) {
                    if (child) children.push_back(child.get());
                }
                return children;
            });
        
        // Register VarDeclNode handler
        register_handler<VarDeclNode<Extra>>("VarDecl", VAR_DECL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << OPERATOR_COLOR << node->type_token.getValue() << RESET 
                       << " " << BOLD_WHITE << node->identifier.getValue() << RESET;
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                if (node->initializer) {
                    return {node->initializer.get()};
                }
                return {};
            });
        
        // Register BlockNode handler
        register_handler<BlockNode<Extra>>("Block", VAR_DECL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": {" << node->statements.size() << " statements}";
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                for (const auto& stmt : node->statements) {
                    if (stmt) children.push_back(stmt.get());
                }
                return children;
            });
        
        // Register IfStmtNode handler
        register_handler<IfStmtNode<Extra>>("IfStmt", VAR_DECL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)node; (void)indent; (void)isFirst; (void)isLast;
                // No additional details for if statement
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                if (node->condition) children.push_back(node->condition.get());
                if (node->statement) children.push_back(node->statement.get());
                if (node->else_stmt) children.push_back(node->else_stmt.get());
                return children;
            });
            
        // Register ReturnStmtNode handler
        register_handler<ReturnStmtNode<Extra>>("ReturnStmt", VAR_DECL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)node; (void)indent; (void)isFirst; (void)isLast;
                // No additional details
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                if (node->ret) {
                    return {node->ret.get()};
                }
                return {};
            });

        // ============ Class-related Nodes ============
        
        // Register ClassNode handler
        register_handler<ClassNode<Extra>>("Class", PROGRAM_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << BOLD_WHITE << node->identifier.getValue() << RESET 
                       << " {" << node->members.size() << " members}";
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                for (const auto& member : node->members) {
                    if (member) children.push_back(member.get());
                }
                return children;
            });
            
        // Register FieldDeclNode handler
        register_handler<FieldDeclNode<Extra>>("FieldDecl", VAR_DECL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << OPERATOR_COLOR << node->access_modifier.getValue() << RESET
                       << (node->is_static ? " static" : "")
                       << " " << OPERATOR_COLOR << node->type.getValue() << RESET
                       << " " << BOLD_WHITE << node->identifier.getValue() << RESET;
            });
            
        // Register MethodDeclNode handler
        register_handler<MethodDeclNode<Extra>>("MethodDecl", BINARY_EXPR_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << OPERATOR_COLOR << node->access_modifier.getValue() << RESET
                       << (node->is_static ? " static" : "")
                       << " " << OPERATOR_COLOR << node->type.getValue() << RESET
                       << " " << BOLD_WHITE << node->identifier.getValue() << RESET
                       << "(" << node->param_list.size() << " params)";
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                for (const auto& param : node->param_list) {
                    if (param) children.push_back(param.get());
                }
                if (node->body) children.push_back(node->body.get());
                return children;
            });
            
        // Register ConstructorDeclNode handler
        register_handler<ConstructorDeclNode<Extra>>("ConstructorDecl", BINARY_EXPR_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << BOLD_WHITE << node->identifier.getValue() << RESET
                       << "(" << node->param_list.size() << " params)";
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                std::vector<ASTNode<Extra>*> children;
                for (const auto& param : node->param_list) {
                    if (param) children.push_back(param.get());
                }
                if (node->body) children.push_back(node->body.get());
                return children;
            });

        // ============ Supporting Nodes ============
        
        // Register ArgumentNode handler
        register_handler<ArgumentNode<Extra>>("Argument", LITERAL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)node; (void)indent; (void)isFirst; (void)isLast;
                // No additional details
            },
            [this](auto* node, const std::string& indent, bool isLast) -> std::vector<ASTNode<Extra>*> {
                (void)indent; (void)isLast;
                if (node->expr) {
                    return {node->expr.get()};
                }
                return {};
            });
            
        // Register ParamNode handler
        register_handler<ParamNode<Extra>>("Param", VAR_DECL_COLOR,
            [this](auto* node, const std::string& indent, bool isFirst, bool isLast) {
                (void)indent; (void)isFirst; (void)isLast;
                output << ": " << OPERATOR_COLOR << node->type.getValue() << RESET
                       << " " << BOLD_WHITE << node->identifier.getValue() << RESET;
            });
    }
    
    template<typename NodeType>
    void register_handler(
        const std::string& name, 
        const std::string& color,
        std::function<void(NodeType*, const std::string&, bool, bool)> detailsPrinter = nullptr,
        std::function<std::vector<ASTNode<Extra>*>(NodeType*, const std::string&, bool)> childGetter = nullptr) {
        
        handlers[std::type_index(typeid(NodeType))] = 
            [this, name, color, detailsPrinter, childGetter](
                ASTNode<Extra>* node, const std::string& indent, bool isFirst, bool isLast, std::ostream& os) {
                
                auto* typedNode = static_cast<NodeType*>(node);
                
                // Print the node header
                std::string marker = isFirst ? "" : isLast ? "└── " : "├── ";
                os << TREE_COLOR << indent << marker << RESET << color << name << RESET;
                
                // Print node-specific details
                if (detailsPrinter) {
                    detailsPrinter(typedNode, indent, isFirst, isLast);
                }
                os << std::endl;
                
                // Print children if any
                if (childGetter) {
                    std::string newIndent = indent + (isLast ? "    " : "│   ");
                    auto children = childGetter(typedNode, newIndent, isLast);
                    
                    for (size_t i = 0; i < children.size(); ++i) {
                        bool isLastChild = (i == children.size() - 1);
                        if (children[i]) {
                            print(children[i], newIndent, false, isLastChild);
                        }
                    }
                }
            };
    }
};

// Static color definitions
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::RESET = "\033[0m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::VAR_DECL_COLOR = "\033[1;34m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::BINARY_EXPR_COLOR = "\033[1;32m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::UNARY_EXPR_COLOR = "\033[1;33m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::LITERAL_COLOR = "\033[1;36m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::OPERATOR_COLOR = "\033[1;31m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::TREE_COLOR = "\033[90m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::BOLD_WHITE = "\033[1;37m";
template<typename Extra>
const std::string ReflectionASTPrinter<Extra>::PROGRAM_COLOR = "\033[1;35m";

// New reflection-based print function
void Log::print_ast_reflection(ASTNode<ParseExtra>* root) {
    ReflectionASTPrinter<ParseExtra> printer;
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
        ss << "  [" << std::setw(3) << i << "] " 
           << std::setw(15) << std::left << static_cast<int>(token.getType())
           << " '" << token.getValue() << "' "
           << "(" << token.getLine() << ":" << token.getCol() << ")";
        std::cout << ss.str() << std::endl;
    }
}
