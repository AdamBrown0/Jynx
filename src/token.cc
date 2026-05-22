#include "token.hh"

#include "log.hh"

void Token::print() {
  LOG_DEBUG("TokenType: {} Value: {} Line: {} Column: {}", this->to_string(),
            value, getLine(), getCol());
}
