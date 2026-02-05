#ifndef DIAGNOSTICS_H_
#define DIAGNOSTICS_H_

#include <string>
#include <vector>

class Diagnostics {
 public:
  static Diagnostics &instance() {
    static Diagnostics instance;
    return instance;
  }

  Diagnostics(const Diagnostics &) = delete;
  Diagnostics &operator=(const Diagnostics &) = delete;

  void clear() {
    errors.clear();
    warnings.clear();
  }

  void report_error(const std::string &message) { errors.push_back(message); }
  void report_warning(const std::string &message) {
    warnings.push_back(message);
  }

  bool has_errors() const { return !errors.empty(); }
  size_t error_count() const { return errors.size(); }
  const std::vector<std::string> &get_errors() const { return errors; }
  const std::vector<std::string> &get_warnings() const { return warnings; }

 private:
  Diagnostics() = default;

  std::vector<std::string> errors;
  std::vector<std::string> warnings;
};

#endif  // DIAGNOSTICS_H_
