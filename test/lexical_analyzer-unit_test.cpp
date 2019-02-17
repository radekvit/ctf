#include <catch.hpp>
#include <sstream>
#include "test_utils.h"

#include "../src/ctf_lexical_analyzer.hpp"

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
  s << "0\n1\n165\n";
  LexicalAnalyzer l{r};

  REQUIRE(l.get_token() == 0_t);
  REQUIRE(l.get_token() == 1_t);
  REQUIRE(l.get_token() == 165_t);
  REQUIRE(l.get_token() == ctf::Symbol::eof());

  s << "";

  REQUIRE(l.get_token() == ctf::Symbol::eof());
}