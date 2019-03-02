#include <catch.hpp>

#include "../src/ctf_lr_lr0.hpp"
#include "test_utils.h"

using ctf::TranslationGrammar;
using Rule = TranslationGrammar::Rule;
using ctf::lr0::Item;
using ctf::vector_set;
using ctf::vector;

static constexpr ctf::Symbol operator""_nt(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "S"))
    return 0_nt;
  if (c_streq(s, "S'"))
    return 1_nt;
  if (c_streq(s, "A"))
    return 2_nt;

  return 100_nt;
}
static constexpr ctf::Symbol operator""_t(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "i"))
    return 0_t;
  if (c_streq(s, "o"))
    return 1_t;
  if (c_streq(s, "("))
    return 2_t;
  if (c_streq(s, ")"))
    return 3_t;

  return 100_t;
}

// already augmented
static TranslationGrammar grammar{{
                                    {"S"_nt, {"S"_nt, "o"_t, "A"_nt}},
                                    {"S"_nt, {"A"_nt}},
                                    {"A"_nt, {"i"_t}},
                                    {"A"_nt, {"("_t, "S"_nt, ")"_t}},
                                    {"S'"_nt, {"S"_nt}},
                                  },
                                  "S'"_nt};

TEST_CASE("lr0::Item construction", "[lr0::Item]") { REQUIRE_NOTHROW(Item(grammar.rules()[0], 0)); }

TEST_CASE("lr0::Item operations", "[lr0::Item]") {
  Item i{grammar.rules()[4], 0};

  vector_set<Item> requiredClosure{
    {grammar.rules()[0], 0},
    {grammar.rules()[1], 0},
    {grammar.rules()[2], 0},
    {grammar.rules()[3], 0},
    {grammar.rules()[4], 0},
  };

  REQUIRE(i.closure(grammar) == requiredClosure);
}

TEST_CASE("lr0::Item comparison operators", "[lr0::Item]") {
  Item item1(grammar.rules()[0], 1);
  Item item2(grammar.rules()[0], 1);
  Item item3(grammar.rules()[0], 0);

  REQUIRE(item1 == item2);
  REQUIRE(item1 < item3);
  REQUIRE(!(item3 < item1));
}
