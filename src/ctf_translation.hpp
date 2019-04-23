/**
\file translation.hpp
\brief Defines class Translation and its methods.
\author Radek Vít
*/
#ifndef CTF_TRANSLATION_H
#define CTF_TRANSLATION_H

#include <fstream>
#include <istream>
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

using CanonicalLR1 = LR1TranslationControl;
using LALR = LALRTranslationControl;
using LSCELR = LSCELRTranslationControl;

inline SavedLRTranslationControl load(std::istream& is) { return SavedLRTranslationControl(is); }

inline SavedLRTranslationControl load(const string& filename) {
  std::ifstream is(filename);
  if (is.fail()) {
    throw std::invalid_argument(string("Could not open file") + filename + ".");
  }
  return load(is);
}

inline SavedLRTranslationControl load(const char* input) {
  std::stringstream ss;
  ss << input;

  return load(ss);
}

/**
\brief Defines a translation. Can be used multiple times for different inputs
and outputs.
*/
template <typename TLexicalAnalyzer,
          typename TOutputGenerator,
          typename TTranslationControl = LSCELR>
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
  Translation(TLexicalAnalyzer&& la,
              const TranslationGrammar& tg,
              TOutputGenerator&& og,
              symbol_string_fn to_str = ctf::to_string)
    : _lexicalAnalyzer(std::move(la))
    , _translationControl()
    , _translationGrammar(tg)
    , _outputGenerator(std::move(og))
    , _toString(to_str) {
    _translationControl.set_lexical_analyzer(_lexicalAnalyzer);
    _translationControl.set_grammar(_translationGrammar, _toString);
  }

  /**
  \brief Constructs Translation with given lexical analyzer, translation
  control, translation grammar and output generator.
  \param[in] la A callable to perform lexical analysis.
  \param[in] tc A translation control to drive the translation.
  \param[in] tg Translation grammar that defines the input and output languages.
  A copy is made.
  \param[in] og A callable to perform output generation.
  */
  Translation(TLexicalAnalyzer&& la,
              TTranslationControl&& tc,
              const TranslationGrammar& tg,
              TOutputGenerator&& og,
              symbol_string_fn to_str = ctf::to_string)
    : _lexicalAnalyzer(std::move(la))
    , _translationControl(std::move(tc))
    , _translationGrammar(tg)
    , _outputGenerator(std::move(og))
    , _toString(to_str) {
    _translationControl.set_lexical_analyzer(_lexicalAnalyzer);
    _translationControl.set_grammar(_translationGrammar, _toString);
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
                        const std::string& inputName = "") {
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
      _translationControl.run(_reader, _toString);
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
      auto& outputTokens = _translationControl.output();
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

  void save(std::ostream& os) const { _translationControl.save(os); }

 protected:
  /**
  \brief The input reader and buffer.
  */
  InputReader _reader;
  /**
  \brief Provides input terminals from istream.
  */
  TLexicalAnalyzer _lexicalAnalyzer;
  /**
  \brief The control class performing the translation.
  */
  TTranslationControl _translationControl;
  /**
  \brief A translation grammar that defines accepted language and output
  language.
  */
  TranslationGrammar _translationGrammar;
  /**
  \brief Outputs output terminals to ostream or elsewhere.
  */
  TOutputGenerator _outputGenerator;

  symbol_string_fn _toString;
};
}  // namespace ctf

#endif
/*** End of file translation.hpp ***/