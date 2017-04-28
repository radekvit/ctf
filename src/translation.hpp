/**
\file translation.hpp
\brief Defines class Translation and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION
#define CTF_TRANSLATION

#include <istream>
#include <memory>
#include <ostream>
#include <sstream>

#include "lexical_analyzer.hpp"
#include "ll_translation_control.hpp"
#include "output_generator.hpp"
#include "translation_control.hpp"
#include "translation_grammar.hpp"

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
  \brief Constructs Translation with given lexical analyzer, translation
  control, translation grammar and output generator.
  \param[in] la A callable to perform lexical analysis.
  \param[in] tc A translation control to drive the translation.
  \param[in] tg Translation grammar that defines the input and output languages.
  A copy is made.
  \param[in] og A callable to perform output generation.
  */
  Translation(LexicalAnalyzer::token_function la, TranslationControl &tc,
              const TranslationGrammar &tg, OutputGenerator::output_function og)
      : lexicalAnalyzer_(la),
        translationControl_(tc),
        translationGrammar_(tg),
        outputGenerator_(og) {
    translationControl_.set_grammar(translationGrammar_);
    translationControl_.set_lexical_analyzer(lexicalAnalyzer_);
  }
  /**
  \brief Constructs Translation with given lexical analyzer, translation grammar
  and output generator. Translation control is constructed by name.
   \param[in] la A callable to perform lexical analysis.
   \param[in] tcName Name of a built-in translation control.
   \param[in] tg Translation grammar that defines the input and output
  languages.
   \param[in] og A callable to perform output generation.
  */
  Translation(LexicalAnalyzer::token_function la, const string &tcName,
              const TranslationGrammar &tg, OutputGenerator::output_function og)
      : lexicalAnalyzer_(la),
        control_(Translation::control(tcName)),
        translationControl_(*control_),
        translationGrammar_(tg),
        outputGenerator_(og) {
    translationControl_.set_grammar(translationGrammar_);
    translationControl_.set_lexical_analyzer(lexicalAnalyzer_);
  }
  /**
  \brief Constructs Translation with given lexical analyzer, translation
  control, translation grammar and output generator. Custom error messages are
  given for syntax errors.
   \param[in] la A callable to perform lexical analysis.
  \param[in] tc A translation control to drive the translation.
  \param[in] tg Translation grammar that defines the input and output languages.
  \param[in] og A callable to perform output generation
  \param[in] syntaxErrors A callable to provide syntax error messages.
  */
  Translation(LexicalAnalyzer::token_function la, TranslationControl &tc,
              const TranslationGrammar &tg, OutputGenerator::output_function og,
              TranslationControl::error_function syntaxErrors)
      : Translation(la, tc, tg, og) {
    translationControl_.set_syntax_error_message(syntaxErrors);
  }
  /**
  \brief Constructs Translation with given lexical analyzer, translation grammar
  and output generator. Translation control is constructed by name. Custom error
  messages are given for syntax errors.
   \param[in] la A callable to perform lexical analysis.
   \param[in] tcName Name of a built-in translation control.
   \param[in] tg Translation grammar that defines the input and output
  languages.
   \param[in] og A callable to perform output generation.
   \param[in] syntaxErrors A callable to provide syntax error messages.
  */
  Translation(LexicalAnalyzer::token_function la, const string &tcName,
              const TranslationGrammar &tg, OutputGenerator::output_function og,
              TranslationControl::error_function syntaxErrors)
      : Translation(la, tcName, tg, og) {
    translationControl_.set_syntax_error_message(syntaxErrors);
  }
  ~Translation() {}  //= default;

  /**
  \brief Translates input from istream and outputs the translation to ostream.
  \param[in] input Input stream.
  \param[out] output Output stream.
  */
  void run(std::istream &input, std::ostream &output) {
    std::stringstream ss;
    lexicalAnalyzer_.set_input(input);
    outputGenerator_.set_output(ss);
    translationControl_.run();
    auto &outputTokens = translationControl_.output();
    for (auto &t : outputTokens) {
      outputGenerator_.get_token(t);
    }
    //buffers the output so that there is no output in case of a semantic error
    output << ss.str();
  }

  /**
  \brief Factory method for creating TranslationControl variants.
  \param[in] name Name of built-in translation control. Viable options are:
  "ll".
  \returns A std::unique_ptr containing a new translation control.
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
      throw std::invalid_argument("No translation control with name " + name +
                                  ".");
    else
      return (*it).second();
  }
};
}  // namespace ctf

#endif
/*** End of file translation.hpp ***/