#include <catch.hpp>

#include "../src/ctf_lr_lr0.hpp"

using ctf::TranslationGrammar;
using Rule = TranslationGrammar::Rule;
using ctf::lr0::Item;
using ctf::lr0::State;
using ctf::LR0StateMachine;
using ctf::vector_set;
using ctf::vector;

using namespace ctf::literals;

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

TEST_CASE("LR0StateMachine correctness", "[LR0StateMachine]") {
  LR0StateMachine sm{grammar};
  auto& rules = grammar.rules();

  size_t activeState = 0;
  size_t state = 0;

  vector<State> expected{
      {Item(rules[2], 1)},
      {Item(rules[1], 1)},
      {Item(rules[4], 1), Item(rules[0], 1)},
      {Item(rules[3], 1),
       Item(rules[0], 0),
       Item(rules[1], 0),
       Item(rules[2], 0),
       Item(rules[3], 0)},
      {Item(rules[3], 2), Item(rules[0], 1)},
      {Item(rules[0], 2), Item(rules[2], 0), Item(rules[3], 0)},
  };
  REQUIRE(sm.states()[0] == Item(grammar.starting_rule(), 0).closure(grammar));
  // check correctness of at least a part of the state machine
  activeState = 0;
  // i transition from 0
  REQUIRE_NOTHROW(state = sm.transitions()[0].at("i"_t));
  REQUIRE(sm.states()[state] == expected[0]);
  // A transition from 0
  REQUIRE_NOTHROW(state = sm.transitions()[0].at("A"_nt));
  REQUIRE(sm.states()[state] == expected[1]);
  // S transition from 0
  REQUIRE_NOTHROW(state = sm.transitions()[0].at("S"_nt));
  REQUIRE(sm.states()[state] == expected[2]);
  // ( transition from 0
  REQUIRE_NOTHROW(state = sm.transitions()[0].at("("_t));
  REQUIRE(sm.states()[state] == expected[3]);
  // no o transition from 0
  REQUIRE_THROWS(sm.transitions()[0].at("o"_t));

  activeState = state;
  REQUIRE_NOTHROW(state = sm.transitions()[activeState].at("("_t));
  REQUIRE(activeState == state);
  REQUIRE_NOTHROW(state = sm.transitions()[activeState].at("i"_t));
  REQUIRE(sm.states()[state] == expected[0]);
  REQUIRE_NOTHROW(state = sm.transitions()[activeState].at("A"_nt));
  REQUIRE(sm.states()[state] == expected[1]);
  REQUIRE_NOTHROW(state = sm.transitions()[activeState].at("S"_nt));
  REQUIRE(sm.states()[state] == expected[4]);

  activeState = state;
  REQUIRE_NOTHROW(state = sm.transitions()[activeState].at("o"_t));
  REQUIRE(sm.states()[state] == expected[5]);
  activeState = state;
  REQUIRE_NOTHROW(state = sm.transitions()[0].at("("_t));
  REQUIRE(sm.states()[state] == expected[3]);
}