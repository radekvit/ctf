/**
\file ll_table.hpp
\brief Defines class LLTable and its methods.
\author Radek Vít
*/
#ifndef CTF_LL_TABLE_H
#define CTF_LL_TABLE_H

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
  \brief Type of cells.
  */
  using cell = size_t;
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
  /**
  \brief Maps 2D indices to 1D indices.
  */
  size_t index(size_t y, size_t x) const { return terminalMap_.size() * y + x; }

 public:
  /**
  \brief Constructs an empty LL table.
  */
  LLTable() = default;
  /**
  \brief Constructs a LLtable from a translation grammar and a predict set.

  \param[in] tg TranslationGrammar. Terminals and Nonterminals are mapped to
  their respective indices. Rule indices are stored in LLTable.
  \param[in] predict Predict set for table construction.
  */
  LLTable(const TranslationGrammar &tg, const vector<vector<Symbol>> &predict)
      : table_(tg.nonterminals().size() * (tg.terminals().size() + 1),
               tg.rules().size()),
        invalidRuleIndex_(tg.rules().size()) {
    if (predict.size() != tg.rules().size())
      throw std::invalid_argument(
          "Mismatched predict and TranslationGrammar.rules "
          "sizes when constructing LLTable.");

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
      auto &terminals = predict[i];
      // TranslationGrammar requires this to be always found
      size_t ni = nonterminalMap_.at(tg.rules()[i].nonterminal());

      for (auto &t : terminals) {
        auto tit = terminalMap_.find(t);
        if (tit == terminalMap_.end())
          throw std::invalid_argument(
              "Terminal in predict not a terminal in translation grammar when "
              "constructing LLTable.");

        size_t ti = tit->second;
        // a rule is already present
        if (table_[index(ni, ti)] != invalidRuleIndex_) {
          throw std::invalid_argument(
              "Constructing LLTable from a non-LL TranslationGrammar.");
        }
        table_[index(ni, terminalMap_.at(t))] = i;
      }  // for all terminals
    }    // for all i
  }
  /**
  \brief Returns an index of the rule to be used when t is the current token
  and nt is at the top of input stack.

  \param[in] nt Nonterminal on the top of the stack.
  \param[in] t Last read terminal.

  \returns Index of the applicable rule or invalid rule index.

  If no rule is applicable, returns the index beyond the last rule.
  */
  size_t rule_index(const Symbol &nt, const Symbol &t) noexcept {
    // iterator to nonterminal index
    auto ntit = nonterminalMap_.find(nt);
    // iterator to terminal index;
    auto tit = terminalMap_.find(t);
    // either of the arguments not found
    if (ntit == nonterminalMap_.end() || tit == terminalMap_.end())
      return invalidRuleIndex_;
    // returning from LL table
    return table_[index(ntit->second, tit->second)];
  }
};
}  // namespace ctf

#endif
/*** End of file ll_table.hpp ***/