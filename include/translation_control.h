#ifndef XVITRA00_TRANSLATION_CONTROL
#define XVITRA00_TRANSLATION_CONTROL

#include <base.h>
#include <lexical_analyzer.h>
#include <translation_grammar.h>
#include <ll_table.h>

namespace bp {

class TranslationControlException: public TranslationException {
    using TranslationException::TranslationException;
};

class TranslationControl {
protected:
    using Rule = TranslationGrammar::Rule;

    LexicalAnalyzer *lexicalAnalyzer_ = nullptr;
    const TranslationGrammar *translationGrammar_ = nullptr;
public:
    virtual ~TranslationControl() = 0;

    void set_lexical_analyzer(LexicalAnalyzer &la) {
        lexicalAnalyzer_ = &la;
    }

    virtual void set_grammar(const TranslationGrammar &tg) {
        translationGrammar_ = &tg;
    }

    virtual vector<Terminal> run() = 0;

    Token next_token(vector<Terminal> &string) {
        string.push_back(lexicalAnalyzer_->get_token());
        return string.back();
    }
};

class LLTranslationControl: public TranslationControl {
protected:
    LLTable llTable_;

    void create_ll_table(const TranslationGrammar &tg);
public:
    LLTranslationControl() = default;
    virtual ~LLTranslationControl() = default;
    LLTranslationControl(LexicalAnalyzer &la, TranslationGrammar &tg);

    virtual void set_grammar(const TranslationGrammar &tg);

    virtual void run();
};

}

#endif