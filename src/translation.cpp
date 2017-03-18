/**
\file translation.cpp
\brief Implements methods of class Translation.
\author Radek Vít
*/
#include <translation.h>

namespace ctf {

Translation::Translation(LexicalAnalyzer &la, TranslationControl &tc,
                         TranslationGrammar &tg, OutputGenerator &og)
    : lexicalAnalyzer_(la),
      translationControl_(tc),
      translationGrammar_(tg),
      outputGenerator_(og) {
  translationControl_.set_grammar(translationGrammar_);
  translationControl_.set_lexical_analyzer(lexicalAnalyzer_);
}

void Translation::run(std::istream &input, std::ostream &output) {
  lexicalAnalyzer_.set_input(input);
  outputGenerator_.set_output(output);
  translationControl_.run();
  auto &outputTokens = translationControl_.output();
  for (auto &t : outputTokens) {
    outputGenerator_.get_token(t);
  }
}
}  // namespace ctf
   /*** End of file translation.cpp ***/