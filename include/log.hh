#ifndef LOG_H_
#define LOG_H_

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "sourcelocation.hh"

// Forward declarations to avoid circular includes
class Token;
template <typename Extra>
struct ASTNode;
struct ParseExtra;

namespace Log {
// Log levels
enum class Level { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3, FATAL = 4 };

// Log configuration
class Logger {
 private:
  static Level current_level;
  static bool show_timestamps;
  static bool show_colors;
  static std::ostream* output_stream;

 public:
  // Configuration methods
  static void set_level(Level level);
  static Level get_level();
  static void enable_timestamps(bool enable = true);
  static void enable_colors(bool enable = true);
  static void set_output(std::ostream& stream);

  // Core logging methods
  static void log(Level level, const std::string& message);
  static void debug(const std::string& message);
  static void info(const std::string& message);
  static void warn(const std::string& message);
  static void error(const std::string& message);
  static void fatal(const std::string& message);

  // Template methods for formatted logging
  template <typename... Args>
  static void debug(const std::string& format, Args&&... args);

  template <typename... Args>
  static void info(const std::string& format, Args&&... args);

  template <typename... Args>
  static void warn(const std::string& format, Args&&... args);

  template <typename... Args>
  static void error(const std::string& format, Args&&... args);

  template <typename... Args>
  static void fatal(const std::string& format, Args&&... args);

 private:
  static std::string get_timestamp();
  static std::string get_level_string(Level level);
  static std::string get_level_color(Level level);
  static std::string format_message(const std::string& format);

  template <typename T, typename... Args>
  static std::string format_message(const std::string& format, T&& value,
                                    Args&&... args);
};

// Template method implementations
template <typename... Args>
void Logger::debug(const std::string& format, Args&&... args) {
  if (current_level <= Level::DEBUG) {
    log(Level::DEBUG, format_message(format, std::forward<Args>(args)...));
  }
}

template <typename... Args>
void Logger::info(const std::string& format, Args&&... args) {
  if (current_level <= Level::INFO) {
    log(Level::INFO, format_message(format, std::forward<Args>(args)...));
  }
}

template <typename... Args>
void Logger::warn(const std::string& format, Args&&... args) {
  if (current_level <= Level::WARN) {
    log(Level::WARN, format_message(format, std::forward<Args>(args)...));
  }
}

template <typename... Args>
void Logger::error(const std::string& format, Args&&... args) {
  if (current_level <= Level::ERROR) {
    log(Level::ERROR, format_message(format, std::forward<Args>(args)...));
  }
}

template <typename... Args>
void Logger::fatal(const std::string& format, Args&&... args) {
  log(Level::FATAL, format_message(format, std::forward<Args>(args)...));
}

template <typename T, typename... Args>
std::string Logger::format_message(const std::string& format, T&& value,
                                   Args&&... args) {
  std::stringstream ss;
  ss << value;

  size_t pos = format.find("{}");
  if (pos != std::string::npos) {
    std::string result =
        format.substr(0, pos) + ss.str() + format.substr(pos + 2);
    if constexpr (sizeof...(args) > 0) {
      return format_message(result, std::forward<Args>(args)...);
    } else {
      return result;
    }
  }
  return format;
};

// Specialized logging functions for compiler components
namespace Compiler {
void lexer_token(const Token& token);
void lexer_error(const std::string& message, SourceLocation loc);
void parser_enter(const std::string& rule);
void parser_exit(const std::string& rule, bool success);
void parser_error(const std::string& message, const Token& token);
void semantic_error(const std::string& message, int line, int col);
}  // namespace Compiler

// AST printing functions (updated for templated AST)
void print_ast(ASTNode<ParseExtra>* root);
void print_ast(ASTNode<ParseExtra>* root, std::string indent, bool isFirst,
               bool isLast);
void print_ast_new(ASTNode<ParseExtra>* root, std::string indent, bool isFirst,
                   bool isLast);
void print_ast_reflection(ASTNode<ParseExtra>* root);

// Utility functions
void print_separator(const std::string& title = "");
void print_header(const std::string& title);
void print_tokens(const std::vector<Token>& tokens);
}  // namespace Log

// Convenience macros for easier logging
#define LOG_DEBUG(...) Log::Logger::debug(__VA_ARGS__)
#define LOG_INFO(...) Log::Logger::info(__VA_ARGS__)
#define LOG_WARN(...) Log::Logger::warn(__VA_ARGS__)
#define LOG_ERROR(...) Log::Logger::error(__VA_ARGS__)
#define LOG_FATAL(...) Log::Logger::fatal(__VA_ARGS__)

// Compiler-specific macros
#define LOG_TOKEN(token) Log::Compiler::lexer_token(token)
#define LOG_LEXER_ERROR(msg, loc) Log::Compiler::lexer_error(msg, loc)
#define LOG_PARSER_ENTER(rule) Log::Compiler::parser_enter(rule)
#define LOG_PARSER_EXIT(rule, success) Log::Compiler::parser_exit(rule, success)
#define LOG_PARSER_ERROR(msg, token) Log::Compiler::parser_error(msg, token)

#endif  // LOG_H_
