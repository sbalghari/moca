#ifndef MOCA_UTILS_ERRORS_HPP
#define MOCA_UTILS_ERRORS_HPP

#include <stdexcept>
#include <string>

class MocaBaseException : public std::runtime_error {
  public:
    MocaBaseException(const std::string &msg) : std::runtime_error(msg) {}
};

class APIException : public MocaBaseException {
  public:
    using MocaBaseException::MocaBaseException;
};

class ToolException : public MocaBaseException {
  public:
    using MocaBaseException::MocaBaseException;
};

class ConfigException : public MocaBaseException {
  public:
    using MocaBaseException::MocaBaseException;
};

#endif // MOCA_UTILS_ERRORS_HPP