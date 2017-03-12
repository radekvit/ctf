/**
\file translation.h
\brief Defines class Translation and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION
#define CTF_TRANSLATION

#include <istream>
#include <ostream>

#include <generic_types.h>
#include <lexical_analyzer.h>
#include <output_generator.h>
#include <translation_control.h>
#include <translation_grammar.h>

namespace ctf {
/**
\brief Defines a translation. Can be used multiple times for different inputs
and outputs.
*/
class Translation {
 protected:
  /**
  \brief Reference to LexicalAnalyzer to be used.
  */
  LexicalAnalyzer &lexicalAnalyzer_;
  /**
  \brief Reference to TranslationControl to be used.
  */
  TranslationControl &translationControl_;
  /**
  \brief Translation grammar that defines accepted language and output
  language.
  */
  TranslationGrammar translationGrammar_;
  /**
  \brief Reference to OutputGenerator to be used.
  */
  OutputGenerator &outputGenerator_;

 public:
  Translation(LexicalAnalyzer &la, TranslationControl &tc,
              TranslationGrammar &tg, OutputGenerator &og);
  ~Translation() = default;

  /**
  \brief Translates input from istream and outputs the translation to ostream.
  */
  void run(std::istream &input, std::ostream &output);
};
}  // namespace ctf

#endif
/*** End of file translation.h ***/