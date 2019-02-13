#include <catch.hpp>

#include <fstream>
#include <sstream>

#include "../src/ctf_translation.hpp"

using ctf::Symbol;
using ctf::Translation;
using ctf::TranslationResult;
using ctf::LexicalAnalyzer;
using ctf::TranslationGrammar;
using ctf::OutputGenerator;

using namespace ctf::literals;

TEST_CASE("Constructing translation", "[Translation]") {
  TranslationGrammar tg{{
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
  REQUIRE_NOTHROW(Translation(
      std::make_unique<LexicalAnalyzer>(), "ll", tg, std::make_unique<OutputGenerator>()));

  auto tcp = Translation::control("ll");
  REQUIRE_NOTHROW(Translation(
      std::make_unique<LexicalAnalyzer>(), *tcp, tg, std::make_unique<OutputGenerator>()));

  REQUIRE_THROWS_AS(Translation(std::make_unique<LexicalAnalyzer>(),
                                "fail, please",
                                tg,
                                std::make_unique<OutputGenerator>()),
                    std::invalid_argument);
}

TEST_CASE("Running translation", "[Translation]") {
  SECTION("full translation") {
    TranslationGrammar tg{{
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
    Translation tr(
        std::make_unique<LexicalAnalyzer>(), "ll", tg, std::make_unique<OutputGenerator>());
    std::stringstream expected;
    std::stringstream out;
    std::stringstream error;
    std::ifstream in("media/in");
    std::ifstream ex("media/expected");
    if (in.fail() || ex.fail())
      throw std::runtime_error("Files not present");
    expected << ex.rdbuf();
    REQUIRE(tr.run(in, out, error) == TranslationResult::SUCCESS);
    REQUIRE(out.str() == expected.str());
  }
  SECTION("translation with empty output") {
    TranslationGrammar tg{{
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
    Translation tr(
        std::make_unique<LexicalAnalyzer>(), "ll", tg, std::make_unique<OutputGenerator>());
    std::stringstream out;
    std::stringstream error;
    std::ifstream in("media/in");
    if (in.fail())
      throw std::runtime_error("Files not present");
    REQUIRE(tr.run(in, out, error) == TranslationResult::SUCCESS);
    REQUIRE(out.str() == "");
  }
}