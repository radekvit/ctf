/**
\file translation.h
\brief Defines class Translation and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION
#define CTF_TRANSLATION

#include <istream>
#include <ostream>
#include <memory>

#include <generic_types.h>
#include <lexical_analyzer.h>
#include <output_generator.h>
#include <translation_control.h>
#include <translation_grammar.h>

#include <ll_translation_control.h>

namespace ctf {
/**
\brief Defines a translation. Can be used multiple times for different inputs
and outputs.
*/
class Translation {
 protected:
  /**
  \brief Provides input terminals from istream.
  */
  LexicalAnalyzer lexicalAnalyzer_;
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
  \brief Outputs output terminals to ostream.
  */
  OutputGenerator outputGenerator_;

 public:
  Translation(LexicalAnalyzer::token_function la, TranslationControl &tc,
              const TranslationGrammar &tg, OutputGenerator::output_function og);
  ~Translation() = default;

  /**
  \brief Translates input from istream and outputs the translation to ostream.
  */
  void run(std::istream &input, std::ostream &output);

  /**
  \brief Factory method for creating TranslationControl variants.
  */
  static std::unique_ptr<TranslationControl> control(const string &name) {
    const static map<string, std::function<std::unique_ptr<TranslationControl>()>> controls{
      {"ll", []()->std::unique_ptr<TranslationControl>{return std::make_unique<LLTranslationControl>();}},
    };
    try {
      return (controls.at(name))();
    }
    catch(std::out_of_range &e) {
      return std::unique_ptr<TranslationControl>(nullptr);
    }

  }
};
}  // namespace ctf

#endif
/*** End of file translation.h ***/