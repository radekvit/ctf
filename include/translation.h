/**
\file translation.h
\brief Defines class Translation and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION
#define CTF_TRANSLATION

#include <istream>
#include <memory>
#include <ostream>

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
  \brief Holds standard control when generated with Translation::control().
  */
  std::unique_ptr<TranslationControl> control_ = nullptr;
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
 /**
 \brief Constructs Translation with given lexical analyzer, translation control, translation grammar and output generator.
 */
  Translation(LexicalAnalyzer::token_function la, TranslationControl &tc,
              const TranslationGrammar &tg,
              OutputGenerator::output_function og);
/**
\brief Constructs Translation with given lexical analyzer, translation grammar and output generator. Translation control is constructed by name.
*/
  Translation(LexicalAnalyzer::token_function la, const string &tcName,
              const TranslationGrammar &tg,
              OutputGenerator::output_function og);
 /**
 \brief Constructs Translation with given lexical analyzer, translation control, translation grammar and output generator. Custom error messages are given for syntax errors.
 */ 
  Translation(LexicalAnalyzer::token_function la, TranslationControl &tc,
              const TranslationGrammar &tg,
              OutputGenerator::output_function og,
              TranslationControl::error_function syntaxErrors):
              Translation(la,tc,tg,og)
              {
                translationControl_.set_syntax_error_message(syntaxErrors);
              }
/**
\brief Constructs Translation with given lexical analyzer, translation grammar and output generator. Translation control is constructed by name. Custom error messages are given for syntax errors.
*/
  Translation(LexicalAnalyzer::token_function la, const string &tcName,
              const TranslationGrammar &tg,
              OutputGenerator::output_function og,
              TranslationControl::error_function syntaxErrors):
                            Translation(la,tcName,tg,og)
              {
                translationControl_.set_syntax_error_message(syntaxErrors);
              }
  ~Translation() {} //= default;

  /**
  \brief Translates input from istream and outputs the translation to ostream.
  */
  void run(std::istream &input, std::ostream &output);

  /**
  \brief Factory method for creating TranslationControl variants.
  */
  static std::unique_ptr<TranslationControl> control(const string &name) {
    const static map<string,
                     std::function<std::unique_ptr<TranslationControl>()>>
        controls{
            {"ll",
             []() -> std::unique_ptr<TranslationControl> {
               return std::make_unique<LLTranslationControl>();
             }},
        };
    auto it = controls.find(name);
    if (it == controls.end())
      throw std::invalid_argument("No translation control with name " + name + ".");
    else
      return (*it).second();
  }
};
}  // namespace ctf

#endif
/*** End of file translation.h ***/