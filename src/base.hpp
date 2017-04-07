/**
\file base.hpp
\brief Defines base formal language types used throughout this project.
\author Radek Vít
*/
#ifndef CTF_BASE_H
#define CTF_BASE_H

#include <ostream>
#include <stdexcept>

#include "generic_types.hpp"

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
  /**
  \brief Type of the Symbol.
  */
  enum class Type {
    /**
    \brief Terminal symbol
    */
    TERMINAL,
    /**
    \brief Nonterminal symbol
    */
    NONTERMINAL,
    /**
    \brief End of input
    */
    EOI,
    /**
    \brief Undefined type
    */
    UNKNOWN,
    /**
    \brief Denotes a symbol with special meaning for the translation control or
    output generator.
    */
    SPECIAL,
  };

 protected:
  /**
  \brief Type of this Symbol.
  */
  Type type_;
  /**
  \brief Name of this Symbol.
  */
  string name_;
  /**
  \brief Attribute of this Symbol. Only valid for some types.
  */
  string attribute_;

 public:
  /**
  \brief Constructs a Symbol with a given type. If specified, sets Symbol's name
  and attribute.
  \param[in] type Type of constructed Symbol.
  \param[in] name Name of constructed Symbol. Defaults to "". "" is only valid
  for Type::EOI.
  \param[in] atr Attribute of constructed Symbol.
  */
  Symbol(Type type, const string &name = "", const string &atr = "")
      : type_(type), name_(name), attribute_(atr) {
    if (type != Symbol::Type::EOI && name == "")
      throw std::invalid_argument(
          "Empty name when constructing non-EOI Symbol.");
  }
  /**
  \brief Constructs a Symbol with unspecified type. Sets Symbol's name and if
  specified, sets attribute.
  \param[in] name Name of constructed Symbol.
  \param[in] atr Attribute of constructed Symbol. Defaults to "".
  */
  Symbol(const string &name, const string &atr = "")
      : Symbol(Type::UNKNOWN, name, atr) {}
  /**
\brief Default destructor.
*/
  ~Symbol() = default;

  /**
  \brief Creates an EOI Symbol.
  \returns An EOI Symbol.
  */
  static Symbol EOI() { return Symbol(Type::EOI); }

  /**
  \brief Returns a reference to name.
  \returns A reference to name.
  */
  string &name() { return name_; }
  /**
\brief Returns a const reference to name.
\returns A const reference to name.
*/
  const string &name() const { return name_; }
  /**
\brief Returns a reference to attribute.
\returns A reference to attribute.
*/
  string &attribute() { return attribute_; }
  /**
\brief Returns a const reference to attribute.
\returns A const reference to attribute.
*/
  const string &attribute() const { return attribute_; }
  /**
\brief Returns a reference to type.
\returns A reference to type.
*/
  Type &type() { return type_; }
  /**
\brief Returns a const reference to type.
\returns A const reference to type.
*/
  const Type &type() const { return type_; }

  /**
  \name Comparison operators
  \brief Lexicographic comparison of Symbol names.
  \param[in] lhs Left Symbol of the comparison.
  \param[out] rhs Right Symbol of the comparison.
  \returns True when the lexicographic comparison is true.
  */
  ///@{
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
    return rhs <= lhs;
  }
  ///@}
};
/**
\brief Returns a Symbol with Type::Terminal, given name and attribute.
\param[in] name Name of returned symbol.
\param[in] attribute Attribute of returned Symbol. Defaults to "".
\returns A Symbol with type Terminal, given name and given attribute.
*/
inline Symbol Terminal(const string &name, const string &attribute = "") {
  return Symbol(Symbol::Type::TERMINAL, name, attribute);
}
/**
\brief Returns a Symbol with Type::Nonterminal, given name and attribute.
\param[in] name Name of returned symbol.
\returns A Symbol with type Nonterminal and given name.
*/
inline Symbol Nonterminal(const string &name) {
  return Symbol(Symbol::Type::NONTERMINAL, name);
}
#ifndef CTF_NO_QUOTE_OPERATORS
inline namespace literals {
/**
\brief Returns a Symbol of Type::Terminal with given name.
\param[in] s C string representing the name of the returned Symbol.
\returns Symbol with type Terminal and given name.
*/
inline Symbol operator""_t(const char *s, size_t) { return Terminal({s}); }
/**
\brief Returns a Symbol of Type::Nonterminal with given name.
\param[in] s C string representing the name of the returned Symbol.
\returns Symbol with type Terminal and given name.
*/
inline Symbol operator""_nt(const char *s, size_t) { return Nonterminal({s}); }
/**
\brief Returns a Symbol of Type::Special with given name.
\param[in] s C string representing the name of the returned Symbol.
\returns Symbol with type Special and given name.
*/
inline Symbol operator""_s(const char *s, size_t) {
  return Symbol(Symbol::Type::SPECIAL, {s});
}
}  // inline namespace literals
#endif

}  // namespace ctf

#endif
/*** End of file base.hpp ***/