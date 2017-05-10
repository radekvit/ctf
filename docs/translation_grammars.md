## Example translation grammars
### Simple expressions
This translation grammar translates very simple infix expression to postfix expressions.
Note that the rule ```{"F"_nt, {"i"_t}, {"i"_t}, {{0}}}``` transfers the attribute of the input terminal *i* to the output terminal *o*. The rule ```{"F"_nt, {"n"}}``` behaves in the same way implicitly.

```c++
TranslationGrammar tg{
  {
    {"E"_nt, {"T"_nt, "E'"_nt}},
    {"E'"_nt, {}},
    {"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}, {"T"_nt, "E'"_nt}},
    {"F"_nt, {"("_t, "E"_nt, ")"_t}, {"E"_nt}},
    {"F"_nt, {"i"_t}, {"o"_t}, {{0}}},
	{"F"_nt, {"n"}},
    {"T"_nt, {"F"_nt, "T'"_nt}},
    {"T'"_nt, {}},
    {"T'"_nt, {"*"_t, "F"_nt, "T'"_nt}, {"F"_nt, "T'"_nt}},
  },
  "E"_nt};
```