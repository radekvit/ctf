![ctf logo](media/ctf-logo-medium.png)

# CTF ain't a Translation Framework

CTF is a C++17 framework for rapid translation definition. Translations are defined with lexical analyzers, translation grammars and output generators. The API is subject to change in the future slightly to allow a more rigid way to define custom error messages and to provide a solid API for future translation methods.

## Getting started

* See **[getting started](docs/getting_started.md)** for a brief tutorial
* See the **[full documentation](http://www.stud.fit.vutbr.cz/~xvitra00/ctf/)**

## Translation Definition
### Lexical analyzer
Lexical analyzers extract tokens from the input. We encourage you to use other C++ tools to do this job, integrating them to CTF should be very easy.

### Translation grammar
Translation grammars are formal systems that describe translations, where output is defined for each production. See the **[examples](docs/translation_grammars.md)** to learn more. In CTF, translation grammars also describe the relationship between input and output token attributes.

Output generators make semantic checks and transform output tokens to text or binary output.

## Self-building translation units

All tables used during the translation are automatically created from the translation grammars,
speeding up development greatly.

## Supported translation methods

* predictive top-down translation - "ll", "predictive"

## Future translation methods

* LR bottom-up translation - "lr"
