#include <catch.hpp>

#include <fstream>
#include <sstream>
#include "../src/ctf_translation.hpp"
#include "test_utils.h"

using ctf::string;
using ctf::Symbol;
using ctf::Token;
using ctf::Attribute;
using ctf::Translation;
using ctf::TranslationResult;
using ctf::LexicalAnalyzer;
using ctf::TranslationGrammar;
using ctf::OutputGenerator;

using namespace ctf::literals;

class TITOG : public OutputGenerator {
 public:
  using OutputGenerator::OutputGenerator;

  void output(const ctf::tstack<Token>& tokens) override {
    auto& os = this->os();
    for (auto& t : tokens) {
      if (t == Symbol::eof())
        return;
      if (t.symbol() == 0_t) {
        os << "i";
      } else if (t.symbol() == 1_t) {
        os << "+";
      } else if (t.symbol() == 2_t) {
        os << "*";
      }
      if (!t.attribute().empty()) {
        os << ".";
        auto& type = t.attribute().type();
        if (type == typeid(string))
          os << t.attribute().get<string>();
        else if (type == typeid(char))
          os << t.attribute().get<char>();
        else if (type == typeid(double))
          os << t.attribute().get<double>();
        else if (type == typeid(size_t))
          os << t.attribute().get<size_t>();
      }
      os << "\n";
    }
  }
};

static constexpr ctf::Symbol operator""_nt(const char* s, size_t) {
  if (c_streq(s, "E"))
    return 0_nt;
  if (c_streq(s, "E'"))
    return 1_nt;
  if (c_streq(s, "T"))
    return 2_nt;
  if (c_streq(s, "T'"))
    return 3_nt;
  if (c_streq(s, "F"))
    return 4_nt;

  return 100_nt;
}
static constexpr ctf::Symbol operator""_t(const char* s, size_t) {
  if (c_streq(s, "i"))
    return 0_t;
  if (c_streq(s, "+"))
    return 1_t;
  if (c_streq(s, "*"))
    return 2_t;
  if (c_streq(s, "("))
    return 3_t;
  if (c_streq(s, ")"))
    return 4_t;

  return 100_t;
}

class TestLexicalAnalyzer : public LexicalAnalyzer {
 public:
  using LexicalAnalyzer::LexicalAnalyzer;

  Symbol name_to_symbol(const string& s) {
    if (s == "i") {
      return 0_t;
    }
    if (s == "+") {
      return 1_t;
    }
    if (s == "*") {
      return 2_t;
    }
    if (s == "(") {
      return 3_t;
    }
    if (s == ")") {
      return 4_t;
    }

    throw std::invalid_argument(s + ": unknown symbol name");
  }

  Token read_token() override {
    string name;
    // first character
    int c = get();
    while (std::isspace(c)) {
      reset_location();
      c = get();
    }

    if (c == std::char_traits<char>::eof()) {
      return token_eof();
    }

    do {
      name += c;
      c = get();
    } while (!isspace(c) && c != std::char_traits<char>::eof());
    if (c != std::char_traits<char>::eof())
      unget();

    return token(name_to_symbol(name), Attribute{attr++});
  }

  void reset_private() override { attr = 0; }

  size_t attr = 0;
};

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
  REQUIRE_NOTHROW(
      Translation(std::make_unique<LexicalAnalyzer>(), "ll", tg, std::make_unique<TITOG>()));

  auto tcp = Translation::control("ll");
  REQUIRE_NOTHROW(
      Translation(std::make_unique<LexicalAnalyzer>(), *tcp, tg, std::make_unique<TITOG>()));

  REQUIRE_THROWS_AS(
      Translation(
          std::make_unique<LexicalAnalyzer>(), "fail, please", tg, std::make_unique<TITOG>()),
      std::invalid_argument);
}

TEST_CASE("Running LR translation", "[Translation]") {
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
        std::make_unique<TestLexicalAnalyzer>(), "canonical lr", tg, std::make_unique<TITOG>());
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
        std::make_unique<TestLexicalAnalyzer>(), "canonical lr", tg, std::make_unique<TITOG>());
    std::stringstream out;
    std::stringstream error;
    std::ifstream in("media/in");
    if (in.fail())
      throw std::runtime_error("Files not present");
    REQUIRE(tr.run(in, out, error) == TranslationResult::SUCCESS);
    REQUIRE(out.str() == "");
  }
}
// TODO fix LL predict
#if 0
TEST_CASE("Running LL translation", "[Translation]") {
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
    Translation tr(std::make_unique<TestLexicalAnalyzer>(), "ll", tg, std::make_unique<TITOG>());
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
    Translation tr(std::make_unique<TestLexicalAnalyzer>(), "ll", tg, std::make_unique<TITOG>());
    std::stringstream out;
    std::stringstream error;
    std::ifstream in("media/in");
    if (in.fail())
      throw std::runtime_error("Files not present");
    REQUIRE(tr.run(in, out, error) == TranslationResult::SUCCESS);
    REQUIRE(out.str() == "");
  }
}
#endif