#include <catch.hpp>

#include "../src/ctf_lr_table.hpp"
#include "test_utils.h"

using ctf::Symbol;
using ctf::TranslationGrammar;
using ctf::SLRTable;
using ctf::LRAction;

static constexpr ctf::Symbol operator""_nt(const char* s, size_t) {
  using namespace ctf::literals;
  if (c_streq(s, "S"))
    return 0_nt;
  if (c_streq(s, "A"))
    return 1_nt;

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

static TranslationGrammar grammar{{
                                      {"S"_nt, {"S"_nt, "o"_t, "A"_nt}},
                                      {"S"_nt, {"A"_nt}},
                                      {"A"_nt, {"i"_t}},
                                      {"A"_nt, {"("_t, "S"_nt, ")"_t}},
                                  },
                                  "S"_nt};

TEST_CASE("SLRTable construction and deletion", "[SLRTable]") {
  REQUIRE_NOTHROW(SLRTable(grammar));
}

TEST_CASE("SLRTable base", "[SLRTable]") {
  SLRTable table{grammar};
  size_t state = 0;

  // check some of the table contents
  REQUIRE(table.lr_action(state, "i"_t).action == LRAction::SHIFT);
  REQUIRE(table.lr_action(state, "("_t).action == LRAction::SHIFT);
  REQUIRE(table.lr_action(state, ")"_t).action == LRAction::ERROR);
  state = table.lr_action(0, "i"_t).argument;
  REQUIRE(table.lr_action(state, "o"_t).action == LRAction::REDUCE);
  REQUIRE(table.lr_action(state, ")"_t).action == LRAction::REDUCE);
  REQUIRE(table.lr_action(state, Symbol::eof()).action == LRAction::REDUCE);

  state = table.lr_goto(0, "S"_nt);
  REQUIRE(table.lr_action(state, Symbol::eof()).action == LRAction::SUCCESS);
}
