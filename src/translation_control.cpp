#include <generic_types.h>
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
            ruleIndex = llTable_.rule_index(top, token);
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
}