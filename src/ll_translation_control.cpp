#include <translation_control.h>

namespace bp {

void LLTranslationControl::create_ll_table()
{
    create_empty();
    create_first();
    create_follow();
    create_predict();

    // TODO finish
}

void LLTranslationControl::create_empty()
{
    const TranslationGrammar &tg = *translationGrammar_;
    empty_ = vector<bool>(tg.nonterminals().size(), false);

    for (auto &r : tg.rules()) {
        if (r.input().size() == 0) {
			empty_[tg.nonterminal_index(r.nonterminal())] = true;			
        }
    }

	bool changed = false;
	do {
    	changed = false;
    	for(auto &r: tg.rules()) {
			bool isempty = true;
			for (auto &s: r.input())
			{
				switch(s.type) {
				case Symbol::Type::TERMINAL:
					isempty = false;
					break;
				case Symbol::Type::NONTERMINAL:
					if(empty_[tg.nonterminal_index(s.nonterminal)] == false) {
						isempty = false;
					}
					break;
				default:
					break;
				}
			}
			if(isempty) {
				if(!empty_[tg.nonterminal_index(r.nonterminal())]) {
					changed = true;
					empty_[tg.nonterminal_index(r.nonterminal())] = true;
				}
			}
   		}
	} while (changed);
}

void LLTranslationControl::create_first()
{
    first_.clear();
    const TranslationGrammar &tg = *translationGrammar_;
    for (auto &n : tg.nonterminals()) {
        (void)n;
        first_.push_back(vector<Terminal>());
    }
    bool changed = false;
    do {
        changed = false;
        for (auto &r: tg.rules()) {
            size_t i = tg.nonterminal_index(r.nonterminal());
            bool empty = true;
            for(auto &symbol: r.input()) {
                if(!empty)
                    break;
                size_t nonterm_i;
                switch(symbol.type) {
                case Symbol::Type::NONTERMINAL:
                    nonterm_i = tg.nonterminal_index(symbol.nonterminal);
                    if(modify_set(first_[i], first_[nonterm_i]))
                        changed = true;
                    empty = empty_[nonterm_i];
                    break;
                case Symbol::Type::TERMINAL:
                    if(modify_set(first_[i], vector<Terminal>({symbol.terminal})))
                        changed = true;
                    empty = false;
                    break;
                default:
                    break;
                }
            }
        }
    } while (changed);
}


void LLTranslationControl::create_follow()
{
    follow_.clear();
    const TranslationGrammar &tg = *translationGrammar_;    
    for (auto &n : tg.nonterminals()) {
        (void)n;
        follow_.push_back(vector<Terminal>());
    }
    follow_[tg.nonterminal_index(tg.starting_symbol().nonterminal)].push_back(
        Terminal::EOI());

    bool changed;
    do {
        changed = false;
        for (auto &r : tg.rules()) {
            // index of origin nonterminal
            size_t i = tg.nonterminal_index(r.nonterminal());
            /* empty set of all symbols to the right of the current one */
            bool compoundEmpty = true;
            /* first set of all symbols to the right of the current symbol */
            vector<Terminal> compoundFirst;
            /* track symbols from back */
            for(auto it = r.input().rbegin(); it < r.input().rend(); ++it) {
                // index of nonterminal in input string, only valid with nonterminal symbol
                size_t ti = 0;
                switch(it->type) {
                case Symbol::Type::NONTERMINAL:
                    ti = tg.nonterminal_index(it->nonterminal);
                    if(modify_set(first_[ti], compoundFirst))
                        changed = true;
                    break;
                    if(compoundEmpty && modify_set(first_[ti], first_[i]))
                        changed = true;
                default:
                    break;
                }
                /* if empty = false */
                if(it->type != Symbol::Type::NONTERMINAL || !empty_[tg.nonterminal_index(it->nonterminal)]) {
                    compoundEmpty = false;
                    switch(it->type) {
                    case Symbol::Type::NONTERMINAL:
                        compoundFirst = first_[ti];
                        break;
                    case Symbol::Type::TERMINAL:
                        compoundFirst = vector<Terminal>({it->terminal});
                        break;
                    default:
                        break;
                    }
                }
                /* empty = true, nonterminal*/
                else {
                    modify_set(compoundFirst, first_[ti]);
                }                
            }
        }
    } while (changed);
}

void LLTranslationControl::create_predict()
{
    predict_.clear();
    const TranslationGrammar &tg = *translationGrammar_;
    for (auto &r : tg.rules()) {
        vector<Terminal> compoundFirst;
        vector<Terminal> rfollow = follow_[tg.nonterminal_index(r.nonterminal())];
        bool compoundEmpty = true;
        for (auto it = r.input().rbegin(); it < r.input().rend(); ++it) {
            size_t i;
            switch(it->type) {
            case Symbol::Type::TERMINAL:
                compoundEmpty = false;
                compoundFirst = vector<Terminal>({it->terminal});
                break;
            case Symbol::Type::NONTERMINAL:
                i = tg.nonterminal_index(it->nonterminal);
                if(!empty_[i]) {
                    compoundEmpty = false;
                    compoundFirst = first_[i];
                }
                else {
                    modify_set(compoundFirst, first_[i]);
                }
            default:
                break;
            }
        }
        predict_.push_back(compoundFirst);

        if (compoundEmpty) {
            modify_set(predict_.back(), rfollow);
        }
    }
}


}