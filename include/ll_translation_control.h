/**
\file ll_translation_control.h
\brief Defines class LLTranslationControl and its methods.
\author Radek Vít
*/
#ifndef CTF_LL_TRANSLATION_CONTROL
#define CTF_LL_TRANSLATION_CONTROL

#include <translation_control.h>

namespace ctf {

/**
\brief Implements LL top down translation control.
*/
class LLTranslationControl : public TranslationControl {
 protected:
  /**
  \brief Empty set for each nonterminal.
  */
  vector<bool> empty_;
  /**
  \brief First set for each nonterminal.
  */
  vector<vector<Symbol>> first_;
  /**
  \brief Follow set for each nonterminal.
  */
  vector<vector<Symbol>> follow_;
  /**
  \brief Predict set for each nonterminal.
  */
  vector<vector<Symbol>> predict_;

  /**
  \brief LL table used to control the translation.
  */
  LLTable llTable_;

  /**
  \brief String of terminals got by LexicalAnalyzer_.
  */
  vector<Symbol> inputString_;

  /**
  Creates all sets and creates a new LL table.
  */
  void create_ll_table();

  /**
  \brief Creates Empty set for each nonterminal.
  */
  void create_empty();
  /**
  \brief Creates First set for each nonterminal.
  */
  void create_first();
  /**
  \brief Creates Follow set for each nonterminal.
  */
  void create_follow();
  /**
  \brief Creates Predict set for each nonterminal.
  */
  void create_predict();

  /**
  \brief Creates iterator attribute actions for incoming terminals.
  \param[in] obegin Iterator to the first Symbol of the output of the applied
  Rule.
  \param[in] targets Indices of the target actions for all input terminals.
  \param[out] attributeActions Targets to append incoming terminal's attributes.
  */
  void create_attibute_actions(
      tstack<Symbol>::iterator obegin, const vector<vector<size_t>> &targets,
      tstack<vector<tstack<Symbol>::iterator>> &attributeActions);

 public:
  /**
  \brief Constructs a LLTranslationControl.
  */
  LLTranslationControl() = default;
  /**
  \brief Default destructor.
  */
  virtual ~LLTranslationControl() = default;
  /**
  \brief Constructs LLTranslationControl with a LexicalAnalyzer and
  TranslationGrammar.
  \param[in] la A reference to the lexical analyzer to be used to get tokens.
  \param[in] tg The translation grammar for this translation.
  */
  LLTranslationControl(LexicalAnalyzer &la, TranslationGrammar &tg);

  /**
  \brief Sets translation grammar.
  \param[in] tg The translation grammar for this translation.
  */
  virtual void set_grammar(const TranslationGrammar &tg);

  /**
  \brief Runs the translation. Output symbols are stored in output_.
  */
  virtual void run();
};
}  // namespace ctf
#endif
/*** End of file ll_translation_control.h ***/