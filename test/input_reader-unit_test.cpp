#include "../src/input_reader.hpp"
#include <catch.hpp>

#include <sstream>

using ctf::InputReader;
using ctf::Location;

TEST_CASE("Constructing InputReader", "[InputReader]") {
  std::stringstream s;
  InputReader r;
  InputReader r2{s, "Snoop Dogg"};

  REQUIRE(r.stream() == nullptr);
  REQUIRE(r2.stream() == &s);
  REQUIRE(r2.stream_name() == "Snoop Dogg");

  r.set_stream(s, "Dr. Dre");

  REQUIRE(r.stream() == &s);
  REQUIRE(r.stream_name() == "Dr. Dre");
}

TEST_CASE("Reading from InputReader", "[InputReader]") {
  std::stringstream s;
  InputReader r{s};
  Location l{};
  s << "ab\ndef\n\nx";

  REQUIRE(r.get() == 'a');
  REQUIRE(r.get() == 'b');
  REQUIRE(r.get() == '\n');
  REQUIRE(r.get(l) == 'd');

  REQUIRE(l == Location(2, 1));
}

TEST_CASE("Get line", "[InputReader]") {
  std::stringstream s;
  InputReader r{s};
  Location l{};
  std::string in = "ab\ndef\n\nx";
  s << in;
  // including EOF
  for (size_t i = 0; i < in.size(); ++i) {
    r.get();
  }
  REQUIRE(r.get() == std::char_traits<char>::eof());
  REQUIRE(r.get_line(2) == "def\n");
  REQUIRE(r.get_line(Location(3, 54)) == "\n");
  REQUIRE(r.get_line(0) == "");
  REQUIRE(r.get_line(55) == "");
}

TEST_CASE("Unget", "[InputReader]") {
  std::stringstream s;
  InputReader r{s};
  Location l{};
  std::string in = "ab\ndef\n\nx";
  s << in;

  for (size_t i = 0; i < 6; ++i) {
    r.get(l);
  }
  // Location: starting from {2,3}
  REQUIRE(char(r.unget(2)) == 'd');
  // {2,1}
  REQUIRE(char(r.unget(l, 1)) == '\n');
  // {1, 3}
  REQUIRE(l == Location(1, 3));
  REQUIRE(char(r.unget(50)) == 'a');
  // {1,1}
  REQUIRE(char(r.get()) == 'b');
  // {1, 2}
}