/**
\file ll_table.hpp
\brief Defines class LLTable and its methods.
\author Radek Vít
*/
#ifndef CTF_LL_H
#define CTF_LL_H

#include <stdexcept>
#include <utility>

#include "translation_grammar.hpp"

namespace ctf {
/**
\brief Class containing rule indices to be used in a LL controlled translation.
*/
class LLTable {
 public:
  /**
  \brief Type of the cells.
  */
  using cell = size_t;
  /**
  \brief Type of the rows.
  */
  using row = vector<cell>;

 protected:
  /**
  \brief Table storing rule indices.
  */
  vector<row> table_;
  /**
  \brief Mapping nonterminals to indices to table_.
  */
  map<Symbol, size_t> nonterminalMap_;
  /**
  \brief Mapping terminals to indices to table_ rows.
  */
  map<Symbol, size_t> terminalMap_;
  /**
  \brief Stores invalid rule index.
  */
  size_t invalidRuleIndex_;

 public:
  /**
  \brief Constructs an empty LL table.
  */
  LLTable() = default;
  /**
  \brief Constructs a LLtable from a translation grammar and a predict set.
  \param[in] tg TranslationGrammar. Terminals and Nonterminals are mapped to
  their indices. Rule indices are stored in LLTable.
  \param[in] predict Predict set for table construction.
  */
  LLTable(const TranslationGrammar &tg, const vector<vector<Symbol>> &predict)
      : table_(tg.nonterminals().size(),
               vector<size_t>(tg.terminals().size() + 1, tg.rules().size())),
        invalidRuleIndex_(tg.rules().size()) {
    if (predict.size() != tg.rules().size())
      throw std::invalid_argument(
          "Mismatched predict and TranslationGrammar.rules "
          "sizes when constructing LLTable.");
    /* create index maps for terminals and nonterminals */
    for (size_t i = 0; i < tg.nonterminals().size(); ++i) {
      nonterminalMap_.insert(std::make_pair(tg.nonterminals()[i], i));
    }
    for (size_t i = 0; i < tg.terminals().size(); ++i) {
      terminalMap_.insert(std::make_pair(tg.terminals()[i], i));
    }
    terminalMap_.insert(std::make_pair(Symbol::EOI(), tg.terminals().size()));
    /* fill table */
    for (size_t i = 0; i < tg.rules().size(); ++i) {
      auto &terminals = predict[i];
      // TranslationGrammar should require this to be found
      size_t ni = nonterminalMap_.at(tg.rules()[i].nonterminal());
      for (auto &t : terminals) {
        auto tit = terminalMap_.find(t);
        if (tit == terminalMap_.end())
          throw std::invalid_argument(
              "Terminal in predict not a terminal in translation grammar when "
              "constructing LLTable.");
        size_t ti = tit->second;
        if (table_[ni][ti] != predict.size()) {
          throw std::invalid_argument(
              "Constructing LLTable from a "
              "non-LL TranslationGrammar.");
        }
        table_[ni][terminalMap_.at(t)] = i;
      }
    }
  }
  /**
  \brief Returns an index of the rule to be used when t is the current token
  and nt is at the top of input stack.
  \param[in] nt Nonterminal on the top of the stack.
  \param[in] t Last read terminal.
  \returns Index of the applicable rule.

  If no rule is applicable, returns the index beyond the last rule.
  */
  size_t rule_index(const Symbol &nt, const Symbol &t) {
    // iterator to nonterminal index
    auto ntit = nonterminalMap_.find(nt);
    // iterator to terminal index;
    auto tit = terminalMap_.find(t);
    if (ntit == nonterminalMap_.end() || tit == terminalMap_.end())
      return invalidRuleIndex_;
    return table_[ntit->second][tit->second];
  }
};
}  // namespace ctf

#endif
/*** End of file ll_table.hpp ***/