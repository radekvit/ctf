#ifndef XVITRA00_TRANSLATION
#define XVITRA00_TRANSLATION

#include <istream>
#include <ostream>

#include <lexical_analyzer.h>
#include <translation_control.h>
#include <translation_grammar.h>
#include <semantic_analysis.h>

namespace bp {

class TranslationException
{
};

class Translation {
    LexicalAnalyzer &lexicalAnalyzer_;
    TranslationControl &translationControl_;
    TranslationGrammar &translationGrammar_;
    OutputGenerator &outputGenerator_;

public:
    Translation(Lexical_analyzer &la, TranslationControl &tc, TranslationGrammar &tg, OutputGenerator &og);
    ~Translation() = default;

    void run(std::istream &input, std::ostream &output);
};

}

#endif