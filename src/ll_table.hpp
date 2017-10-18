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

#include "translation_grammar.hpp"

namespace ctf {

template <typename T>
class DecisionTable {
 public:
  /**
  \brief Type of cells.
  */
  using cell = T;
  /**
  \brief Type of rows.
  */
  using row = vector<cell>;

 protected:
  /**
  \brief Table storing rule indices. 2D array mapped to 1D array.
  */
  row table_;
  /**
  \brief Mapping nonterminals to table_ indices.
  */
  map<Symbol, size_t> nonterminalMap_;
  /**
  \brief Mapping terminals to table_ row indices.
  */
  map<Symbol, size_t> terminalMap_;
  /**
  \brief Stores invalid value.
  */
  cell invalid_;
  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t index(size_t y, size_t x) const { return terminalMap_.size() * y + x; }

  virtual void initialize_invalid(const TranslationGrammar&) { return; }

  virtual void insert_rule(const size_t insertedRule, const size_t i) {
    table_[i] = {insertedRule};
  }

  void initialize() {}

  void initialize(const TranslationGrammar& tg,
                  const vector<vector<Symbol>>& predict) {
    initialize_invalid(tg);
    table_ =
        row(tg.nonterminals().size() * (tg.terminals().size() + 1), invalid_);
    if (predict.size() != tg.rules().size())
      throw std::invalid_argument(
          "Mismatched predict and TranslationGrammar.rules "
          "sizes when constructing a decision table.");

    // create index maps for terminals and nonterminals
    for (size_t i = 0; i < tg.nonterminals().size(); ++i) {
      nonterminalMap_.insert(std::make_pair(tg.nonterminals()[i], i));
    }
    for (size_t i = 0; i < tg.terminals().size(); ++i) {
      terminalMap_.insert(std::make_pair(tg.terminals()[i], i));
    }
    terminalMap_.insert(std::make_pair(Symbol::eof(), tg.terminals().size()));

    /* fill table */
    for (size_t i = 0; i < tg.rules().size(); ++i) {
      auto& terminals = predict[i];
      // TranslationGrammar requires this to be always found
      size_t ni = nonterminalMap_.at(tg.rules()[i].nonterminal());

      for (auto& t : terminals) {
        auto tit = terminalMap_.find(t);
        if (tit == terminalMap_.end())
          throw std::invalid_argument(
              "Terminal in predict not a terminal in translation grammar when "
              "constructing a decision table.");

        size_t ti = tit->second;
        insert_rule(i, index(ni, ti));
      }  // for all terminals
    }    // for all i
  }

 public:
  /**
  \brief Constructs an empty LL table.
  */
  DecisionTable() = default;
  /**
  \brief Constructs a decision table from a translation grammar and a predict
  set.

  \param[in] tg TranslationGrammar. Terminals and Nonterminals are mapped to
  their respective indices. Rule indices are stored in the decision table.
  \param[in] predict Predict set for table construction.
  */
  DecisionTable(const TranslationGrammar& tg,
                const vector<vector<Symbol>>& predict) {
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
  size_t rule_index(const Symbol& nt, const Symbol& t) noexcept {
    // iterator to nonterminal index
    auto ntit = nonterminalMap_.find(nt);
    // iterator to terminal index;
    auto tit = terminalMap_.find(t);
    // either of the arguments not found
    if (ntit == nonterminalMap_.end() || tit == terminalMap_.end())
      return invalid_;
    // returning from LL table
    return table_[index(ntit->second, tit->second)];
  }
};

/**
\brief Class containing rule indices to be used in a LL controlled translation.
*/
class LLTable : public DecisionTable<size_t> {
  void initialize_invalid(const TranslationGrammar& tg) override {
    invalid_ = tg.rules().size();
  }

  void insert_rule(const size_t insertedRule, const size_t i) override {
    // a rule is already present
    if (table_[i] != invalid_) {
      throw std::invalid_argument(
          "Constructing LLTable from a non-LL TranslationGrammar.");
    }
    table_[i] = insertedRule;
  }

 public:
  LLTable(const TranslationGrammar& tg, const vector<vector<Symbol>>& predict) {
    initialize(tg, predict);
  }

  LLTable() { invalid_ = 0; }
};

class PriorityLLTable: public LLTable {
  void insert_rule(const size_t insertedRule, const size_t i) override {
    // insert high priority rule
    if (table_[i] == invalid_ || table_[i] > insertedRule) {
      table_[i] = insertedRule;
    }
  }

public:
  PriorityLLTable(const TranslationGrammar& tg, const vector<vector<Symbol>>& predict) {
    initialize(tg, predict);
  }

  PriorityLLTable() { invalid_ = 0; }
};

class GeneralLLTable : public DecisionTable<vector<size_t>> {
  void insert_rule(const size_t insertedRule, const size_t i) override {
    cell inserted{insertedRule};
    cell newTable{};
    std::set_union(table_[i].begin(), table_[i].end(), inserted.begin(),
                   inserted.end(), std::back_inserter(newTable));
    std::swap(table_[i], newTable);
  }

  void initialize_invalid(const TranslationGrammar&) override { invalid_ = {}; }

 public:
  GeneralLLTable(const TranslationGrammar& tg,
                 const vector<vector<Symbol>>& predict) {
    initialize(tg, predict);
  }

  GeneralLLTable() { initialize(); };
};
}  // namespace ctf

#endif
/*** End of file ll_table.hpp ***/