#ifndef XVITRA00_TRANSLATION_CONTROL
#define XVITRA00_TRANSLATION_CONTROL

#include <base.h>
#include <lexical_analyzer.h>
#include <ll_table.h>
#include <translation_grammar.h>

namespace bp {

class TranslationControlException : public TranslationException {
    using TranslationException::TranslationException;
};

class TranslationControl {
protected:
    using Rule = TranslationGrammar::Rule;

    LexicalAnalyzer *lexicalAnalyzer_ = nullptr;
    const TranslationGrammar *translationGrammar_ = nullptr;

    tstack<Symbol> output_;

public:
    virtual ~TranslationControl() = default;

    void set_lexical_analyzer(LexicalAnalyzer &la) { lexicalAnalyzer_ = &la; }

    virtual void set_grammar(const TranslationGrammar &tg)
    {
        translationGrammar_ = &tg;
    }

    virtual void run() = 0;

    Token next_token(vector<Terminal> &string)
    {
        string.push_back(lexicalAnalyzer_->get_token());
        return string.back();
    }

    const tstack<Symbol> &output() { return output_; }
};

class LLTranslationControl : public TranslationControl {
protected:
    vector<bool> empty_;
    vector<vector<Terminal>> first_;
    vector<vector<Terminal>> follow_;
    vector<vector<Terminal>> predict_;

    LLTable llTable_;

    tstack<Symbol> input_;
    vector<Terminal> inputString_;

    void create_ll_table();

    void create_empty();
    void create_first();
    void create_follow();
    void create_predict();

public:
    LLTranslationControl() = default;
    virtual ~LLTranslationControl() = default;
    LLTranslationControl(LexicalAnalyzer &la, TranslationGrammar &tg);

    virtual void set_grammar(const TranslationGrammar &tg);

    virtual void run();
};
}

#endif