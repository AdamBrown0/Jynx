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
    
    void lexer_error(const std::string& message, int line, int col) {
        std::stringstream ss;
        ss << "Lexer error at line " << line << ", col " << col << ": " << message;
        Logger::error(ss.str());
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
    
    void parser_error(const std::string& message, const Token& token) {
        std::stringstream ss;
        ss << "Parser error at line " << token.getLine() << ", col " << token.getCol() 
           << ": " << message << " (found: '" << token.getValue() << "')";
        Logger::error(ss.str());
    }
    
    void semantic_error(const std::string& message, int line, int col) {
        std::stringstream ss;
        ss << "Semantic error at line " << line << ", col " << col << ": " << message;
        Logger::error(ss.str());
    }
}

// AST printing functions (enhanced from existing)
void Log::print_ast(ASTNode* root) {
    print_ast(root, "", true, false);
}

void Log::print_ast(ASTNode* root, std::string indent, bool isFirst, bool isLast) {
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
    if (auto varDecl = dynamic_cast<VarDeclNode*>(root)) {
        std::cout << VAR_DECL_COLOR << "VarDecl: " << RESET 
                  << BOLD_WHITE << varDecl->identifier.getValue() << std::endl;
        
        // Print initializer if it exists
        if (varDecl->initializer) {
            std::string newIndent = indent + (isFirst ? "" : "    ");
            print_ast(varDecl->initializer.get(), newIndent, false, true);
        }
    } else if (auto binaryExpr = dynamic_cast<BinaryExprNode*>(root)) {
        std::cout << BINARY_EXPR_COLOR << "BinaryExpr: " << RESET 
                  << OPERATOR_COLOR << "'" << binaryExpr->op.getValue() << "'" << RESET << std::endl;
        
        // Calculate new indent based on current position
        std::string newIndent = indent + (isLast ? "    " : "│   ");
        
        if (binaryExpr->left && binaryExpr->right) {
            // Both children exist
            print_ast(binaryExpr->left.get(), newIndent, false, false);
            print_ast(binaryExpr->right.get(), newIndent, false, true);
        } else if (binaryExpr->left) {
            // Only left child
            print_ast(binaryExpr->left.get(), newIndent, false, true);
        } else if (binaryExpr->right) {
            // Only right child
            print_ast(binaryExpr->right.get(), newIndent, false, true);
        }
    } else if (auto unaryExpr = dynamic_cast<UnaryExprNode*>(root)) {
        std::cout << UNARY_EXPR_COLOR << "UnaryExpr: " << RESET 
                  << OPERATOR_COLOR << "'" << unaryExpr->op.getValue() << "'" << RESET
                  << " operand: " << LITERAL_COLOR << "'" << unaryExpr->operand.getValue() << "'" << RESET << std::endl;
    } else if (auto literalExpr = dynamic_cast<LiteralExprNode*>(root)) {
        std::cout << LITERAL_COLOR << "Literal: " << RESET 
                  << "'" << BOLD_WHITE << literalExpr->literal_token.getValue() << "'" << std::endl;
    } else if (auto expr = dynamic_cast<ExprNode*>(root)) {
        std::cout << "Expression (generic)" << std::endl;
    } else {
        std::cout << "ASTNode (unknown type)" << std::endl;
    }
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
