#include <translation_control.h>

namespace bp {

LLTranslationControl::LLTranslationControl(LexicalAnalyzer &la,
                                           TranslationGrammar &tg)
{
    set_grammar(tg);
    set_lexical_analyzer(la);
}

void LLTranslationControl::set_grammar(const TranslationGrammar &tg)
{
    translationGrammar_ = &tg;
    create_ll_table();
}

void LLTranslationControl::run()
{
    using Type = Symbol::Type;

    if (!lexicalAnalyzer_)
        throw TranslationControlException("No lexical analyzer was attached.");
    else if (!translationGrammar_)
        throw TranslationControlException(
            "No translation grammar was attached.");

    stack<Symbol> input;
    stack<Symbol> output;
    vector<Terminal> inputString;
    vector<const Rule *> rules;

    Terminal token = next_token(inputString);

    input.push(Symbol::EOI());
    input.push(translationGrammar_->starting_symbol());
    while (1) {
        Symbol &top = input.top();
        size_t ruleIndex;
        switch (top.type) {
        case Type::EOI:
            if (token.name() == "")
                return;
            else
                throw TranslationControlException(
                    "Unexpected token after derivation is done.");
            break;
        case Type::TERMINAL:
            if (top.terminal == token) {
                input.pop();
                token = next_token(inputString);
            } else {
                throw TranslationControlException("Unexpected token.");
            }
            break;
        case Type::NONTERMINAL:
            ruleIndex = llTable_.rule_index(top.nonterminal, token);
            if (ruleIndex < translationGrammar_->rules().size()) {
                auto &rule = translationGrammar_->rules()[ruleIndex];
                input.pop();
                for (auto &s : rule.input()) {
                    input.push(s);
                }
                rules.push_back(&(rule));
            } else {
                throw TranslationControlException("No rule can be applied.");
            }
            break;
        default:
            break;
        }
    }
}

void LLTranslationControl::create_ll_table()
{
    create_empty();
    create_first();
    create_follow();
    create_predict();

    llTable_ = LLTable(*translationGrammar_, predict_);
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
        for (auto &r : tg.rules()) {
            bool isempty = true;
            for (auto &s : r.input()) {
                switch (s.type) {
                case Symbol::Type::TERMINAL:
                    isempty = false;
                    break;
                case Symbol::Type::NONTERMINAL:
                    if (empty_[tg.nonterminal_index(s.nonterminal)] == false) {
                        isempty = false;
                    }
                    break;
                default:
                    break;
                }
            }
            if (isempty) {
                if (!empty_[tg.nonterminal_index(r.nonterminal())]) {
                    changed = true;
                    empty_[tg.nonterminal_index(r.nonterminal())] = true;
                }
            }
        }
    } while (changed);
}

void LLTranslationControl::create_first()
{
    const TranslationGrammar &tg = *translationGrammar_;
    first_ = {tg.nonterminals().size(), vector<Terminal>{}};

    bool changed = false;
    do {
        changed = false;
        for (auto &r : tg.rules()) {
            size_t i = tg.nonterminal_index(r.nonterminal());
            bool empty = true;
            for (auto &symbol : r.input()) {
                if (!empty)
                    break;
                size_t nonterm_i;
                switch (symbol.type) {
                case Symbol::Type::NONTERMINAL:
                    nonterm_i = tg.nonterminal_index(symbol.nonterminal);
                    if (modify_set(first_[i], first_[nonterm_i]))
                        changed = true;
                    empty = empty_[nonterm_i];
                    break;
                case Symbol::Type::TERMINAL:
                    if (modify_set(first_[i],
                                   vector<Terminal>({symbol.terminal})))
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
    const TranslationGrammar &tg = *translationGrammar_;
    follow_ = {tg.nonterminals().size(), vector<Terminal>{}};
    follow_[tg.nonterminal_index(tg.starting_symbol().nonterminal)].push_back(
        Terminal::EOI());

    bool changed = false;
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
            for (auto it = r.input().rbegin(); it < r.input().rend(); ++it) {
                // index of nonterminal in input string, only valid with
                // nonterminal symbol
                size_t ti = 0;
                switch (it->type) {
                case Symbol::Type::NONTERMINAL:
                    ti = tg.nonterminal_index(it->nonterminal);
                    if (modify_set(follow_[ti], compoundFirst))
                        changed = true;
                    break;
                    if (compoundEmpty && modify_set(follow_[ti], follow_[i]))
                        changed = true;
                default:
                    break;
                }
                /* if empty = false */
                if (it->type != Symbol::Type::NONTERMINAL ||
                    !empty_[tg.nonterminal_index(it->nonterminal)]) {
                    compoundEmpty = false;
                    switch (it->type) {
                    case Symbol::Type::NONTERMINAL:
                        compoundFirst = first_[ti];
                        break;
                    case Symbol::Type::TERMINAL:
                        compoundFirst = {it->terminal};
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
        vector<Terminal> rfollow =
            follow_[tg.nonterminal_index(r.nonterminal())];
        bool compoundEmpty = true;
        for (auto it = r.input().rbegin(); it < r.input().rend(); ++it) {
            size_t i;
            switch (it->type) {
            case Symbol::Type::TERMINAL:
                compoundEmpty = false;
                compoundFirst = vector<Terminal>({it->terminal});
                break;
            case Symbol::Type::NONTERMINAL:
                i = tg.nonterminal_index(it->nonterminal);
                if (!empty_[i]) {
                    compoundEmpty = false;
                    compoundFirst = first_[i];
                } else {
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