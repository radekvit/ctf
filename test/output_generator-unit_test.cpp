#include <catch.hpp>

#include <iostream>
#include <sstream>
#include "../src/ctf_output_generator.hpp"

using ctf::OutputGenerator;

using namespace ctf::literals;

TEST_CASE("OutputGenerator construction", "[OutputGenerator]") {
  OutputGenerator o1{};
  OutputGenerator o2{std::cout};

  REQUIRE(o1.has_stream() == false);
  REQUIRE(o2.has_stream() == true);

  o1.set_output_stream(std::cout);
  REQUIRE(o1.has_stream() == true);
}

TEST_CASE("OutputGenerator default output", "[OutputGenerator]") {
  std::stringstream s;
  OutputGenerator o{s};

  o.output({ctf::Token(0_t, ctf::Attribute('a')), 0_t, ctf::Symbol::eof()});

  REQUIRE(s.str() == "0_t.a\n0_t\nEOF\n");
}
