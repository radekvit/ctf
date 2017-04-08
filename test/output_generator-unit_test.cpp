#include <catch.hpp>

#include "../src/output_generator.hpp"

#include <iostream>
#include <sstream>

using ctf::OutputGenerator;

TEST_CASE("OutputGenerator construction", "[OutputGenerator]") {
  OutputGenerator o1([](std::ostream &, const ctf::Symbol &) { return; });
  OutputGenerator o2{std::cout};

  REQUIRE(o1.stream_set() == false);
  REQUIRE(o2.stream_set() == true);

  o1.set_output(std::cout);
  REQUIRE(o1.stream_set() == true);
}

TEST_CASE("OutputGenerator default output", "[OutputGenerator]") {
  std::stringstream s;
  OutputGenerator o{s};

  o.get_token(ctf::Terminal("n", "a"));
  o.get_token(ctf::Terminal("n"));
  o.get_token(ctf::Symbol::eof());

  REQUIRE(s.str() == "n.a\nn\n");
}
