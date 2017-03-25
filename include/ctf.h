/**
\file ctf.h
\brief Single header to be included by end user.
\author Radek Vít

If the user wants to keep namespace ctf, CTF_NO_USING_NAMESPACE shoud be defined
before including this header.
*/
#ifndef CTF_CTF_HEADER
#define CTF_CTF_HEADER

#include "ll_translation_control.h"
#include "translation.h"

#ifndef CTF_NO_USING_NAMESPACE
// Translation class
using namespace ctf;

#endif  // endif CTF_KEEP_NAMESPACE

#endif
/*** End of file ctf.h ***/