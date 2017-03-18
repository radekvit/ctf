#ifndef CTF_CTF_HEADER
#define CTF_CTF_HEADER

#include "ll_translation_control.h"
#include "translation.h"

#ifndef CTF_KEEP_NAMESPACE
// Translation class
using ctf::Translation;
using ctf::TranslationControl;

using ctf::TranslationGrammar;
using ctf::LexicalAnalyzer;
using ctf::OutputGenerator;

using ctf::TranslationException;

using ctf::LLTranslationControl;

#endif  // endif CTF_KEEP_NAMESPACE

#endif