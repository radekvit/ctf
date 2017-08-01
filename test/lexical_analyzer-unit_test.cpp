#include <catch.hpp>

#include "../src/lexical_analyzer.hpp"

#include <sstream>

using ctf::LexicalAnalyzer;
using ctf::InputReader;

using std::stringstream;

TEST_CASE("LexicalAnalyzer construction and setup", "[LexicalAnalyzer]") {
  InputReader r;
  std::stringstream s;

  LexicalAnalyzer l1{};
  LexicalAnalyzer l2{r};

  REQUIRE(l1.has_input() == false);
  REQUIRE(l2.has_input() == false);
  r.set_stream(s);
  REQUIRE(l2.has_input() == true);
}

TEST_CASE("LexicalAnalyzer default input", "[LexicalAnalyzer]") {
  using namespace ctf::literals;
  stringstream s;
  InputReader r{s};
  s << "a\nb\n";
  LexicalAnalyzer l{r};

  REQUIRE(l.get_token() == "a"_t);
  REQUIRE(l.get_token() == "b"_t);
  REQUIRE(l.get_token() == ctf::Symbol::eof());

  s << "";

  REQUIRE(l.get_token() == ctf::Symbol::eof());
}