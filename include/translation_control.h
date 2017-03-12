/**
\file translation_control.h
\brief Defines class TranslationControl and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_CONTROL
#define CTF_TRANSLATION_CONTROL

#include <base.h>
#include <lexical_analyzer.h>
#include <ll_table.h>
#include <translation_grammar.h>

namespace ctf {

/**
\brief Exception class for TranslationControl specific exceptions.
*/
class TranslationControlException : public TranslationException {
    using TranslationException::TranslationException;
};

/**
\brief Abstract class for syntax driven translation control.
*/
class TranslationControl {
protected:
    /**
    \brief Alias for TranslationGrammar::Rule
    */
    using Rule = TranslationGrammar::Rule;

    /**
    \brief Lexical analyzer for getting tokens from input.
    */
    LexicalAnalyzer *lexicalAnalyzer_ = nullptr;
    /**
    \brief Translation grammar defining the input and output languages.
    */
    const TranslationGrammar *translationGrammar_ = nullptr;
    /**
    \brief Tstack of output symbols.
    */
    tstack<Symbol> output_;

public:
    virtual ~TranslationControl() = default;

    /**
    \brief Sets lexical analyzer.
    */
    void set_lexical_analyzer(LexicalAnalyzer &la) { lexicalAnalyzer_ = &la; }
    /**
    \brief Sets translation grammar.
    */
    virtual void set_grammar(const TranslationGrammar &tg)
    {
        translationGrammar_ = &tg;
    }
    /**
    \brief Runs translation.
    */
    virtual void run() = 0;
    /**
    \brief Gets token from lexicalAnalyzer_ and stores it in a given vector.
    Returns this terminal.
    */
    Token next_token(vector<Terminal> &string)
    {
        string.push_back(lexicalAnalyzer_->get_token());
        return string.back();
    }
    /**
    \brief Returns a constant reference to output symbols.
    */
    const tstack<Symbol> &output() const { return output_; }
};
} // namespace ctf
#endif
/*** End of file translation_control.h ***/