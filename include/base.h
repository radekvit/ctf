/**
\file base.h
\brief Defines base formal language types used throughout this project.
\author Radek Vít
*/
#ifndef CTF_BASE_H
#define CTF_BASE_H

#include <generic_types.h>
#include <ostream>
#include <stdexcept>

namespace ctf {

/**
\brief Base exception class for project specific exceptions.
*/
class TranslationException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
\brief Symbol, may represent a Terminal, Nonterminal or end of input.
*/
class Symbol {
 public:
  enum class Type {
    TERMINAL,
    NONTERMINAL,
    EOI,
    UNKNOWN,
  };

 protected:
  Type type_;
  string name_;
  string attribute_;

 public:
  Symbol(Type type, const string &name = "", const string &atr = "")
      : type_(type), name_(name), attribute_(atr) {
    if (type != Symbol::Type::EOI && name == "")
      throw std::invalid_argument(
          "Empty name when constructing non-EOI Symbol.");
  }
  Symbol(const string &name, const string &atr = "")
      : Symbol(Type::UNKNOWN, name, atr) {}
  ~Symbol() = default;

  static Symbol EOI() { return Symbol(Type::EOI); }

  string &name() { return name_; }
  const string &name() const { return name_; }
  string &attribute() { return attribute_; }
  const string &attribute() const { return attribute_; }
  Type &type() { return type_; }
  const Type &type() const { return type_; }

  friend bool operator<(const Symbol &lhs, const Symbol &rhs) {
    return lhs.name_ < rhs.name_;
  }

  friend bool operator==(const Symbol &lhs, const Symbol &rhs) {
    return lhs.name_ == rhs.name_;
  }

  friend bool operator!=(const Symbol &lhs, const Symbol &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator>(const Symbol &lhs, const Symbol &rhs) {
    return rhs < lhs;
  }

  friend bool operator<=(const Symbol &lhs, const Symbol &rhs) {
    return lhs < rhs || lhs == rhs;
  }

  friend bool operator>=(const Symbol &lhs, const Symbol &rhs) {
    return lhs > rhs || lhs == rhs;
  }
};

inline Symbol Terminal(const string &name, const string &attribute = "") {
  return Symbol(Symbol::Type::TERMINAL, name, attribute);
}
inline Symbol Nonterminal(const string &name) {
  return Symbol(Symbol::Type::NONTERMINAL, name);
}
inline Symbol operator ""_t(const char *s, size_t) {
  return Terminal({s});
}
inline Symbol operator ""_nt(const char *s, size_t) {
  return Nonterminal({s});
}

}  // namespace ctf

#endif
/*** End of file base.h ***/