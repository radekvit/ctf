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
  vector<vector<Terminal>> first_;
  /**
  \brief Follow set for each nonterminal.
  */
  vector<vector<Terminal>> follow_;
  /**
  \brief Predict set for each nonterminal.
  */
  vector<vector<Terminal>> predict_;

  /**
  \brief LL table used to control the translation.
  */
  LLTable llTable_;

  /**
  \brief Input tstack holding symbols.
  */
  tstack<Symbol> input_;
  /**
  \brief String of terminals got by LexicalAnalyzer_.
  */
  vector<Terminal> inputString_;

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
  \brief Creates iterator attribute targets for incoming terminals.
  */
  void create_attibute_targets(
      tstack<Symbol>::iterator obegin, const vector<vector<size_t>> &targets,
      tstack<vector<tstack<Symbol>::iterator>> &attributeTargets);

 public:
  LLTranslationControl() = default;
  virtual ~LLTranslationControl() = default;
  LLTranslationControl(LexicalAnalyzer &la, TranslationGrammar &tg);

  virtual void set_grammar(const TranslationGrammar &tg);

  /**
  \brief Runs the translation. Output symbols are stored in output_.

  If the translation fails, an appropriate exception is thrown.
  */
  virtual void run();
};
}  // namespace ctf
#endif
/*** End of file ll_translation_control.h ***/