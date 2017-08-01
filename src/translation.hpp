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
\brief Exception class for syntax errors.
*/
class TranslationError : public TranslationException {
  using TranslationException::TranslationException;
};
/**
\brief An exception class for semantic errors.
*/
class SemanticError : public TranslationException {
 public:
  using TranslationException::TranslationException;
};
/**
\brief Defines a translation. Can be used multiple times for different inputs
and outputs.
*/
class Translation {
 protected:
  /**
  \brief The input reader and buffer.
  */
  InputReader reader_;
  /**
  \brief Lexical Analyzer ownership.
  */
  std::unique_ptr<LexicalAnalyzer> lexer_;
  /**
  \brief Provides input terminals from istream.
  */
  LexicalAnalyzer &lexicalAnalyzer_;
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
  \brief Output generator ownership
  */
  std::unique_ptr<OutputGenerator> generator_;
  /**
  \brief Outputs output terminals to ostream.
  */
  OutputGenerator &outputGenerator_;

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
  Translation(std::unique_ptr<LexicalAnalyzer> &&la, TranslationControl &tc,
              const TranslationGrammar &tg,
              std::unique_ptr<OutputGenerator> &&og)
      : lexer_(std::move(la)),
        lexicalAnalyzer_(*lexer_),
        translationControl_(tc),
        translationGrammar_(tg),
        generator_(std::move(og)),
        outputGenerator_(*generator_) {
    translationControl_.set_lexical_analyzer(lexicalAnalyzer_);
    translationControl_.set_grammar(translationGrammar_);
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
  Translation(std::unique_ptr<LexicalAnalyzer> &&la, const string &tcName,
              const TranslationGrammar &tg,
              std::unique_ptr<OutputGenerator> &&og)
      : lexer_(std::move(la)),
        lexicalAnalyzer_(*lexer_),
        control_(Translation::control(tcName)),
        translationControl_(*control_),
        translationGrammar_(tg),
        generator_(std::move(og)),
        outputGenerator_(*generator_) {
    translationControl_.set_grammar(translationGrammar_);
    translationControl_.set_lexical_analyzer(lexicalAnalyzer_);
  }
  ~Translation() {}  //= default;

  /**
  \brief Translates input from istream and outputs the translation to ostream.
  \param[in] input Input stream.
  \param[out] output Output stream.
  */
  void run(std::istream &input, std::ostream &output, const string &inputName = "") {
    // extra output buffer
    std::stringstream ss;
    // setup
    translationControl_.reset();
    lexicalAnalyzer_.reset();
    lexicalAnalyzer_.set_reader(reader_);
    reader_.set_stream(input, inputName);
    outputGenerator_.set_stream(ss);

    // lexical analysis, syntax analysis and translation
    translationControl_.run();

    // translation error
    if (lexicalAnalyzer_.error() || translationControl_.error()) {
      throw TranslationError{lexicalAnalyzer_.error_message() +
                             translationControl_.error_message()};
    }

    // semantic analysis and code generation
    auto &outputTokens = translationControl_.output();
    outputGenerator_.output(outputTokens);

    // semantic error
    if (outputGenerator_.error()) {
      throw SemanticError{outputGenerator_.error_message()};
    }

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
            {"predictive",
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