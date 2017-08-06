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
\brief Placeholder type for Attribute class.
*/
using Attribute = string;

/**
\brief Base exception class for ctf specific exceptions.
*/
class TranslationException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
\brief POD struct holding location coordinates.

Valid row and col numbers start at 1. Zero value row or col values are equal to
the invalid() constant.
**/
struct Location {
  /**
  \brief Row number. The lowest valid row number is 1.
  */
  uint64_t row;
  /**
  \brief Col number. The lowest valid col number is 1.
  */
  uint64_t col;
  /**
  \brief The name of the source of the location.
  */
  string fileName;

  /**
  \brief Basic constructor.

  \param[in] _row The row of the created object.
  \param[in] _col The col of the created object.
  \param[in] _fileName The name of the source file.
  */
  Location(uint64_t _row, uint64_t _col, string _fileName = "")
      : row(_row), col(_col), fileName(_fileName) {
    if (row == 0 || col == 0) {
      throw std::invalid_argument(
          "ctf::Location constructed with row or col with value 0.");
    }
  }
  /**
  \brief Implicit first location constructor.

  \param[in] _fileName The name of the source file.
  */
  Location(string _fileName = "") : row(1), col(1), fileName(_fileName) {}
  Location(const Location &) = default;
  Location(Location &&) = default;
  ~Location() = default;

  /**
  \brief Static constant invalid location object.

  \returns A const reference to the single invalid Location object.
  */
  static const Location &invalid() noexcept {
    static const Location ns{false};
    return ns;
  }

  Location &operator=(const Location &) = default;
  Location &operator=(Location &&) = default;
  /**
  \brief Compares two Location objects by row and col numbers.

  \param[in] lhs The left-hand side Location.
  \param[in] rhs The right-hand side Location.

  \returns True if both are invalid or when both have the same row and col.
  False
  otherwise.
  */
  friend bool operator==(const Location &lhs, const Location &rhs) {
    // Location::invalid comparison
    if ((lhs.row == 0 || lhs.col == 0) && (rhs.row == 0 || rhs.col == 0))
      return true;
    // regular comparison
    return lhs.row == rhs.row && lhs.col == rhs.col;
  }
  /**
  \brief Compares two Location objects by row and col numbers.

  \param[in] lhs The left-hand side Location.
  \param[in] rhs The right-hand side Location.

  \returns False if both are invalid or when both have the same row and col.
  True
  otherwise.
  */
  friend bool operator!=(const Location &lhs, const Location &rhs) {
    return !(lhs == rhs);
  }

  /**
  \brief Creates a string from this Location.

  \returns A string in the format "fileName:row:col"
  */
  string to_string() const {
    if (*this == Location::invalid()) {
      return "";
    }
    return fileName + ":" + std::to_string(row) + ":" + std::to_string(col);
  }

 private:
  /**
  \brief Constructs an invalid Location.
  */
  Location(bool) : row(0), col(0) {}
};

/**
\brief A single symbol in the translation process. May represent a Terminal,
Nonterminal or end of input.
*/
class Symbol {
 public:
  /**
  \brief Type of the Symbol.
  */
  enum class Type : unsigned char {
    /**
    \brief Undefined type
    */
    UNKNOWN,
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
    \brief Denotes a symbol with special meaning for translation control or
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
  Attribute attribute_;
  /**
  \brief Location of the origin of this Symbol.
  */
  Location location_;

 public:
  /**
  \brief Constructs a Symbol with a given type. If specified, sets Symbol's name
  and attribute.
  \param[in] type Type of constructed Symbol.
  \param[in] name Name of constructed Symbol. Defaults to "". "" is only valid
  for Type::EOI.
  \param[in] atr Attribute of constructed Symbol.
  */
  Symbol(Type type, const string &name = "", const Attribute &atr = "",
         const Location &loc = Location::invalid())
      : type_(type), name_(name), attribute_(atr), location_(loc) {
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
  Symbol(const string &name, const Attribute &atr = "",
         const Location &loc = Location::invalid())
      : Symbol(Type::UNKNOWN, name, atr, loc) {}
  /**
  \brief Default destructor.
  */
  ~Symbol() = default;

  /**
  \brief Creates an EOF Symbol.
  \returns An EOF Symbol.
  */
  static Symbol eof() { return Symbol(Type::EOI); }

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
  Attribute &attribute() { return attribute_; }
  /**
  \brief Returns a const reference to attribute.
  \returns A const reference to attribute.
  */
  const Attribute &attribute() const { return attribute_; }
  /**
  \brief Returns a const reference to type.
  \returns A const reference to type.
  */
  const Type &type() const { return type_; }
  /**
  \brief Returns the Symbol's location.
  \returns The Symbol's original location.
  */
  const Location &location() const { return location_; }

  /**
  \brief Merges symbol's attribute and sets location if not set.
  */
  void add_attribute(const Symbol &other) {
    // TODO change for future Attribute type
    attribute_ += other.attribute();
    if (location_.row == 0 || location_.col == 0)
      location_ = other.location();
  }

  /**
  \name Comparison operators
  \brief Numeric comparison of types and lexicographic comparison of names.
  Types have higher priority.
  \param[in] lhs Left Symbol of the comparison.
  \param[out] rhs Right Symbol of the comparison.
  \returns True when the lexicographic comparison is true.
  */
  ///@{
  friend bool operator<(const Symbol &lhs, const Symbol &rhs) {
    return lhs.type_ < rhs.type_ ||
           (lhs.type_ == rhs.type_ && lhs.name_ < rhs.name_);
  }

  friend bool operator==(const Symbol &lhs, const Symbol &rhs) {
    return lhs.type_ == rhs.type_ && lhs.name_ == rhs.name_;
  }

  friend bool operator!=(const Symbol &lhs, const Symbol &rhs) {
    return !(lhs == rhs);
  }

  friend bool operator>(const Symbol &lhs, const Symbol &rhs) {
    return rhs < lhs;
  }

  friend bool operator<=(const Symbol &lhs, const Symbol &rhs) {
    return lhs == rhs || lhs < rhs;
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
inline Symbol Terminal(const string &name, const Attribute &attribute = "",
                       const Location &loc = Location::invalid()) {
  return Symbol(Symbol::Type::TERMINAL, name, attribute, loc);
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