![ctf logo](media/ctf-logo-medium.png)

# CTF ain't a Translation Framework

CTF is a C++17 framework for rapid translation definition. Translations are defined with lexical analyzers, translation grammars and output generators.

## Requirements
CTF requires a C++-17 compilant compiler.
The framework was tested on `g++-7.4.0` and `clang++-6.0.0`.

To run tests, run `make test` from the project's root directory.

## Including CTF in other projects.
CTF source is a single-header library.
To start using CTF in a source file, add the `ctf/include` folder to your include paths and simply insert
```
#include <ctf.hpp>
```
to your source files.

The header includes `using namespace ctf`. If you wish to opt-out of this, simply define the symbol `CTF_NO_USING_NAMESPACE` before including the header.

We recommend including the `ctf.hpp` header in few source files. Including it in many unrelated source files will unfortunately result
in high compile times.

We provide two tools `grammarc` and `parsergen` in the `tools/` folder. To compile `grammarc`, run `make` in the root directory of ctf.

## Basic usage

```
#include <iostream>
#include <ctf.hpp>

#include "mygrammar.h"

class Lex : public LexicalAnalyzer {
	// ...
};
class Out : public OutputGenerator {
	// ...
};

int main() {
	// construct a translation from a grammar, uses LSCELR
	Translation t1(Lex{}, mygrammar::grammar, Out{}, mygrammar::to_string);
	// select algorithm (LALR, LSCELR or CanonicalLR1)
	Translation t2(Lex{}, LALR{}, mygrammar::grammar, Out{}, mygrammar::to_string);
	// load saved tables
	Translation t3(Lex{}, load(std::string("filename")), Out{}, mygrammar::to_string);

	/* ctf::TranslationResult::{SUCCESS,
	                            LEXICAL_ERROR,
								TRANSLATION_ERROR,
								SEMANTIC_ERROR,
								CODE_GENERATION_ERROR}
	*/
	auto result = t1.run(std::cin, std::cout, std::cerr, "std::cin");

	return 0;
}
```

## Translation Grammars
CTF uses attribute translation grammars with precedence and associativity to define translation.
The recommended way of specifying these grammars is with our text representation.
To translate this representation to CTF source files, use `tools/grammarc`.

The format of the description is shown in the following example:
```
grammar grammar_name

# Comments look like this.
# Grammar name is in snake_case and is always the first thing in the file.
# The name determines the output file names and the C++ namespace defined in these files.
# For the full grammar, see tools/grammar/grammar.ctfg

# optional
precedence:
	left 'a' 'ab' 'some terminal name' # highest precedence
	right 'b' # tab indentation
	none 'x' # lowest precedence

StartingNonterminal: # nonterminals are in CamelCase
	'first production input' A | A 'first production output'
	'second production input and output'
	- | 'some output terminal' # reduce to nothing

A:
	A '.' ':' A | A A '.' ':' '!'
		precedence 'x' # precedence of a different symbol than ':' (the last terminal on input)
		3 # transfer the attribute of the first terminal to position 3 in the output
		4, 5 # transfer the attribute of the second input terminal to both last output symbols

```

## Lexical Analyzers
For implementing lexical analyzers, we recommend using `ctf::LexicalAnalyzer` as a base class to comply with the required interface.
We will list the virtual methods you should override for your lexical analyzers.

* `ctf::Token read_token()`: This method performs the lexical analyzer's function. This function will be called every time a token is required.
* `void reset_private()`: This method is called before each invokation of the prepared translation. If you plan on using the same `Translation` object multiple times, you should implement resetting your lexical analyzer's custom attributes.

### Implementing read_token()
This is a list of protected methods of `ctf::LexicalAnalyzer` that will help you implement your lexical analyzers.
By using these methods, you will gain automatic tracking of position in the source file, which will be embedded in the returned tokens.

* `int get()`: This method will extract a single character from input. This either contains a `char` value of the read byte, or the special `std::char_traits<char>::eof()` value.
* `int get(std::function<bool(int)> accept)`: This method will extract the first character from the input that satisfies the `accept` predicate (or EOF if it comes first).
* `void unget(std::size_t num = 1)`: Unget n characters.
* `void reset_location()`: The base class always keeps the location of the first read symbol. To reset that value, call this method.
* `ctf::Token token(ctf::Symbol s, const ctf::Attribute& attr = Attribute{})`: This method will construct the `Token` object to return from `read_token()`. It adds the location info and the optional attribute.
* `ctf::Token token(std::size_t id, const ctf::Attribute& attr = Attribute{})`: This constructs the token directly with a specific id. We recommend using the previous overload of this function in conjunction with the literal functions generated by `grammarc` (see section Compiling Grammars).
* `ctf::Token token_eof()`: This method returns the special EOF token with the appropriate location.
* `void set_error()`: Sets the error flag. If we encounter an error but have no error message, we can indicate this by calling this method.
* `void warning(const std::string& message)`: Prints a warning message.
* `void error(const std::string& message)`: Prints an error message and sets the error flag.
* `void fatal_error(const std::string& message)`: Prints an error message and sets the error flag. Throws an exception that is caught by the `Translation` object.
* `std::ostream& err()`: Returns the error stream for additional custom error messages.

## Output Generators
For implementing output generators, we recommend using `ctf::OutputGenerator` as a base class to comply with the required interface.

* `void output(const ctf::tstack<ctf::Token>& tokens)`: This method performs the output generator's function. You obtain a string of output tokens and produce the text output to the designated output stream.
* `void reset_private()`: This method is called before each invokation of the prepared translation. If you plan on using the same `Translation` object multiple times, you should implement resetting your output generator's custom attributes.

### Implementing output()
This is a list of protected methods of `ctf::OutputGenerator` that will help you implement your output generators.

* `std::ostream& os()`: Get the output stream of the output generator. This is a buffer for the output stream set by the translation call. Your custom output generators may use different streams for their output.
* `std::ostream& err()`: Get the error stream set by the translation call.
* `void set_error()`: Sets the error flag. If we encounter an error but have no error message, we can indicate this by calling this method.
* `void warning(const std::string& message)`: Prints a warning message.
* `void error(const std::string& message)`: Prints an error message and sets the error flag.
* `void fatal_error(const std::string& message)`: Prints an error message and sets the error flag. Throws an exception that is caught by the `Translation` object.
* Each of the previous three methods provides an overload with an additional `ctf::tstack<Token>::const_iterator` parameter. These overloads additionally print the location of the token behind the iterator before the error or warning message.
