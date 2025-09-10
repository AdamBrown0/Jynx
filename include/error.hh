#ifndef ERROR_H_
#define ERROR_H_

#include <string>
#define ERROR(msg, loc) \
  LOG_ERROR("ERROR: {} at {}", msg, loc)

namespace Error {

}

#endif  // ERROR_H_
