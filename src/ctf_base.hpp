/**
\file base.hpp
\brief Defines base formal language types used throughout this project.
\author Radek Vít
*/
#ifndef CTF_BASE_H
#define CTF_BASE_H

#include <any>
#include <mutex>
#include <ostream>
#include <stdexcept>

#include "ctf_generic_types.hpp"

namespace ctf {
/**
\brief Base exception class for ctf specific exceptions.
*/
class TranslationException : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
\brief A single symbol in the translation process. May represent a Terminal, Nonterminal or end of
input.
*/
class Symbol {
 public:
  /**
  \brief Type of the Symbol.
  */
  enum class Type : unsigned char {
    /**
    \brief Nonterminal symbol
    */
    NONTERMINAL = 0,
    /**
    \brief Terminal symbol
    */
    TERMINAL = 1,
    /**
    \brief End of input
    */
    EOI = 3,
  };

  using enum_type = size_t;

  /**
  \brief Default destructor.
  */
  ~Symbol() = default;

  /**
  \brief Returns a Symbol with Type::Terminal, given name and attribute.
  \param[in] name Name of returned symbol.
  \param[in] attribute Attribute of returned Symbol. Defaults to "".
  \returns A Symbol with type Terminal, given name and given attribute.
  */
  friend constexpr Symbol Terminal(size_t id) noexcept;

  /**
  \brief Returns a Symbol with Type::Nonterminal, given name and attribute.
  \param[in] name Name of returned symbol.
  \returns A Symbol with type Nonterminal and given name.
  */
  friend constexpr Symbol Nonterminal(const size_t id) noexcept;

  /**
  \brief Creates an EOF Symbol.
  \returns An EOF Symbol.
  */
  static constexpr Symbol eof() noexcept { return Symbol(Type::EOI); }

  /**
  \brief Returns the type of the symbol.
  \returns The symbol's type.
  */
  constexpr Type type() const noexcept { return Type((storage_ & type_mask()) >> type_shift()); }

  constexpr size_t id() const noexcept { return storage_ & id_mask(); }

  /**
  \brief Returns true if the symbol is a terminal and false otherwise.

  Symbol::eof() is a terminal symbol as well.
  */
  constexpr bool terminal() const noexcept {
    return storage_ & (static_cast<size_t>(0x1) << type_shift());
  }
  /**
  \brief Returns true if the symbol is a nonterminal.
  */
  constexpr bool nonterminal() const noexcept { return (storage_ & type_mask()) == 0; }

  /**
  \name Comparison operators
  \brief Numeric comparison of types and ids.
  Types have higher priority.
  \param[in] lhs Left Symbol of the comparison.
  \param[out] rhs Right Symbol of the comparison.
  \returns True when the numeric comparison is true.
  */
  ///@{
  friend constexpr bool operator<(const Symbol& lhs, const Symbol& rhs) {
    static_assert(sizeof(Symbol) == sizeof(size_t), "Symbol must match size_t size");
    return lhs.storage_ < rhs.storage_;
  }

  friend constexpr bool operator==(const Symbol& lhs, const Symbol& rhs) {
    return lhs.storage_ == rhs.storage_;
  }

  friend constexpr bool operator!=(const Symbol& lhs, const Symbol& rhs) { return !(lhs == rhs); }

  friend constexpr bool operator>(const Symbol& lhs, const Symbol& rhs) { return rhs < lhs; }

  friend constexpr bool operator<=(const Symbol& lhs, const Symbol& rhs) { return !(lhs > rhs); }

  friend constexpr bool operator>=(const Symbol& lhs, const Symbol& rhs) { return rhs <= lhs; }
  ///@}

  string to_string() const {
    if (type() == Type::EOI) {
      return "EOF";
    }
    return std::to_string(id() - (terminal() ? 1 : 0)) +
           (type() == Type::NONTERMINAL ? "_nt" : "_t");
  }

  explicit operator std::string() const { return to_string(); }

  explicit constexpr operator size_t() { return reinterpret_cast<const size_t&>(*this); }

 protected:
  constexpr Symbol(Type type, size_t id = 0) noexcept
      : storage_((static_cast<size_t>(type) << type_shift()) | (id & id_mask())) {}

  /**
  \brief Id of this Symbol.
  */
  size_t storage_;

  static constexpr size_t id_mask() noexcept {
    return (std::numeric_limits<size_t>::max() << 2) >> 2;
  }
  static constexpr size_t type_mask() noexcept { return ~id_mask(); }
  static constexpr size_t type_shift() noexcept { return sizeof(size_t) * 8 - 2; }
};

/**
  \brief Returns a Symbol with Type::Terminal, given name and attribute.
  \param[in] name Name of returned symbol.
  \param[in] attribute Attribute of returned Symbol. Defaults to "".
  \returns A Symbol with type Terminal, given name and given attribute.
  */
inline constexpr Symbol Terminal(size_t id) noexcept {
  return Symbol(Symbol::Type::TERMINAL, id + 1);
}

/**
\brief Returns a Symbol with Type::Nonterminal, given name and attribute.
\param[in] name Name of returned symbol.
\returns A Symbol with type Nonterminal and given name.
*/
inline constexpr Symbol Nonterminal(const size_t id) noexcept {
  return Symbol(Symbol::Type::NONTERMINAL, id);
}

/**
\brief POD struct holding location coordinates.

Valid row and col numbers start at 1. Zero value row or col values are equal to the invalid()
constant.
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
    assert(row != 0);
    assert(col != 0);
  }
  /**
  \brief Implicit first location constructor.

  \param[in] _fileName The name of the source file.
  */
  explicit Location(string _fileName = "") : row(1), col(1), fileName(_fileName) {}
  Location(const Location&) = default;
  Location(Location&&) noexcept = default;
  ~Location() = default;

  /**
  \brief Static constant invalid location object.

  \returns A const reference to the single invalid Location object.
  */
  static const Location& invalid() noexcept {
    static const Location ns{false};
    return ns;
  }

  Location& operator=(const Location&) & = default;
  Location& operator=(Location&&) & noexcept = default;
  /**
  \brief Compares two Location objects by row and col numbers.

  \param[in] lhs The left-hand side Location.
  \param[in] rhs The right-hand side Location.

  \returns True if both are invalid or when both have the same row and col.
  False
  otherwise.
  */
  friend bool operator==(const Location& lhs, const Location& rhs) {
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
  friend bool operator!=(const Location& lhs, const Location& rhs) { return !(lhs == rhs); }

  /**
  \brief Creates a string from this Location.

  \returns A string in the format "fileName:row:col" if the location is valid.
  */
  string to_string() const {
    if (*this == Location::invalid()) {
      return "";
    }
    return fileName + ":" + std::to_string(row) + ":" + std::to_string(col);
  }

  explicit operator string() const { return to_string(); }

  friend std::ostream& operator<<(std::ostream& os, const Location& location) {
    os << location.to_string();
    return os;
  }

 private:
  /**
  \brief Constructs an invalid Location.
  */
  explicit Location(bool) : row(0), col(0) {}
};

/**
\brief Attribute class. Holds values of any type.
*/
class Attribute {
 public:
  constexpr Attribute() = default;
  /**
  \brief Default copy constructor.
  */
  Attribute(const Attribute&) = default;
  /**
  \brief Default copy constructor.
  */
  Attribute(Attribute&&) noexcept = default;
  /**
  \brief Constructs Attribute from a reference to T.

  \tparam T The type of stored object.
  */
  template <
      typename T,
      typename = typename std::enable_if<
          !std::is_same<typename std::remove_reference<T>::type, Attribute>::value &&
          !std::is_same<typename std::remove_reference<T>::type, const Attribute>::value>::type>
  explicit Attribute(T&& arg) : storage_(arg) {}

  /**
  \brief Default assignment operator.
  */
  Attribute& operator=(const Attribute& other) & {
    storage_ = other.storage_;
    return *this;
  }
  /**
  \brief Default assignment operator.
  */
  Attribute& operator=(Attribute&& other) & {
    storage_ = other.storage_;
    return *this;
  }
  /**
  \brief Assigns rhs to the Attribute object.

  \tparam T The type of the assigned object.
  */
  template <typename T>
  Attribute& operator=(T& rhs) & {
    storage_ = rhs;
    return *this;
  }
  /**
  \brief Assigns rhs to the Attribute object.

  \tparam T The type of the assigned object.
  */
  template <typename T>
  Attribute& operator=(T&& rhs) {
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
  void set(const T& value) {
    storage_.emplace(value);
  }
  /**
  \brief Sets a value.

  \tparam T The type of the assigned value.

  \param[in] value A rvalue reference to the stored value.
  */
  template <typename T>
  void set(T&& value) {
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
  auto emplace(Args&&... args) {
    return storage_.emplace<T>(std::forward(args)...);
  }

  /**
  \brief Resets the stored value.
  */
  void clear() noexcept { storage_.reset(); }
  /**
  \brief Staps the contents of an Attribute with another.

  \param[in/out] other The other Attribute to be swapped.
  */
  void swap(Attribute& other) { storage_.swap(other.storage_); }

  /**
  \brief Returns true if there is no value stored.

  \returns True when no value is stored in the Attribute.
  */
  bool empty() const noexcept { return !storage_.has_value(); }

  /**
  \brief Get the type info of the stored object.
  */
  const std::type_info& type() const noexcept { return storage_.type(); }

  /**
  \name Comparison operators
  \brief If compared to the same type, it compares the contents of Attribute to the other value.

  \returns True when the operands are of the same type and they are equal.
  */
  ///@{
  template <typename T>
  friend bool operator==(const Attribute& lhs, const T& rhs) {
    if (lhs.type() != typeid(rhs))
      return false;
    return lhs.get<T>() == rhs;
  }

  template <typename T>
  friend bool operator==(const T& lhs, const Attribute& rhs) {
    if (lhs.type() != typeid(rhs))
      return false;
    return lhs == rhs.get<T>();
  }
  ///@}

 private:
  /**
  \brief Stores any value.
  */
  std::any storage_;
};

class Token {
 public:
  Token(const Symbol symbol,
        const Attribute& atr = Attribute{},
        const Location& loc = Location::invalid())
      : symbol_(symbol), attribute_(atr), location_(loc) {}
  /**
  \brief Merges symbol's attribute and sets location if not set.
  */
  void set_attribute(const Token& other) {
    attribute_ = other.attribute();
    if (location_ == Location::invalid())
      location_ = other.location();
  }

  Symbol& symbol() noexcept { return symbol_; }
  const Symbol& symbol() const noexcept { return symbol_; }

  size_t id() const noexcept { return symbol_.id(); }
  Symbol::Type type() const noexcept { return symbol_.type(); }

  bool terminal() const noexcept { return symbol_.terminal(); }
  bool nonterminal() const noexcept { return symbol_.nonterminal(); }

  /**
  \brief Returns a reference to attribute.
  \returns A reference to attribute.
  */
  Attribute& attribute() { return attribute_; }
  /**
  \brief Returns a const reference to attribute.
  \returns A const reference to attribute.
  */
  const Attribute& attribute() const { return attribute_; }

  /**
  \brief Returns the Token's location.
  \returns The Token's original location.
  */
  const Location& location() const { return location_; }

  friend bool operator<(const Token& lhs, const Token& rhs) { return lhs.symbol() < rhs.symbol(); }

  friend bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.symbol() == rhs.symbol();
  }

  friend bool operator!=(const Token& lhs, const Token& rhs) { return !(lhs == rhs); }

  friend bool operator>(const Token& lhs, const Token& rhs) { return rhs < lhs; }

  friend bool operator<=(const Token& lhs, const Token& rhs) { return !(lhs > rhs); }

  friend bool operator>=(const Token& lhs, const Token& rhs) { return rhs <= lhs; }

  string to_string() const {
    string result;
    if (location() != Location::invalid()) {
      result = location().to_string() + ": ";
    }
    result += symbol_.to_string();
    return result;
  }

  explicit operator string() const { return to_string(); }

  explicit operator Symbol() const { return symbol(); }

 private:
  Symbol symbol_;
  /**
  \brief Attribute of this Symbol. Only valid for some types of symbols.
  */
  Attribute attribute_;
  /**
  \brief Location of the origin of this Token.
  */
  Location location_;
};

#ifndef CTF_NO_LITERALS
inline namespace literals {
/**
\brief Returns a Symbol of Type::Terminal with given name.
\param[in] s C string representing the name of the returned Symbol.
\returns Symbol with type Terminal and given name.
*/
inline constexpr Symbol operator""_t(unsigned long long int id) { return Terminal(id); }
/**
\brief Returns a Symbol of Type::Nonterminal with given name.
\param[in] s C string representing the name of the returned Symbol.
\returns Symbol with type Terminal and given name.
*/
inline constexpr Symbol operator""_nt(unsigned long long int id) { return Nonterminal(id); }
}  // namespace literals
#endif

class TerminalSet : public bit_set {
 public:
  explicit TerminalSet(size_t bits) : bit_set(bits) {}
  TerminalSet(size_t bits, std::initializer_list<Symbol> il) : bit_set(bits) {
    for (auto& symbol : il) {
      insert(symbol);
    }
  }
  TerminalSet(const bit_set& s) : bit_set(s) {}
  TerminalSet(bit_set&& s) : bit_set(std::move(s)) {}

  struct InsertResult {
    reference p;
    bool inserted;
  };

  InsertResult insert(Symbol s) noexcept {
    reference p = (*this)[s];
    bool inserted = ~p;
    p = true;
    return {p, inserted};
  }

  bool operator[](size_t i) const noexcept {
    return ((storage_[i / bitsPerStorage]) >> (bitsPerStorage - (i % bitsPerStorage + 1))) & 0x1;
  }
  reference operator[](size_t i) noexcept { return get_reference(i); }

  reference operator[](Symbol s) noexcept { return (*this)[s.id()]; }
  bool operator[](Symbol s) const noexcept { return (*this)[s.id()]; }

  vector<Symbol> symbols() const {
    vector<Symbol> result;
    if (capacity() == 0)
      return result;
    result.reserve(capacity());
    if ((*this)[0]) {
      result.push_back(Symbol::eof());
    }
    for (size_t i = 1; i < capacity(); ++i) {
      if ((*this)[i]) {
        result.push_back(Terminal(i - 1));
      }
    }
    return result;
  }

  string to_string(string (*string_fn)(Symbol s) = [](Symbol s) { return s.to_string(); }) const {
    auto terminals = symbols();
    if (terminals.empty()) {
      return "{}";
    }
    string result = "{ ";
    for (Symbol symbol : terminals) {
      result += string_fn(symbol) + ", ";
    }
    result.pop_back();
    result.pop_back();
    result += " }";
    return result;
  }
};
}  // namespace ctf

namespace std {
inline void swap(ctf::Attribute& lhs, ctf::Attribute& rhs) noexcept { lhs.swap(rhs); }

template <>
struct hash<ctf::Symbol> {
  size_t operator()(const ctf::Symbol& s) const noexcept {
    // reinterpret as a size_t
    return std::hash<size_t>{}(reinterpret_cast<const size_t&>(s));
  }
};
}  // namespace std

#endif
/*** End of file base.hpp ***/
