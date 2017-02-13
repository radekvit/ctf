#ifndef XVITRA00_TRANSLATION
#define XVITRA00_TRANSLATION

#include <istream>
#include <ostream>

#include <generic_types.h>
#include <lexical_analyzer.h>
#include <translation_control.h>
#include <translation_grammar.h>
#include <output_generator.h>

namespace bp {

class Translation {
protected:
    LexicalAnalyzer &lexicalAnalyzer_;
    TranslationControl &translationControl_;
    TranslationGrammar &translationGrammar_;
    OutputGenerator &outputGenerator_;

public:
    Translation(LexicalAnalyzer &la, TranslationControl &tc, TranslationGrammar &tg, OutputGenerator &og);
    ~Translation() = default;

    void run(std::istream &input, std::ostream &output);
};

}

#endif