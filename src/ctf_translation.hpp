/**
\file translation.hpp
\brief Defines class Translation and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_H
#define CTF_TRANSLATION_H

#include <istream>
#include <memory>
#include <ostream>
#include <sstream>

#include "ctf_lexical_analyzer.hpp"
#include "ctf_lr_translation_control.hpp"
#include "ctf_output_generator.hpp"
#include "ctf_translation_control.hpp"
#include "ctf_translation_grammar.hpp"

namespace ctf {
/**
\brief The potential results of a translation.
*/
enum class TranslationResult {
  SUCCESS = 0,
  LEXICAL_ERROR,
  TRANSLATION_ERROR,  // syntax errors
  SEMANTIC_ERROR,
  CODE_GENERATION_ERROR,
};
/**
\brief Defines a translation. Can be used multiple times for different inputs
and outputs.
*/
class Translation {
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
  Translation(std::unique_ptr<LexicalAnalyzer>&& la,
              TranslationControl& tc,
              const TranslationGrammar& tg,
              std::unique_ptr<OutputGenerator>&& og)
    : _lexer(std::move(la))
    , _lexicalAnalyzer(*_lexer)
    , _translationControl(tc)
    , _translationGrammar(tg)
    , _generator(std::move(og))
    , _outputGenerator(*_generator) {
    _translationControl.set_lexical_analyzer(_lexicalAnalyzer);
    _translationControl.set_grammar(_translationGrammar);
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
  Translation(std::unique_ptr<LexicalAnalyzer>&& la,
              const string& tcName,
              const TranslationGrammar& tg,
              std::unique_ptr<OutputGenerator>&& og)
    : _lexer(std::move(la))
    , _lexicalAnalyzer(*_lexer)
    , _control(Translation::control(tcName))
    , _translationControl(*_control)
    , _translationGrammar(tg)
    , _generator(std::move(og))
    , _outputGenerator(*_generator) {
    _translationControl.set_grammar(_translationGrammar);
    _translationControl.set_lexical_analyzer(_lexicalAnalyzer);
  }
  ~Translation() {}  //= default;

  /**
  \brief Translates input from istream and outputs the translation to ostream.
  \param[in] input Input stream.
  \param[out] output Output stream.

  \returns True when no errors were encountered.
  */
  TranslationResult run(std::istream& inputStream,
                        std::ostream& outputStream,
                        std::ostream& errorStream,
                        const std::string& inputName = "",
                        symbol_string_fn to_str = ctf::to_string) {
    // extra output buffer
    std::stringstream ss;
    // error flags
    bool lexError = false;
    bool synError = false;
    bool semError = false;
    bool genError = false;
    // setup
    _translationControl.reset();
    _lexicalAnalyzer.set_reader(_reader);
    _lexicalAnalyzer.set_error_stream(errorStream);
    _lexicalAnalyzer.reset();
    _reader.set_stream(inputStream, inputName);

    _translationControl.set_error_stream(errorStream);

    _outputGenerator.set_error_stream(errorStream);
    _outputGenerator.set_output_stream(ss);

    try {
      // lexical analysis, syntax analysis and translation
      _translationControl.run(to_str);
    } catch (LexicalException& le) {
      lexError = true;
    } catch (SyntaxException& se) {
      synError = true;
    }

    if (_lexicalAnalyzer.error() || lexError) {
      return TranslationResult::LEXICAL_ERROR;
    } else if (_translationControl.error() || synError) {
      return TranslationResult::TRANSLATION_ERROR;
    }

    // semantic analysis and code generation
    try {
      auto&& outputTokens = _translationControl.output();
      _outputGenerator.output(outputTokens);
    } catch (SemanticException& se) {
      semError = true;
    } catch (CodeGenerationException& cge) {
      genError = true;
    }

    if (_outputGenerator.error() || semError) {
      return TranslationResult::SEMANTIC_ERROR;
    } else if (genError) {
      return TranslationResult::CODE_GENERATION_ERROR;
    }

    outputStream << ss.str();
    return TranslationResult::SUCCESS;
  }

  /**
  \brief Factory method for creating TranslationControl variants.
  \param[in] name Name of built-in translation control. Viable options are:
  "ll".
  \returns A std::unique_ptr containing a new translation control.
  */
  static std::unique_ptr<TranslationControl> control(const string& name) {
    const static unordered_map<string, std::function<std::unique_ptr<TranslationControl>()>>
      controls{
        {"canonical lr",
         []() -> std::unique_ptr<TranslationControl> {
           return std::make_unique<LR1TranslationControl>();
         }},
        {"lalr",
         []() -> std::unique_ptr<TranslationControl> {
           return std::make_unique<LALRTranslationControl>();
         }},
        {"lscelr",
         []() -> std::unique_ptr<TranslationControl> {
           return std::make_unique<LSCELRTranslationControl>();
         }},
      };
    auto it = controls.find(name);
    if (it == controls.end())
      throw std::invalid_argument("No translation control with name " + name + ".");
    else
      return (*it).second();
  }

 protected:
  /**
  \brief The input reader and buffer.
  */
  InputReader _reader;
  /**
  \brief Lexical Analyzer ownership.
  */
  std::unique_ptr<LexicalAnalyzer> _lexer;
  /**
  \brief Provides input terminals from istream.
  */
  LexicalAnalyzer& _lexicalAnalyzer;
  /**
  \brief Holds standard control when generated with Translation::control().
  */
  std::unique_ptr<TranslationControl> _control = nullptr;
  /**
  \brief Reference to TranslationControl to be used.
  */
  TranslationControl& _translationControl;
  /**
  \brief Translation grammar that defines accepted language and output
  language.
  */
  TranslationGrammar _translationGrammar;
  /**
  \brief Output generator ownership
  */
  std::unique_ptr<OutputGenerator> _generator;
  /**
  \brief Outputs output terminals to ostream.
  */
  OutputGenerator& _outputGenerator;
};
}  // namespace ctf

#endif
/*** End of file translation.hpp ***/