/**
\file base.hpp
\brief Defines base formal language types used throughout this project.
\author Radek Vít
*/
#ifndef CTF_BASE_H
#define CTF_BASE_H

#include <any>
#include <ostream>
#include <stdexcept>

#include "generic_types.hpp"

namespace ctf {

/**
\brief Attribute class. Holds values of any type.
*/
class Attribute {
 private:
  /**
  \brief Stores any value.
  */
  std::any storage_;

 public:
  /**
  \brief Default copy constructor.
  */
  Attribute(const Attribute &) = default;
  /**
  \brief Constructs Attribute from a reference to any type.

  \tparam T The type of stored object.
  */
  template<typename T>
  Attribute(const T &arg): storage_(arg) {
  }
  /**
  \brief Constructs Attribute from a rvalue reference.

  \tparam T The type of stored object.
  */
  template<typename T>
  Attribute(T &&arg): storage_(arg) {
  }
  /**
  \brief Constructs Attribute by passing constructor arguments to std::any.

  \tparam Args Variadic arguments type to be passed to std::any constuctor.
  */
  template <typename... Args>
  Attribute(Args &&... args) : storage_(std::forward(args)...) {}

  /**
  \brief Default assignment operator.
  */
  Attribute &operator=(const Attribute &) = default;
  /**
  \brief Default assignment operator.
  */
  Attribute &operator=(Attribute &&) = default;
  /**
  \brief Assigns rhs to the Attribute object.

  \tparam T The type of the assigned object.
  */
  template <typename T>
  Attribute &operator=(T &rhs) {
    storage_ = rhs;
    return *this;
  }
  /**
  \brief Assigns rhs to the Attribute object.

  \tparam T The type of the assigned object.
  */
  template <typename T>
  Attribute &operator=(T &&rhs) {
    storage_ = rhs;
    return *this;
  }

  /**
  \brief Default destructor.
  */
  ~Attribute() = default;

  /**
  \brief Retreives a value from storage.

  \tparam T The type of the retreived object.

  \return The stored value.
  */
  template <typename T>
  T get() const {
    return std::any_cast<T>(storage_);
  }
  /**
  \brief Sets a value.

  \tparam T The type of the assigned value.

  \param[in] value A constant reference to the stored value.
  */
  template <typename T>
  void set(const T &value) {
    storage_.emplace(value);
  }
  /**
  \brief Sets a value.

  \tparam T The type of the assigned value.

  \param[in] value A rvalue reference to the stored value.
  */
  template <typename T>
  void set(T &&value) {
    storage_.emplace(value);
  }

  /**
  \brief Emplaces a value.

  \tparam T The type of stored object.
  \tparam Args The variadic arguments passed to the constructor of T.

  \param[in] args The arguments forwarded to std::any::emplace.

  \returns A reference to the emplaced object.
  */
  template <typename T, typename... Args>
  auto emplace(Args &&... args) {
    return storage_.emplace<T>(std::forward(args)...);
  }

  /**
  \brief Resets the stored value.
  */
  void clear() noexcept {
    storage_.reset();
  }
  /**
  \brief Staps the contents of an Attribute with another.

  \param[in/out] other The other Attribute to be swapped.
  */
  void swap(Attribute &other) { storage_.swap(other.storage_); }

  /**
  \brief Returns true if there is no value stored.

  \returns True when no value is stored in the Attribute.
  */
  bool empty() const noexcept { return !storage_.has_value(); }

  /**
  \brief Get the type info of the stored object.
  */
  const std::type_info &type() const noexcept { return storage_.type(); }

  /**
  \name Comparison operators
  \brief If compared to the same type, it compares the contents of Attribute to
  the other value.

  \returns True when the operands are of the same type and they are equal.
  */
  ///@{
  template <typename T>
  friend bool operator==(const Attribute &lhs, const T &rhs) {
    if (lhs.type() != typeid(rhs))
      return false;
    return lhs.get<T>() == rhs;
  }

  template <typename T>
  friend bool operator==(const T &lhs, const Attribute &rhs) {
    if (lhs.type() != typeid(rhs))
      return false;
    return lhs == rhs.get<T>();
  }
  ///@}
};

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
  Location(Location &&) noexcept = default;
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
  Location &operator=(Location &&) noexcept = default;
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
  Symbol(Type type, const string &name = "", const Attribute &atr = Attribute{},
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
  Symbol(const string &name, const Attribute &atr = Attribute{},
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
  void set_attribute(const Symbol &other) {
    // TODO change for future Attribute type
    attribute_ = other.attribute();
    if (location_ == Location::invalid())
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
inline Symbol Terminal(const string &name,
                       const Attribute &attribute = Attribute{},
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

namespace std {
inline void swap(ctf::Attribute &lhs, ctf::Attribute &rhs) noexcept {
  lhs.swap(rhs);
}
}  // namespace std

#endif
/*** End of file base.hpp ***/