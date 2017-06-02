#include <catch.hpp>

#include "../src/lexical_analyzer.hpp"

#include <sstream>

using ctf::LexicalAnalyzer;

using std::stringstream;

TEST_CASE("LexicalAnalyzer construction and setup", "[LexicalAnalyzer]") {
  stringstream s;
  LexicalAnalyzer l1{};
  LexicalAnalyzer l2{s};

  REQUIRE(l1.stream_set() == false);
  REQUIRE(l2.stream_set() == true);
  l1.set_stream(s);
  REQUIRE(l1.stream_set() == true);
}

TEST_CASE("LexicalAnalyzer default input", "[LexicalAnalyzer]") {
  using namespace ctf::literals;
  stringstream s;
  s << "a\nb\n";
  LexicalAnalyzer l{s};

  REQUIRE(l.get_token() == "a"_t);
  REQUIRE(l.get_token() == "b"_t);
  REQUIRE(l.get_token() == ctf::Symbol::eof());

  s << "";

  REQUIRE(l.get_token() == ctf::Symbol::eof());
}