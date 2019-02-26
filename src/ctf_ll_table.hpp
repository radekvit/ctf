/**
\file ll_table.hpp
\brief Defines class LLTable and its methods.
\author Radek Vít
*/
#ifndef CTF_LL_TABLE_H
#define CTF_LL_TABLE_H

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "ctf_table_sets.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf {

template <typename T>
class LLGenericTable {
 public:
  /**
  \brief Type of cells.
  */
  using cell = T;
  /**
  \brief Type of rows.
  */
  using row = vector<cell>;

  /**
  \brief Constructs an empty LL table.
  */
  LLGenericTable() = default;
  /**
  \brief Constructs a decision table from a translation grammar and a predict
  set.

  \param[in] tg TranslationGrammar. Terminals and Nonterminals are mapped to
  their respective indices. Rule indices are stored in the decision table.
  \param[in] predict Predict set for table construction.
  */
  LLGenericTable(const TranslationGrammar& tg, const predict_t& predict) {
    initialize(tg, predict);
  }
  /**
  \brief Returns an index of the rule to be used when t is the current token
  and nt is at the top of input stack.

  \param[in] nt Nonterminal on the top of the stack.
  \param[in] t Last read terminal.

  \returns Index of the applicable rule or invalid rule index.

  If no rule is applicable, returns the index beyond the last rule.
  */
  const T& rule_index(const Symbol& nt, const Symbol& t) noexcept {
    // iterator to nonterminal index
    auto ntid = nt.id();
    // iterator to terminal index;
    auto tid = t.id();
    // either of the arguments not found
    if (ntid >= _nonterminals || tid >= _terminals)
      return _invalid;
    // returning from LL table
    return _table[index(ntid, tid)];
  }

 protected:
  /**
  \brief Table storing rule indices. 2D array mapped to 1D array.
  */
  row _table;
  /**
  \brief Mapping nonterminals to _table indices.
  */
  size_t _nonterminals;
  /**
  \brief Mapping terminals to _table row indices.
  */
  size_t _terminals;
  /**
  \brief Stores invalid value.
  */
  cell _invalid;
  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t index(size_t y, size_t x) const { return _terminals * y + x; }

  virtual void initialize_invalid(const TranslationGrammar&) { return; }

  virtual void insert_rule(const size_t insertedRule, const size_t i) {
    _table[i] = {insertedRule};
  }

  void initialize() {}

  void initialize(const TranslationGrammar& tg, const predict_t& predict) {
    initialize_invalid(tg);
    _table = row(tg.nonterminals() * tg.terminals(), _invalid);
    assert(predict.size() == tg.rules().size());

    _nonterminals = tg.nonterminals();
    _terminals = tg.terminals();

    /* fill table */
    for (size_t i = 0; i < tg.rules().size(); ++i) {
      // TranslationGrammar requires this to be always found
      size_t ni = tg.rules()[i].nonterminal().id();

      for (auto& t : predict[i].symbols()) {
        auto ti = t.id();
        insert_rule(i, index(ni, ti));
      }  // for all terminals
    }    // for all i
  }
};

/**
\brief Class containing rule indices to be used in a LL controlled translation.
*/
class LLTable : public LLGenericTable<size_t> {
  void initialize_invalid(const TranslationGrammar& tg) override { _invalid = tg.rules().size(); }

  void insert_rule(const size_t insertedRule, const size_t i) override {
    // a rule is already present
    if (_table[i] != _invalid) {
      throw std::invalid_argument("Constructing LLTable from a non-LL TranslationGrammar.");
    }
    _table[i] = insertedRule;
  }

 public:
  LLTable(const TranslationGrammar& tg, const predict_t& predict) { initialize(tg, predict); }

  LLTable() { _invalid = 0; }
};

class PriorityLLTable : public LLTable {
 public:
  PriorityLLTable(const TranslationGrammar& tg, const predict_t& predict) {
    initialize(tg, predict);
  }

  PriorityLLTable() { _invalid = 0; }

 private:
  void insert_rule(const size_t insertedRule, const size_t i) override {
    // insert high priority rule
    if (_table[i] == _invalid || _table[i] > insertedRule) {
      _table[i] = insertedRule;
    }
  }
};

class GeneralLLTable : public LLGenericTable<vector_set<size_t>> {
 public:
  GeneralLLTable(const TranslationGrammar& tg, const predict_t& predict) {
    initialize(tg, predict);
  }

  GeneralLLTable() { initialize(); };

 private:
  void insert_rule(const size_t insertedRule, const size_t i) override {
    _table[i].insert(insertedRule);
  }

  void initialize_invalid(const TranslationGrammar&) override { _invalid = {}; }
};
}  // namespace ctf

#endif
/*** End of file ll_table.hpp ***/