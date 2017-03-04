#ifndef XVITRA00_LL_H
#define XVITRA00_LL_H

#include <base.h>
#include <generic_types.h>
#include <stdexcept>
#include <translation_grammar.h>
#include <utility>

#include <iostream>//TEMP

namespace bp {

class LLTable {
public:
    using cell = size_t;
    using row = vector<cell>;

protected:
    vector<row> table_;
    map<Nonterminal, size_t> nonterminalMap;
    map<Terminal, size_t> terminalMap;

public:
    LLTable() = default;
    LLTable(const TranslationGrammar &tg,
            const vector<vector<Terminal>> &predict)
        : table_(tg.nonterminals().size(),
                 vector<size_t>(tg.terminals().size() + 1, tg.rules().size()))
    {
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
                    throw std::invalid_argument("Constructing LLTable from a "
                                                "non-LL TranslationGrammar.");
                }
                table_[ni][terminalMap.at(t)] = i;
            }
        }
#if 0
        //TEMP
        using std::cout;
        cout << "\t";
        for(auto &t: tg.terminals())
            cout << t.name() << "\t";
        cout << "\n";
        for(size_t i = 0; i < tg.nonterminals().size(); ++i)
        {
            auto &nt = tg.nonterminals()[i];
            cout << nt.name() << "\t";
            for(size_t i2 = 0; i2 < tg.terminals().size() + 1; ++i2) {
                auto &t = tg.terminals()[i2];
                auto & rule = tg.rules()[table_[i][i2]];
                if(table_[i][i2] < tg.rules().size()) {
                    cout << rule.nonterminal().name() << "-";
                    for(auto &s: rule.input()) {
                        cout << s.terminal.name() << s.nonterminal.name();
                    }
                }
                cout << "\t";
                cout << std::flush;
            }
            cout << "\n";
        }
#endif
    }

    size_t rule_index(const Nonterminal &nt, const Terminal &t)
    {
        return table_[nonterminalMap.at(nt)][terminalMap.at(t)];
    }
};
}

#endif