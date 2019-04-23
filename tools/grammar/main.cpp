#include <ctf.hpp>

#include "ctfgc.h"

#include <tclap/CmdLine.h>
#include <fstream>
#include <iostream>
#include <set>

using namespace ctfgc::literals;

class TGLex : public LexicalAnalyzer {
 public:
  using LexicalAnalyzer::LexicalAnalyzer;

  Token read_token() override {
    if (_buffered) {
      --_buffered;
      return _bufferedToken;
    }
  read_new:
    int c = get();
    switch (c) {
      case '|':
        return token("|"_t);
      case ':':
        return token(":"_t);
      case ',':
        return token(","_t);
      case '-':
        return token("-"_t);
      case '\'':
        return token_terminal();
      case ' ':
      case '\t':
        goto read_new;
      case '#':
        return comment();
      case '\n':
        return token_newline();
      case std::char_traits<char>::eof():
        return token_eof();
      default:
        break;
    }
    if (std::islower(c)) {
      return token_grammar_name(c);
    }
    if (std::isupper(c)) {
      return token_nonterminal(c);
    }
    if (std::isdigit(c)) {
      return token_integer(c);
    }
    fatal_error(string("unexpected character ") + (char)c);
    return Symbol::eof();
  }

 private:
  unsigned char _tabs = 0;
  unsigned char _buffered = 0;
  Token _bufferedToken = Symbol::eof();

  Token token_terminal() {
    string s;
    for (int c = get(); c != '\''; c = get()) {
      switch (c) {
        case '\\':
          switch (c = get(); c) {
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
              s += '\\';
              [[fallthrough]];
            case '\\':
            case '\'':
            case '"':
              s += '\\';
              s += (char)c;
              break;
            default:
              fatal_error(string("invalid escaped character ") + (char)c + " in terminal");
          }
          break;
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '"':
          fatal_error(
            "Forbidden formatting character in terminal.\n\\b, \\f, \\n, \\r, \\t and \" must be escaped.");
          break;
        case '\'':
          break;
        case std::char_traits<char>::eof():
          fatal_error("Read EOF while reading a terminal.");
          break;
        default:
          s += (char)c;
      }
    }
    if (s.empty()) {
      fatal_error("Empty terminal identifier.");
    }
    return token("terminal"_t, Attribute(s));
  }

  Token token_grammar_name(int c) {
    string s;
    char prev = '\0';
    do {
      s += (char)c;
      prev = c;
      c = get();
      if (c == '_' && prev == '_') {
        fatal_error("Consecutive '_' characters are forbidden in grammar name.");
      }
    } while (std::islower(c) || c == '_');
    unget();
    if (std::isalpha(c)) {
      fatal_error("Uppercase letters are forbidden in grammar name.");
    }

    // check for keywords
    if (s == "grammar")
      return token("grammar"_t);
    else if (s == "precedence") {
      return token("precedence"_t);
    } else if (s == "none") {
      return token("none"_t);
    } else if (s == "left") {
      return token("left"_t);
    } else if (s == "right") {
      return token("right"_t);
    }
    return token("grammar name"_t, Attribute(s));
  }

  Token token_nonterminal(int c) {
    string s;
    do {
      s += (char)c;
      c = get();
    } while (std::isalnum(c) || c == '\'');
    unget();

    return token("nonterminal"_t, Attribute(s));
  }

  Token token_integer(int c) {
    size_t number = 0;
    do {
      number = number * 10 + c - '0';
      c = get();
    } while (std::isdigit(c));
    unget();

    return token("integer"_t, Attribute(number));
  }

  Token token_newline() {
    Token nl = token("NEWLINE"_t);
    reset_location();
    size_t tabs = 0;
    int c;
    while ((c = get()) == '\t') {
      ++tabs;
    }
    unget();
    if (c == ' ')
      warning("Spaces are not allowed at the start of a new line.");
    if (tabs < _tabs) {
      _buffered = _tabs - tabs;
      _tabs = tabs;
      _bufferedToken = token("DEDENT"_t);
    } else if (tabs > _tabs) {
      _buffered = tabs - _tabs;
      _tabs = tabs;
      _bufferedToken = token("INDENT"_t);
    }

    return nl;
  }

  Token comment() {
    int c;
    do {
      c = get();
    } while (c != '\n' && c != std::char_traits<char>::eof());
    unget();
    reset_location();
    get();
    if (c == '\n') {
      return token_newline();
    }
    return token_eof();
  }

  void reset_private() override {
    _buffered = 0;
    _tabs = 0;
  }
};

// output generator:
// generate operators ""_nt, ""_t
// generate to_string function
// generate grammar

class TGOutput : public OutputGenerator {
 public:
  TGOutput(const std::string& outFolder) : OutputGenerator(), _outFolder(outFolder) {}

  virtual void output(const tstack<Token>& out) override {
    // first pass: get all terminals and nonterminals and map them to size_t
    // get all precedences
    auto it = out.begin();
    _grammarName = it->attribute().get<string>();
    ++it;
    // precedence
    build_precedence(it);
    // rules
    build_symbol_maps(it);
    std::stringstream hs;
    std::stringstream cpps;
    // output symbol mapping functions
    generate_header(hs);
    // second pass: construct rules and functions, ensure that rules are in the correct form
    generate_rules(it, cpps);
    // output if there were no errors
    if (!error()) {
      std::ofstream hfs(_outFolder + "/" + _grammarName + ".h");
      std::ofstream cppfs(_outFolder + "/" + _grammarName + ".cpp");
      if (hfs.fail()) {
        error(string("Could not open ") + _outFolder + "/" + _grammarName + ".h for writing");
        throw CodeGenerationException("Could not open output files.");
      }
      if (cppfs.fail()) {
        error(string("Could not open ") + _outFolder + "/" + _grammarName + ".cpp for writing");
        throw CodeGenerationException("Could not open output files.");
      }
      hfs << hs.str();
      cppfs << cpps.str();
    }
  }

 private:
  string _grammarName;
  string _outFolder;
  std::set<string> _nonterminals;
  std::set<string> _terminals;
  std::set<string> _outTerminals;
  map<string, size_t> _terminalMap;
  map<string, size_t> _nonterminalMap;
  vector<tuple<Associativity, vector<string>>> _precedences;

  virtual void reset_private() override {
    _grammarName.clear();
    _nonterminals.clear();
    _terminals.clear();
    _outTerminals.clear();
    _terminalMap.clear();
    _nonterminalMap.clear();
    _precedences.clear();
  }

  void build_precedence(tstack<Token>::const_iterator& it) {
    if (*it != "precedence"_t) {
      return;
    }
    // precedence rules
    ++it;
    while (*it != "precedence end"_t) {
      Associativity associativity = Associativity::NONE;
      vector<string> symbols;
      if (*it == "none"_t) {
        associativity = Associativity::NONE;
      } else if (*it == "left"_t) {
        associativity = Associativity::LEFT;
      } else if (*it == "right"_t) {
        associativity = Associativity::RIGHT;
      }
      ++it;
      while (*it != "level end"_t) {
        // *it == "terminal"_t
        const string& tid = it->attribute().get<string>();
        symbols.push_back(tid);
        _outTerminals.insert(tid);
        ++it;
      }
      _precedences.push_back({associativity, std::move(symbols)});
      ++it;
    }
    // move beyond precedence end
    ++it;
  }
  void build_symbol_maps(tstack<Token>::const_iterator it) {
    while (*it != Symbol::eof()) {
      // *it == "nonterminal"_t
      const string& nid = it->attribute().get<string>();
      _nonterminals.insert(nid);
      ++it;
      while (*it != "rule block end"_t) {
        while (*it != "rule end"_t) {
          while (*it != "string end"_t) {
            const string& id = it->attribute().get<string>();
            if (*it == "terminal"_t) {
              _terminals.insert(id);
            } else if (*it == "nonterminal"_t) {
              _nonterminals.insert(id);
            }
            ++it;
          }
          ++it;
          if (*it == "|"_t) {
            ++it;
            while (*it != "string end"_t) {
              if (*it == "terminal"_t) {
                const string& id = it->attribute().get<string>();
                _outTerminals.insert(id);
              }
              // nonterminals must match, no need to insert
              ++it;
            }
            // move beyond output string end
            ++it;
          }
          if (*it == "attributes"_t) {
            // process attributes
            ++it;
            if (*it == "precedence"_t) {
              ++it;
              // terminal
              const string& id = it->attribute().get<string>();
              _outTerminals.insert(id);
              ++it;
            }
            while (*it != "attribute list end"_t) {
              // skip attributes in the first pass
              ++it;
            }
            ++it;
          }
        }
        // move beyond rule end
        ++it;
      }
      // move beyond rule block end
      ++it;
    }
    // associate identifiers to size_t
    for (auto& id : _nonterminals) {
      size_t s = _nonterminalMap.size();
      _nonterminalMap[id] = s;
    }
    for (auto& id : _terminals) {
      size_t s = _terminalMap.size();
      _terminalMap[id] = s;
    }
    for (auto& id : _outTerminals) {
      size_t s = _terminalMap.size();
      _terminalMap.emplace(id, s);
    }
  }

  void generate_rules(tstack<Token>::const_iterator it, std::ostream& os) {
    os << "#include \"" << _grammarName
       << ".h\"\n\n"
          "using namespace "
       << _grammarName
       << ";\n"
          "ctf::TranslationGrammar "
       << _grammarName << "::grammar({\n";
    if (*it == Symbol::eof()) {
      fatal_error(it, "There must be at least one nonterminal in the grammar.");
    }
    string startingSymbol = it->attribute().get<string>();

    while (*it != Symbol::eof()) {
      string nt = it->attribute().get<string>();
      ++it;
      os << "    // " << nt << "\n";
      while (*it != "rule block end"_t) {
        generate_rule(nt, it, os);
      }
      os << "\n";
      ++it;
    }
    os << "  },\n  \"" << startingSymbol << "\"_nt";
    if (!_precedences.empty()) {
      os << ",\n  {";
      for (auto& [associativity, symbolVec] : _precedences) {
        os << "\n    ctf::PrecedenceSet{ctf::Associativity::";
        switch (associativity) {
          case Associativity::NONE:
            os << "NONE";
            break;
          case Associativity::LEFT:
            os << "LEFT";
            break;
          case Associativity::RIGHT:
            os << "RIGHT";
            break;
        }
        os << ", {";
        for (auto& id : symbolVec) {
          os << "\"" << id << "\"_t, ";
        }
        os << "}},";
      }
      os << "\n  }";
    }
    os << "\n);\n";
  }

  void generate_rule(string nt, tstack<Token>::const_iterator& it, std::ostream& os) {
    os << "    ctf::Rule(\"" << nt << "\"_nt,\n";
    auto start = it;
    // TODO good error messages
    while (*it != "rule end"_t) {
      vector<string> inputNonterminals;
      vector<string> outputNonterminals;
      vector<bool> outputTerminals;
      size_t inputTerminals = 0;
      size_t outputSymbols = 0;
      size_t printedAttributes = 0;
      bool customPrecedence = false;
      bool differentOut = false;

      string precedenceSymbol;
      os << "      {";
      while (*it != "string end"_t) {
        const string& id = it->attribute().get<string>();
        if (*it == "terminal"_t) {
          os << "\"" << id << "\"_t, ";
          ++inputTerminals;
        } else if (*it == "nonterminal"_t) {
          os << "\"" << id << "\"_nt, ";
          inputNonterminals.push_back(id);
        }
        ++it;
      }
      os << "}";
      ++it;
      if (*it == "|"_t) {
        differentOut = true;
        os << ",\n      {";
        ++it;
        while (*it != "string end"_t) {
          const string& id = it->attribute().get<string>();
          ++outputSymbols;
          if (*it == "terminal"_t) {
            os << "\"" << id << "\"_t, ";
            outputTerminals.push_back(true);
          } else if (*it == "nonterminal"_t) {
            os << "\"" << id << "\"_nt, ";
            outputNonterminals.push_back(id);
            outputTerminals.push_back(false);
          }
          ++it;
        }
        ++it;
        os << "}";
        // output string
        if (inputNonterminals != outputNonterminals) {
          string errorMessage = "Nonterminals don't match:\n";
          errorMessage += nt + " ->\n[";
          for (auto& i : inputNonterminals) {
            errorMessage += ' ';
            errorMessage += i;
          }
          errorMessage += " ]\n[";
          for (auto& i : outputNonterminals) {
            errorMessage += ' ';
            errorMessage += i;
          }
          errorMessage += " ]\n";
          error(start, errorMessage);
        }
      }
      // TODO: check that input and output nonterminals are the same
      if (*it == "attributes"_t) {
        // process attributes
        ++it;
        if (*it == "precedence"_t) {
          ++it;
          // terminal
          customPrecedence = true;
          precedenceSymbol = it->attribute().get<string>();
          ++it;
        }
        if (differentOut) {
          os << ",\n      ctf::vector<ctf::vector_set<size_t>>{";
          while (*it != "attribute list end"_t) {
            os << "{";
            while (*it != "attribute end"_t) {
              size_t target = it->attribute().get<size_t>() - 1;
              if (target >= outputTerminals.size() || !outputTerminals[target]) {
                string errorMessage = "Attribute target is not a terminal in rule derived from ";
                errorMessage += nt + ".";
                error(it, errorMessage);
              }
              os << target << ", ";
              ++it;
            }
            os << "}, ";
            ++printedAttributes;
            // process attributes
            ++it;
          }
          if (printedAttributes > inputTerminals) {
            string errorMessage = "Too many attributes in rule derived from ";
            errorMessage += nt + ": at most " + std::to_string(inputTerminals) +
                            " attribute targets may be specified.";
            error(----it, errorMessage);
            ++++it;
          }
          while (printedAttributes < inputTerminals) {
            os << "{}, ";
            ++printedAttributes;
          }
          os << "}";
        }
        if (customPrecedence) {
          os << ",\n      true, \"" << precedenceSymbol << "\"_t";
        }
        ++it;
      }
    }
    os << "\n    ),\n",
      // move beyond rule end
      ++it;
  }
  // generate header with declarations and name mapping functions
  void generate_header(std::ostream& os) {
    // include ctf
    os << "#ifndef CTFGRAMMAR_" << _grammarName
       << "_H\n"
          "#define CTFGRAMMAR_"
       << _grammarName
       << "_H\n\n"
          "#define CTF_NO_USING_NAMESPACE\n"
          "#include <ctf.hpp>\n"
          "#undef CTF_NO_USING_NAMESPACE\n\n"
          "namespace "
       << _grammarName
       << " {\n\n"
          "inline namespace literals {\n\n";
    // generate ""_nt
    os << "inline constexpr ctf::Symbol operator\"\"_nt(const char* s, size_t) {\n";
    for (auto& [key, value] : _nonterminalMap) {
      os << "  if (ctf::c_streq(s, \"" << key
         << "\"))\n"
            "    return ctf::Nonterminal("
         << value << ");\n";
    }
    os << "\n  return ctf::Nonterminal(" << _nonterminalMap.size() << ");\n}\n\n";
    // generate ""_t
    os << "inline constexpr ctf::Symbol operator\"\"_t(const char* s, size_t) {\n";
    for (auto& [key, value] : _terminalMap) {
      os << "  if (ctf::c_streq(s, \"" << key
         << "\"))\n"
            "    return ctf::Terminal("
         << value << ");\n";
    }
    os << "\n  return ctf::Terminal(" << _terminalMap.size() << ");\n}\n\n";
    // generate to_string
    os << "}\n\n";

    os << "inline ctf::string to_string(ctf::Symbol s) {\n"
          "  using namespace ctf::literals;\n"
          "  static ctf::map<ctf::Symbol, ctf::string> names = {\n";
    for (auto& [key, value] : _terminalMap) {
      os << "    {ctf::Terminal(" << value << "), \"'" << key << "'\"},\n";
    }
    for (auto& [key, value] : _nonterminalMap) {
      os << "    {ctf::Nonterminal(" << value << "), \"" << key << "\"},\n";
    }
    os << "  };\n";
    os << "  auto it = names.find(s);\n"
          "  if (it != names.end()) {\n"
          "    return it->second;\n"
          "  }\n"
          "  return s.to_string();\n"
          "}\n\n"
          "extern ctf::TranslationGrammar grammar;\n\n";

    os << "}\n#endif\n";
  }
};

// TODO file or stdin input
// todo which arguments

// ./ctfgc [-i] [input/stdin] [-o] [output folder/.]
int main(int argc, char** argv) try {
  TCLAP::CmdLine cmd("ctfgc: translate translation grammar .ctfg files to C++", ' ', "1.0");
  TCLAP::UnlabeledValueArg<std::string> inputArg("input", "input file", true, "", "input file");
  TCLAP::ValueArg<std::string> outputArg("o", "output", "output folder", false, ".", "output file");
  cmd.add(inputArg);
  cmd.add(outputArg);
  cmd.parse(argc, argv);
  std::string outputFolder = outputArg.getValue();
  std::string input = inputArg.getValue();

  std::istream* i;
  std::ifstream file;
  if (input == "") {
    i = &std::cin;
    input = "stdin";
  } else {
    file.open(input);
    if (!file) {
      std::cerr << "Error: Could not open " << input << ".\n";
      return 1;
    }
    i = &file;
  }
  // run translation
  Translation t(TGLex(), ctfgc::grammar, TGOutput(outputFolder), ctfgc::to_string);
  auto result = t.run(*i, std::cout, std::cerr, input);
  switch (result) {
    case TranslationResult::SUCCESS:
      return 0;
    case TranslationResult::LEXICAL_ERROR:
      return 2;
    case TranslationResult::TRANSLATION_ERROR:
      return 3;
    case TranslationResult::SEMANTIC_ERROR:
    case TranslationResult::CODE_GENERATION_ERROR:
      return 4;
  }
} catch (TCLAP::ArgException& e) {
  std::cerr << "error: " << e.error() << " for argument " << e.argId() << "\n";
  return 1;
}
