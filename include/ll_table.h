/**
\file ll_table.h
\brief Defines class LLTable and its methods.
\author Radek Vít
*/
#ifndef CTF_LL_H
#define CTF_LL_H

#include <base.h>
#include <generic_types.h>
#include <translation_grammar.h>
#include <stdexcept>
#include <utility>

namespace ctf {
/**
\brief Class containing rule indices to be used in a LL controlled translation.
*/
class LLTable {
 public:
  /**
  \brief Type of the cell.
  */
  using cell = size_t;
  /**
  \brief Row type.
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
  map<Nonterminal, size_t> nonterminalMap;
  /**
  \brief Mapping terminals to indices to table_ rows.
  */
  map<Terminal, size_t> terminalMap;

 public:
  /**
  \brief Constructs an empty LL table.
  */
  LLTable() = default;
  /**
  \brief Constructs a LLtable from a translation grammar and a predict set.
  */
  LLTable(const TranslationGrammar &tg, const vector<vector<Terminal>> &predict)
      : table_(tg.nonterminals().size(),
               vector<size_t>(tg.terminals().size() + 1, tg.rules().size())) {
    if (predict.size() != tg.rules().size())
      throw std::invalid_argument(
          "Mismatched predict and TranslationGrammar.rules "
          "sizes when constructing LLTable.");
    /* create index maps for terminals and nonterminals */
    for (size_t i = 0; i < tg.nonterminals().size(); ++i) {
      nonterminalMap.insert(std::make_pair(tg.nonterminals()[i], i));
    }
    for (size_t i = 0; i < tg.terminals().size(); ++i) {
      terminalMap.insert(std::make_pair(tg.terminals()[i], i));
    }
    terminalMap.insert(std::make_pair(Terminal::EOI(), tg.terminals().size()));
    /* fill table */
    for (size_t i = 0; i < tg.rules().size(); ++i) {
      auto &terminals = predict[i];
      size_t ni = nonterminalMap.at(tg.rules()[i].nonterminal());
      for (auto &t : terminals) {
        if (table_[ni][terminalMap.at(t)] != predict.size()) {
          throw std::invalid_argument(
              "Constructing LLTable from a "
              "non-LL TranslationGrammar.");
        }
        table_[ni][terminalMap.at(t)] = i;
      }
    }
  }
  /**
  \brief Returns an index of the rule to be used when t is the current token
  and nt is at the top of input stack. If no rule is applicable, returns
  tg.rules().size().
  */
  size_t rule_index(const Nonterminal &nt, const Terminal &t) {
    return table_[nonterminalMap.at(nt)][terminalMap.at(t)];
  }
};
}  // namespace ctf

#endif
/*** End of file ll_table.h ***/