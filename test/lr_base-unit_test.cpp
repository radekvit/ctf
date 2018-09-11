#include <catch.hpp>

#include "../src/ctf_lr_base.hpp"

using ctf::TranslationGrammar;
using Rule = TranslationGrammar::Rule;
using ctf::LR0Item;
using ctf::set;

using namespace ctf::literals;

static TranslationGrammar grammar{{
                                      {"S'"_nt, {"S"_nt}},
                                      {"S"_nt, {"S"_nt, "o"_t, "A"_nt}},
                                      {"S"_nt, {"A"_nt}},
                                      {"A"_nt, {"i"_t}},
                                      {"A"_nt, {"("_t, "S"_nt, ")"_t}},
                                  },
                                  "S'"_nt};

TEST_CASE("LR0Item construction", "[LR0Item]") {
  REQUIRE_NOTHROW(LR0Item(grammar.rules()[0], 0));
}

TEST_CASE("LR0Item operations", "[LR0Item]") {
  LR0Item i{grammar.rules()[0], 0};

  set<LR0Item> requiredClosure{
      {grammar.rules()[0], 0},
      {grammar.rules()[1], 0},
      {grammar.rules()[2], 0},
      {grammar.rules()[3], 0},
      {grammar.rules()[4], 0},
  };

  REQUIRE(i.closure(grammar) == requiredClosure);
}