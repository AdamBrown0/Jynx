#include "token.hh"
#include "log.hh"
#include <iostream>

void Token::print() {
    LOG_DEBUG("TokenType: {} Value: {} Line: {} Column: {}", this->to_string(), value, line, col);
}
