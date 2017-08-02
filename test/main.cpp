#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "../include/ctf.hpp"

std::ostream &operator<<(std::ostream &os, const Symbol &s) {
  switch (s.type()) {
    case Symbol::Type::NONTERMINAL:
      os << "NONTERMINAL: ";
      break;
    case Symbol::Type::TERMINAL:
      os << "TERMINAL: ";
      break;
    case Symbol::Type::EOI:
      os << "EOF";
      break;
    case Symbol::Type::SPECIAL:
      os << "SPECIAL: ";
      break;
    default:
      os << "SYMBOL: ";
      break;
  }
  os << s.name();

  return os;
}

std::ostream &operator<<(std::ostream &os, const Location &l) {
  return os << l.to_string();
}
