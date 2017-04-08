#include "../src/translation.hpp"
#include <catch.hpp>

#include <fstream>
#include <sstream>

using ctf::Translation;
using ctf::LexicalAnalyzer;
using ctf::TranslationGrammar;
using ctf::OutputGenerator;

using namespace ctf::literals;

TEST_CASE("Constructing translation", "[Translation]") {
  TranslationGrammar tg{
      {
          {"E"_nt, {"T"_nt, "E'"_nt}},
          {"E'"_nt, {}},
          {"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}, {"T"_nt, "+"_t, "E'"_nt}},
          {"F"_nt, {"("_t, "E"_nt, ")"_t}, {"E"_nt}},
          {"F"_nt, {"i"_t}},
          {"T"_nt, {"F"_nt, "T'"_nt}},
          {"T'"_nt, {}},
          {"T'"_nt, {"*"_t, "F"_nt, "T'"_nt}, {"F"_nt, "*"_t, "T'"_nt}},
      },
      "E"_nt};
  REQUIRE_NOTHROW(Translation(LexicalAnalyzer::default_input, "ll", tg,
                              OutputGenerator::default_output));

  auto tcp = Translation::control("ll");
  REQUIRE_NOTHROW(Translation(LexicalAnalyzer::default_input, *tcp, tg,
                              OutputGenerator::default_output));

  REQUIRE_THROWS_AS(Translation(LexicalAnalyzer::default_input, "fail, please",
                                tg, OutputGenerator::default_output),
                    std::invalid_argument &);
}

TEST_CASE("Running translation", "[Translation]") {
  SECTION("full translation") {
    TranslationGrammar tg{
        {
            {"E"_nt, {"T"_nt, "E'"_nt}},
            {"E'"_nt, {}},
            {"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}, {"T"_nt, "+"_t, "E'"_nt}},
            {"F"_nt, {"("_t, "E"_nt, ")"_t}, {"E"_nt}},
            {"F"_nt, {"i"_t}},
            {"T"_nt, {"F"_nt, "T'"_nt}},
            {"T'"_nt, {}},
            {"T'"_nt, {"*"_t, "F"_nt, "T'"_nt}, {"F"_nt, "*"_t, "T'"_nt}},
        },
        "E"_nt};
    Translation tr(LexicalAnalyzer::default_input, "ll", tg,
                   OutputGenerator::default_output);
    std::stringstream expected;
    std::stringstream out;
    std::ifstream in("media/in");
    std::ifstream ex("media/expected");
    if (in.fail() || ex.fail())
      throw std::runtime_error("Files not present");
    expected << ex.rdbuf();
    tr.run(in, out);
    REQUIRE(out.str() == expected.str());
  }
  SECTION("translation with empty output") {
    TranslationGrammar tg{
        {
            {"E"_nt, {"T"_nt, "E'"_nt}},
            {"E'"_nt, {}},
            {"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}, {"T"_nt, "E'"_nt}},
            {"F"_nt, {"("_t, "E"_nt, ")"_t}, {"E"_nt}},
            {"F"_nt, {"i"_t}, {}},
            {"T"_nt, {"F"_nt, "T'"_nt}},
            {"T'"_nt, {}},
            {"T'"_nt, {"*"_t, "F"_nt, "T'"_nt}, {"F"_nt, "T'"_nt}},
        },
        "E"_nt};
    Translation tr(LexicalAnalyzer::default_input, "ll", tg,
                   OutputGenerator::default_output);
    std::stringstream out;
    std::ifstream in("media/in");
    if (in.fail())
      throw std::runtime_error("Files not present");
    tr.run(in, out);
    REQUIRE(out.str() == "");
  }
}