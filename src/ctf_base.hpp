/**
\file ctf_base.hpp
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
\brief A single symbol. May represent a Terminal, Nonterminal or end of input.
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

  /**
  \brief Default destructor.
  */
  ~Symbol() = default;

  /**
  \brief Returns a Symbol with Type::Terminal and a given id.
  \param[in] id The id - 1 of the created terminal.
  \returns A Symbol with type Terminal and a given id.
  */
  friend constexpr Symbol Terminal(std::size_t id) noexcept;

  /**
  \brief Returns a Symbol with Type::Nonterminal and a given id.
  \param[in] id The id of the nonterminal.
  \returns A Symbol with type Nonterminal and a given id.
  */
  friend constexpr Symbol Nonterminal(const std::size_t id) noexcept;

  /**
  \brief Creates an EOF Symbol.
  \returns An EOF Symbol.
  */
  static constexpr Symbol eof() noexcept { return Symbol(Type::EOI); }

  /**
  \brief Returns the type of the symbol.
  \returns The symbol's type.
  */
  constexpr Type type() const noexcept { return Type((_storage & type_mask()) >> type_shift()); }

  /**
  \brief Returns the id of the symbol.
  \returns The symbol's id.
  */
  constexpr std::size_t id() const noexcept { return _storage & id_mask(); }

  /**
  \brief Returns true if the symbol is a terminal and false otherwise.

  Symbol::eof() is a terminal symbol as well.
  */
  constexpr bool terminal() const noexcept {
    return _storage & (static_cast<std::size_t>(0x1) << type_shift());
  }
  /**
  \brief Returns true if the symbol is a nonterminal.
  */
  constexpr bool nonterminal() const noexcept { return (_storage & type_mask()) == 0; }

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
    return lhs._storage < rhs._storage;
  }

  friend constexpr bool operator==(const Symbol& lhs, const Symbol& rhs) {
    return lhs._storage == rhs._storage;
  }

  friend constexpr bool operator!=(const Symbol& lhs, const Symbol& rhs) { return !(lhs == rhs); }

  friend constexpr bool operator>(const Symbol& lhs, const Symbol& rhs) { return rhs < lhs; }

  friend constexpr bool operator<=(const Symbol& lhs, const Symbol& rhs) { return !(lhs > rhs); }

  friend constexpr bool operator>=(const Symbol& lhs, const Symbol& rhs) { return rhs <= lhs; }
  ///@}

  /**
  \brief Returns the basic string representation of the symbol.
  */
  string to_string() const {
    if (type() == Type::EOI) {
      return "EOF";
    }
    return std::to_string(id() - (terminal() ? 1 : 0)) +
           (type() == Type::NONTERMINAL ? "_nt" : "_t");
  }
  /**
  \brief Returns the basic string representation of the symbol.
  */
  explicit operator std::string() const { return to_string(); }
  /**
  \brief Returns the basic numeric representation of the symbol's id.
  */
  explicit constexpr operator std::size_t() { return _storage; }

 protected:
  constexpr Symbol(Type type, std::size_t id = 0) noexcept
    : _storage((static_cast<std::size_t>(type) << type_shift()) | (id & id_mask())) {}

  /**
  \brief Id of this Symbol.
  */
  std::size_t _storage;

  static constexpr std::size_t id_mask() noexcept {
    return (std::numeric_limits<std::size_t>::max() << 2) >> 2;
  }
  static constexpr std::size_t type_mask() noexcept { return ~id_mask(); }
  static constexpr std::size_t type_shift() noexcept { return sizeof(std::size_t) * 8 - 2; }
};

/**
  \brief Returns a terminal with a specific id with an offset.
  \param[in] id A numerical identifier of the symbol.
  \returns A Symbol with type Terminal and given id + 1.
  */
inline constexpr Symbol Terminal(std::size_t id) noexcept {
  return Symbol(Symbol::Type::TERMINAL, id + 1);
}

/**
\brief Returns a nonterminal with a given id.
\param[in] id Id of the returned symbol.
\returns A Symbol with type Nonterminal and a given id.
*/
inline constexpr Symbol Nonterminal(const std::size_t id) noexcept {
  return Symbol(Symbol::Type::NONTERMINAL, id);
}

using symbol_string_fn = string (*)(Symbol s);

inline string to_string(Symbol s) { return s.to_string(); }
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
  template <typename T,
            typename = typename std::enable_if<
              !std::is_same<typename std::remove_reference<T>::type, Attribute>::value &&
              !std::is_same<typename std::remove_reference<T>::type, const Attribute>::value>::type>
  explicit Attribute(T&& arg) : _storage(arg) {}

  /**
  \brief Default assignment operator.
  */
  Attribute& operator=(const Attribute& other) & {
    _storage = other._storage;
    return *this;
  }
  /**
  \brief Default assignment operator.
  */
  Attribute& operator=(Attribute&& other) & {
    _storage = other._storage;
    return *this;
  }
  /**
  \brief Assigns rhs to the Attribute object.

  \tparam T The type of the assigned object.
  */
  template <typename T>
  Attribute& operator=(T& rhs) & {
    _storage = rhs;
    return *this;
  }
  /**
  \brief Assigns rhs to the Attribute object.

  \tparam T The type of the assigned object.
  */
  template <typename T>
  Attribute& operator=(T&& rhs) {
    _storage = rhs;
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
    return std::any_cast<T>(_storage);
  }
  /**
  \brief Sets a value.

  \tparam T The type of the assigned value.

  \param[in] value A constant reference to the stored value.
  */
  template <typename T>
  void set(const T& value) {
    _storage.emplace(value);
  }
  /**
  \brief Sets a value.

  \tparam T The type of the assigned value.

  \param[in] value A rvalue reference to the stored value.
  */
  template <typename T>
  void set(T&& value) {
    _storage.emplace(value);
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
    return _storage.emplace<T>(std::forward(args)...);
  }

  /**
  \brief Resets the stored value.
  */
  void clear() noexcept { _storage.reset(); }
  /**
  \brief Staps the contents of an Attribute with another.

  \param [in,out] other The other Attribute to be swapped.
  */
  void swap(Attribute& other) { _storage.swap(other._storage); }

  /**
  \brief Returns true if there is no value stored.

  \returns True when no value is stored in the Attribute.
  */
  bool empty() const noexcept { return !_storage.has_value(); }

  /**
  \brief Get the type info of the stored object.
  */
  const std::type_info& type() const noexcept { return _storage.type(); }

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
  std::any _storage;
};

/**
\brief A single token. Contains a symbol, its location and its attribute.
*/
class Token {
 public:
  /**
  \brief Constructs Token from a symbol, its location, and its attribute.
  */
  Token(const Symbol symbol,
        const Attribute& atr = Attribute{},
        const Location& loc = Location::invalid())
    : _symbol(symbol), _attribute(atr), _location(loc) {}
  /**
  \brief Merges symbol's attribute and sets location if not set.
  */
  void set_attribute(const Token& other) {
    _attribute = other.attribute();
    if (_location == Location::invalid())
      _location = other.location();
  }
  /**
  \brief Get the represented symbol.

  \returns A reference to the represented symbol.
  */
  Symbol& symbol() noexcept { return _symbol; }
  /**
  \brief Get the represented symbol.

  \returns A const reference to the represented symbol.
  */
  const Symbol& symbol() const noexcept { return _symbol; }

  /**
  \brief Get the represented symbol's id.
  */
  std::size_t id() const noexcept { return _symbol.id(); }
  /**
  \brief Get the represented symbol's type.
  */
  Symbol::Type type() const noexcept { return _symbol.type(); }
  /**
  \brief Returns true if the represented symbol is a terminal.
  */
  bool terminal() const noexcept { return _symbol.terminal(); }
  /**
  \brief Returns true if the represented symbol is a nonterminal.
  */
  bool nonterminal() const noexcept { return _symbol.nonterminal(); }

  /**
  \brief Returns a reference to attribute.
  \returns A reference to attribute.
  */
  Attribute& attribute() { return _attribute; }
  /**
  \brief Returns a const reference to attribute.
  \returns A const reference to attribute.
  */
  const Attribute& attribute() const { return _attribute; }

  /**
  \brief Returns the Token's location.
  \returns The Token's original location.
  */
  const Location& location() const { return _location; }

  /**
  \name Comparison operators for the represented symbols.
  \brief Numeric comparison of types and ids.
  Types have higher priority.
  \param[in] lhs Left Symbol of the comparison.
  \param[out] rhs Right Symbol of the comparison.
  \returns True when the numeric comparison is true.
  */
  ///@{
  friend bool operator<(const Token& lhs, const Token& rhs) { return lhs.symbol() < rhs.symbol(); }

  friend bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.symbol() == rhs.symbol();
  }

  friend bool operator!=(const Token& lhs, const Token& rhs) { return !(lhs == rhs); }

  friend bool operator>(const Token& lhs, const Token& rhs) { return rhs < lhs; }

  friend bool operator<=(const Token& lhs, const Token& rhs) { return !(lhs > rhs); }

  friend bool operator>=(const Token& lhs, const Token& rhs) { return rhs <= lhs; }
  ///@}

  /**
  \brief Returns the string representation of the token.
  Prepends the location to the string representation of the token.

  \param[in] to_str The function for string representaton of symbols.
  */
  string to_string(symbol_string_fn to_str = ctf::to_string) const {
    string result;
    if (location() != Location::invalid()) {
      result = location().to_string() + ": ";
    }
    result += to_str(_symbol);
    return result;
  }
  /**
  \brief Returns the string representation of the token.
  Prepends the location to the string representation of the token.
  */
  explicit operator string() const { return to_string(); }
  /**
  \brief Converts the token to a symbol.
  */
  explicit operator Symbol() const { return symbol(); }

 private:
  /**
  \brief The represented Symbol.
  */
  Symbol _symbol;
  /**
  \brief Attribute of this Token. Only valid for some types of symbols.
  */
  Attribute _attribute;
  /**
  \brief Location of the origin of this Token.
  */
  Location _location;
};

#ifndef CTF_NO_LITERALS
inline namespace literals {
/**
\brief Returns a Symbol of Type::Terminal with given name.
\param[in] id The id - 1 of the created terminal.
\returns Symbol with type Terminal and given name.
*/
inline constexpr Symbol operator""_t(unsigned long long int id) { return Terminal(id); }
/**
\brief Returns a Symbol of Type::Nonterminal with given name.
\param[in] id The id of the created nonterminal.
\returns Symbol with type Terminal and given name.
*/
inline constexpr Symbol operator""_nt(unsigned long long int id) { return Nonterminal(id); }
}  // namespace literals
#endif

/**
\brief A specialization of ctf::bit_set.
Provides extra methods for interfacing with Symbols.
*/
class TerminalSet : public bit_set {
 public:
  /**
  \brief Constructs the terminal set with a set storage size.
  */
  explicit TerminalSet(std::size_t bits) : bit_set(bits) {}
  /**
  \brief Constructs the terminal set with a set storage size and inserts the supplied terminals to
  the set.
  */
  TerminalSet(std::size_t bits, std::initializer_list<Symbol> il) : bit_set(bits) {
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
  /**
  \brief Inserts a terminal symbol to the set.
  */
  InsertResult insert(Symbol s) noexcept {
    reference p = (*this)[s];
    bool inserted = ~p;
    p = true;
    return {p, inserted};
  }
  /**
  \brief Index into the set and get a reference to the membership of an id.
  */
  reference operator[](std::size_t i) noexcept { return get_reference(i); }
  /**
  \brief Index into the const set and get the membership value of an id.
  */
  bool operator[](std::size_t i) const noexcept {
    return ((_storage[i / bitsPerStorage]) >> (bitsPerStorage - (i % bitsPerStorage + 1))) & 0x1;
  }

  /**
  \brief Index into the set and get a reference to the membership of a terminal.
  */
  reference operator[](Symbol s) noexcept { return (*this)[s.id()]; }
  /**
  \brief Index into the const set and get the membership value of a terminal.
  */
  bool operator[](Symbol s) const noexcept { return (*this)[s.id()]; }

  /**
  \brief Get a vector of all symbols that are members of the set.
  */
  vector<Symbol> symbols() const {
    vector<Symbol> result;
    if (capacity() == 0)
      return result;
    result.reserve(capacity());
    if ((*this)[0]) {
      result.push_back(Symbol::eof());
    }
    for (std::size_t i = 1; i < capacity(); ++i) {
      if ((*this)[i]) {
        result.push_back(Terminal(i - 1));
      }
    }
    return result;
  }
  /**
  \brief Get the string representation of this set of symbols.

  \param[in] to_str The function for string representaton of symbols.
  */
  string to_string(symbol_string_fn to_str = ctf::to_string) const {
    auto terminals = symbols();
    if (terminals.empty()) {
      return "{}";
    }
    string result = "{ ";
    for (Symbol symbol : terminals) {
      result += to_str(symbol) + ", ";
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
  std::size_t operator()(const ctf::Symbol& s) const noexcept {
    // reinterpret as a std::size_t
    return std::hash<std::size_t>{}(reinterpret_cast<const std::size_t&>(s));
  }
};
}  // namespace std

#endif
/*** End of file ctf_base.hpp ***/
