#include <catch.hpp>

#include "../src/ctf_base.hpp"

TEST_CASE("TranslationException") {
  REQUIRE_THROWS_AS(throw ctf::TranslationException("m"),
                    ctf::TranslationException &);
  REQUIRE_THROWS_WITH(throw ctf::TranslationException("m"), "m");
}
using namespace std::string_literals;

TEST_CASE("Symbol Construction", "[Symbol]") {
  using ctf::Symbol;
  Symbol s(Symbol::Type::UNKNOWN, "name", "a"s);
  REQUIRE(s.type() == Symbol::Type::UNKNOWN);
  REQUIRE(s.name() == "name");
  REQUIRE(s.attribute() == "a"s);

  REQUIRE_THROWS_AS(s = Symbol(""), std::invalid_argument &);

  using namespace ctf::literals;
  using ctf::Attribute;

  s = "ter"_t;
  REQUIRE(s.type() == Symbol::Type::TERMINAL);
  REQUIRE(s.name() == "ter");
  REQUIRE(s.attribute().empty());

  s = "nter"_nt;
  REQUIRE(s.type() == Symbol::Type::NONTERMINAL);
  REQUIRE(s.name() == "nter");
  REQUIRE(s.attribute().empty());

  s = "spec"_s;
  REQUIRE(s.type() == Symbol::Type::SPECIAL);
  REQUIRE(s.name() == "spec");
  REQUIRE(s.attribute().empty());

  s = Symbol::eof();
  REQUIRE(s.type() == Symbol::Type::EOI);
  REQUIRE(s.name() == "");
}

TEST_CASE("operators", "[Symbol]") {
  using namespace ctf::literals;
  using ctf::Symbol;

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
  REQUIRE(s3 >= s2);
  REQUIRE_FALSE(s2 >= s4);
  REQUIRE(s2 <= s4);
}