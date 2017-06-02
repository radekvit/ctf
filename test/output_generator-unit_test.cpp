#include <catch.hpp>

#include "../src/output_generator.hpp"

#include <iostream>
#include <sstream>

using ctf::OutputGenerator;

TEST_CASE("OutputGenerator construction", "[OutputGenerator]") {
  OutputGenerator o1([](std::ostream &, const ctf::tstack<ctf::Symbol> &) { return; });
  OutputGenerator o2{std::cout};

  REQUIRE(o1.stream_set() == false);
  REQUIRE(o2.stream_set() == true);

  o1.set_stream(std::cout);
  REQUIRE(o1.stream_set() == true);
}

TEST_CASE("OutputGenerator default output", "[OutputGenerator]") {
  std::stringstream s;
  OutputGenerator o{s};

  o.output({ctf::Terminal("n", "a"), ctf::Terminal("n"), ctf::Symbol::eof()});

  REQUIRE(s.str() == "n.a\nn\n");
}
