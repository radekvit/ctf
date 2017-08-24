# Getting ctf

Download ctf source from [github](https://github.com/radekvit/ctf).

## Where to put it

Ctf is a header only library. You only need your project to reach include/ctf.hpp.
The rest of this tutorial will assume that ctf.h is avaliable unqualified, but you might need to prefix it with a folder name if neccessary.

Everything in CTF is in `namespace ctf`. The header ctf.h has `using namespace ctf;` in it, but this can be switched off by defining `CTF_KEEP_NAMESPACE` before including the header. Note that this will prevent you from using the built-in custom literals; they can be accessed via the use of `using namespace ctf::literals;`.

# Defining translations

Defining translations is a matter of three steps:
 * creating a lexical analyzer
 * defining a translation grammar
 * creating an output generator

We will go through them in order in the following sections.

## Creating a lexical analyzer

A lexical analyzer transforms the binary input to a string of terminal symbols.

In our example lexical analyzer `ExampleLex`, the string before an optional `.` is the token name, and the remaining characters before a newline are token's attribute. If the file doesn't end in a newline or if a token's name would be empty, a lexical error is thrown.

The base class LexicalAnalyzer provides the basic interface and we will create a simple subclass to implement our own simple lexical analyzer. In this section, we will discuss the methods to be overriden in a user-defined class.
### constructors
You may define any custom constuctors from within your class. No explicit parent class constructor calls are neccessary.

### read_token
The method `ctf::Symbol LexicalAnalyzer::read_token()` provides the core functionality of your lexical analyzer. Characters are to be read with the built-in `get` and `unget` methods and terminal symbols are to be created via the `token` method. When a lexical error is encountered, this method should call `set_error()` to set the error flag, and then store the error message (see `error_message` for returning the error message; storing the messages is left up to the user).

### reset_private
This method resets the state of the lexical analyzer. This method is called after the basic lexical analyzer cleanup. This method is called by the public `reset` method, so is must **not** be called by this method. 

### error_message
This method returns the lexical error message. How the message is stored or generated is left to the user to decide.

### clear_message
This method should be overriden to clear all error messages. This method is called from within `reset` and `clear_error`, so they must **not** be called by this method.

### Full ExampleLex Definition
This is the full example class definition.

```c++
#include <ctf.h>

class ExampleLex : public LexicalAnalyzer {
  // we will use the parent class' constuctors
  using LexicalAnalyzer::LexicalAnalyzer;

  Token read_token(std::istream &is) override {
    string name;
    string attribute;
    const int eof = std::char_traits<char>::eof();

    int c = get();
    if (c == eof)
      return Symbol::eof();
    if (c == '.') {
      set_error();
      return Symbol::eof();
    }
    while (c != '.' && c != '\n') {
      if (c == eof) {
        set_error();
        return Symbol::eof();
      }
      name += static_cast<char>(c);

      c = get();
    }
    if(c == '.')
      c = get();
    while (c != '\n') {
      if (c == eof)
        break;
      attribute += string{static_cast<char>(c)};
      c = get();
    }
    // an Attribute object is constructed from the string
    return token(name, attribute);
  }

  void reset_private() noexcept override {
    // custom added resets
  }

  string error_message() override {
    return "Custom error message. A string member variable is usually a good idea.";
  }

  void clear_message() noexcept override {
    // error messages are cleared here
  }
};
```

## Defining a translation grammar

A translation grammar's input is essentially a Context-free grammar: A single nonterminal produces a string of terminals and nonterminals. Every production, however, has an output counterpart. The only limitation is that nonterminals in the output must match the nonterminals in the input. CTF provides a convenient way to write terminal and nonterminal literals as `"terminal"_t` and `"Nonterminal"_nt`, respectively.


If the rule has only the input production, it is taken as both the input and output productions.

In CTF, every rule has an optional attribute action; this defines the relationship between input and output terminals. If a rule is in the form `TranslationGrammar::Rule("A"_nt, {"x"_t, "A"_nt}, {"A"_nt, "y"_t}, {{1}})`, the attribute action states that the first terminal's attribute will be copied to the symbol with index 1 from the production. The target must be a terminal symbol, and the number of sources must be the same as the number of terminals in the input.

We encourage to define translation grammars as a vector of rules and the starting nonterminal only, as the translation grammar's terminals and nonterminals are infered from the provided rules. You can also provide your own sets of terminals and nonterminals if you wish to be warned about the potential mistakes and typos in the rules.

In this example, we define a translation grammar that takes simple infix expressions with addition and multiplication and translates them into postfix expressions.

```c++
#include <ctf.h>

TranslationGrammar grammar{
  // The vector of rules
  {
	  // this rule has identical input and output
    {"E"_nt, {"T"_nt, "E'"_nt}},
    {"E'"_nt, {}},
    {"E'"_nt, {"+"_t, "T"_nt, "E'"_nt}, {"T"_nt, "+"_t, "E'"_nt}},
    {"F"_nt, {"("_t, "E"_nt, ")"_t}, {"E"_nt}},
	  // this rule defines the attribute action for terminal i
    // this would be impplicit for a definition in the form
    // {"F"_nt, {"i"_t}},
    {"F"_nt, {"i"_t}, {"i"_t}, {{0}}},
    {"T"_nt, {"F"_nt, "T'"_nt}},
    {"T'"_nt, {}},
    {"T'"_nt, {"*"_t, "F"_nt, "T'"_nt}, {"F"_nt, "*"_t, "T'"_nt}},
  },
  // this is the starting nonterminal
  "E"_nt
};
```

## Creating an output generator

The output generator transforms the output terminal and special symbols to text or binary format.

In this example, we accept the output from the translation grammar above and output it in the same way the lexical analyzer reads them. In this example, any symbols `i` with the attribute `"null"` are considered a semantic error.

```c++
#include <ctf.h>

class ExampleGnr: public OutputGenerator {
  void generator(std::ostream &os, const Symbol &s) {
    if(s.name() == "i" && s.attribute() == "null") {
      throw SemanticError("Identifier with name 'PHP'.");
    }

    out << s.name();
    if (s.attribute() != "")
      out << '.' << s.attribute();
    out << '\n';
  }
}
```

## Creating the translation object

The `Translation` object is created with the lexical analyzer, translation grammar and output generator. The translation method must be selected by either providing an object of any subclass of `TranslationControl`, or preferably, using one of the implemented methods by passing a string identifying them.

Optionally, you can provide a callable that provides syntax error messages for each nonterminal-terminal pair. This feature is in the experimental stage and the API is subject to change.

In this example, we use the modules from the previous examples to create the `Translation` object.

```c++
#include <ctf.h>
#include <iostream>

extern TranslationGrammar grammar;

Token analyzer(std::istream &);
void generator(std::ostream &, const Symbol &);

int main() {
  Translation translation(analyzer, "ll", grammar, generator);

  try {
    //running translation from stdin to stdout
    translation.run(std::cin, std::cout);
  }
  catch (TranslationException &e) {
    std::cerr << "Something went wrong: " + e.what() + "\n";
	return 1;
  }

  return 0;
}
```
