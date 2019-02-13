#include <catch.hpp>

#include "../src/ctf_base.hpp"

#include <type_traits>

TEST_CASE("TranslationException") {
  REQUIRE_THROWS_AS(throw ctf::TranslationException("m"), ctf::TranslationException);
  REQUIRE_THROWS_WITH(throw ctf::TranslationException("m"), "m");
}
using namespace std::string_literals;

using ctf::Symbol;
using ctf::Token;
using ctf::Attribute;

TEST_CASE("Symbol construction and assignment", "[Symbol]") {
  // constructor traits
  REQUIRE(std::is_trivially_constructible<Symbol, const Symbol&>::value);
  REQUIRE(std::is_trivially_constructible<Symbol, Symbol&&>::value);
  REQUIRE(std::is_nothrow_constructible<Symbol, Symbol::Type, size_t>::value);
  // assignment traits
  REQUIRE(std::is_trivially_assignable<Symbol, const Symbol&>::value);
  REQUIRE(std::is_trivially_assignable<Symbol, Symbol&&>::value);
}

TEST_CASE("Symbol operators", "[Symbol]") {
  using namespace ctf::literals;
  using ctf::Token;

  Symbol s1 = "0"_t;
  Symbol s2 = "5"_t;
  Symbol s3 = "5"_nt;
  Symbol s4 = "7"_t;

  REQUIRE(s1 < s2);
  REQUIRE_FALSE(s2 < s1);
  REQUIRE_FALSE(s3 == s2);
  REQUIRE_FALSE(s1 == s4);
  REQUIRE(s1 != s2);
  REQUIRE_FALSE(s2 != s2);
  REQUIRE(s4 > s2);
  REQUIRE(s4 >= s2);
  REQUIRE(s3 <= s2);
  REQUIRE_FALSE(s2 >= s4);
  REQUIRE(s2 <= s4);
}

TEST_CASE("Token Construction", "[Token]") {
  using namespace ctf::literals;
  Token s("name"_t, Attribute("a"s));
  REQUIRE(s.type() == Symbol::Type::TERMINAL);
  REQUIRE(s.terminal());
  REQUIRE(s.name() == "name");
  REQUIRE(s.attribute() == "a"s);

  using namespace ctf::literals;
  using ctf::Attribute;

  s = "ter"_t;
  REQUIRE(s.type() == Symbol::Type::TERMINAL);
  REQUIRE(s.terminal());
  REQUIRE(s.name() == "ter");
  REQUIRE(s.attribute().empty());

  s = "nter"_nt;
  REQUIRE(s.type() == Symbol::Type::NONTERMINAL);
  REQUIRE(s.nonterminal());
  REQUIRE(s.name() == "nter");
  REQUIRE(s.attribute().empty());

  s = Symbol::eof();
  REQUIRE(s.type() == Symbol::Type::EOI);
  REQUIRE(s.name() == "EOF");
}

TEST_CASE("Token operators", "[Token]") {
  using namespace ctf::literals;
  using ctf::Token;

  Token s1 = "0"_t;
  Token s2 = "5"_t;
  Token s3 = "5"_nt;
  Token s4 = "7"_t;

  REQUIRE(s1 < s2);
  REQUIRE_FALSE(s2 < s1);
  REQUIRE_FALSE(s3 == s2);
  REQUIRE_FALSE(s1 == s4);
  REQUIRE(s1 != s2);
  REQUIRE_FALSE(s2 != s2);
  REQUIRE(s4 > s2);
  REQUIRE(s4 >= s2);
  REQUIRE(s3 <= s2);
  REQUIRE_FALSE(s2 >= s4);
  REQUIRE(s2 <= s4);
}