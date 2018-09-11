#include <catch.hpp>

#include "../src/ctf_lr_table.hpp"

using ctf::Symbol;
using ctf::TranslationGrammar;
using ctf::SLRTable;
using ctf::LRActionType;

using namespace ctf::literals;

static TranslationGrammar grammar{{
                                      {"S"_nt, {"S"_nt, "o"_t, "A"_nt}},
                                      {"S"_nt, {"A"_nt}},
                                      {"A"_nt, {"i"_t}},
                                      {"A"_nt, {"("_t, "S"_nt, ")"_t}},
                                  },
                                  "S"_nt};

TEST_CASE("SLRTable construction and deletion", "[SLRTable]") {
  auto empty = ctf::create_empty(grammar);
  auto first = ctf::create_first(grammar, empty);
  auto follow = ctf::create_follow(grammar, empty, first);

  REQUIRE_NOTHROW(SLRTable(grammar, follow));
}

TEST_CASE("SLRTable base", "[SLRTable]") {
  auto empty = ctf::create_empty(grammar);
  auto first = ctf::create_first(grammar, empty);
  auto follow = ctf::create_follow(grammar, empty, first);

  SLRTable table{grammar, follow};
  size_t state = 0;

  // check some of the table contents
  REQUIRE(table.lr_action(state, "i"_t).type == LRActionType::SHIFT);
  REQUIRE(table.lr_action(state, "("_t).type == LRActionType::SHIFT);
  REQUIRE(table.lr_action(state, ")"_t).type == LRActionType::ERROR);
  state = table.lr_action(0, "i"_t).argument;
  REQUIRE(table.lr_action(state, "o"_t).type == LRActionType::REDUCE);
  REQUIRE(table.lr_action(state, ")"_t).type == LRActionType::REDUCE);
  REQUIRE(table.lr_action(state, Symbol::eof()).type == LRActionType::REDUCE);

  state = table.lr_goto(0, "S"_nt);
  REQUIRE(table.lr_action(state, Symbol::eof()).type == LRActionType::SUCCESS);
}
