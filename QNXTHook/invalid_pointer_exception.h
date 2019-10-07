#pragma once

#include <stdexcept>
#include <typeinfo>

class InvalidPointerException : public std::runtime_error {
 public:
  InvalidPointerException(const std::string message)
      : std::runtime_error(message) {}

  const char *what() noexcept { return std::runtime_error::what(); }

 private:
};