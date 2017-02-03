#ifndef XVITRA00_TRANSLATION
#define XVITRA00_TRANSLATION

#include <istream>
#include <ostream>

#include <lexical_analyzer.h>
#include <translation_control.h>
#include <translation_grammar.h>
#include <semantic_analysis.h>

namespace bp {

class Translation {
public:
    Translation(Lexical_analyzer &la, TranslationControl &tc, TranslationGrammar &tg, SemanticAnalyzer &sa);
    ~Translation() = default;

    void run(std::istream &input, std::ostream &output);
}

}

#endif