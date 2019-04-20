#include "ctfgc.h"

using namespace ctfgc;
ctf::TranslationGrammar ctfgc::grammar({
    // S
    ctf::Rule("S"_nt,
      {"NEWLINE"_t, "S"_nt, },
      {"S"_nt, }
    ),
    ctf::Rule("S"_nt,
      {"grammar"_t, "grammar name"_t, "NEWLINE"_t, "Precedence"_nt, "Rules"_nt, },
      {"grammar"_t, "Precedence"_nt, "Rules"_nt, },
      ctf::vector<ctf::vector_set<size_t>>{{}, {0, }, {}, }
    ),

    // Precedence
    ctf::Rule("Precedence"_nt,
      {}
    ),
    ctf::Rule("Precedence"_nt,
      {"NEWLINE"_t, "Precedence"_nt, },
      {"Precedence"_nt, }
    ),
    ctf::Rule("Precedence"_nt,
      {"precedence"_t, ":"_t, "NEWLINE"_t, "INDENT"_t, "PrecedenceLevels"_nt, "DEDENT"_t, },
      {"precedence"_t, "PrecedenceLevels"_nt, "precedence end"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, {}, {}, {}, {}, }
    ),

    // PrecedenceLevels
    ctf::Rule("PrecedenceLevels"_nt,
      {}
    ),
    ctf::Rule("PrecedenceLevels"_nt,
      {"NEWLINE"_t, },
      {}
    ),
    ctf::Rule("PrecedenceLevels"_nt,
      {"Associativity"_nt, "TokenList"_nt, "NEWLINE"_t, "PrecedenceLevels"_nt, },
      {"Associativity"_nt, "TokenList"_nt, "level end"_t, "PrecedenceLevels"_nt, }
    ),

    // Associativity
    ctf::Rule("Associativity"_nt,
      {"none"_t, }
    ),
    ctf::Rule("Associativity"_nt,
      {"left"_t, }
    ),
    ctf::Rule("Associativity"_nt,
      {"right"_t, }
    ),

    // TokenList
    ctf::Rule("TokenList"_nt,
      {"terminal"_t, }
    ),
    ctf::Rule("TokenList"_nt,
      {"terminal"_t, ","_t, },
      {"terminal"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, {}, }
    ),
    ctf::Rule("TokenList"_nt,
      {"terminal"_t, ","_t, "TokenList"_nt, },
      {"terminal"_t, "TokenList"_nt, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, {}, }
    ),

    // Rules
    ctf::Rule("Rules"_nt,
      {"NEWLINE"_t, "Rules"_nt, },
      {"Rules"_nt, }
    ),
    ctf::Rule("Rules"_nt,
      {"Rule"_nt, }
    ),
    ctf::Rule("Rules"_nt,
      {"Rule"_nt, "Rules"_nt, }
    ),

    // Rule
    ctf::Rule("Rule"_nt,
      {"nonterminal"_t, ":"_t, "NEWLINE"_t, "INDENT"_t, "RuleClauses"_nt, "DEDENT"_t, },
      {"nonterminal"_t, "RuleClauses"_nt, "rule block end"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, {}, {}, {}, {2, }, }
    ),

    // RuleClauses
    ctf::Rule("RuleClauses"_nt,
      {}
    ),
    ctf::Rule("RuleClauses"_nt,
      {"NEWLINE"_t, "RuleClauses"_nt, },
      {"RuleClauses"_nt, }
    ),
    ctf::Rule("RuleClauses"_nt,
      {"SingleRule"_nt, "RuleClauses"_nt, },
      {"SingleRule"_nt, "rule end"_t, "RuleClauses"_nt, }
    ),

    // SingleRule
    ctf::Rule("SingleRule"_nt,
      {"String"_nt, "NEWLINE"_t, },
      {"String"_nt, }
    ),
    ctf::Rule("SingleRule"_nt,
      {"String"_nt, "NEWLINE"_t, "INDENT"_t, "AttributesLight"_nt, "DEDENT"_t, },
      {"String"_nt, "AttributesLight"_nt, }
    ),
    ctf::Rule("SingleRule"_nt,
      {"String"_nt, "|"_t, "OutputString"_nt, }
    ),

    // String
    ctf::Rule("String"_nt,
      {"-"_t, },
      {"string end"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, }
    ),
    ctf::Rule("String"_nt,
      {"terminal"_t, },
      {"terminal"_t, "string end"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, 1, }, }
    ),
    ctf::Rule("String"_nt,
      {"nonterminal"_t, },
      {"nonterminal"_t, "string end"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, 1, }, }
    ),
    ctf::Rule("String"_nt,
      {"nonterminal"_t, "String"_nt, }
    ),
    ctf::Rule("String"_nt,
      {"terminal"_t, "String"_nt, }
    ),

    // OutputString
    ctf::Rule("OutputString"_nt,
      {"NEWLINE"_t, "INDENT"_t, "String"_nt, "NEWLINE"_t, "DEDENT"_t, },
      {"String"_nt, }
    ),
    ctf::Rule("OutputString"_nt,
      {"NEWLINE"_t, "INDENT"_t, "String"_nt, "NEWLINE"_t, "Attributes"_nt, "DEDENT"_t, },
      {"String"_nt, "Attributes"_nt, }
    ),
    ctf::Rule("OutputString"_nt,
      {"String"_nt, "NEWLINE"_t, },
      {"String"_nt, }
    ),
    ctf::Rule("OutputString"_nt,
      {"String"_nt, "NEWLINE"_t, "INDENT"_t, "Attributes"_nt, "DEDENT"_t, },
      {"String"_nt, "Attributes"_nt, }
    ),

    // Attributes
    ctf::Rule("Attributes"_nt,
      {"RulePrecedence"_nt, },
      {"attributes"_t, "RulePrecedence"_nt, "attribute list end"_t, }
    ),
    ctf::Rule("Attributes"_nt,
      {"AttributeList"_nt, },
      {"attributes"_t, "AttributeList"_nt, "attribute list end"_t, }
    ),
    ctf::Rule("Attributes"_nt,
      {"RulePrecedence"_nt, "AttributeList"_nt, },
      {"attributes"_t, "RulePrecedence"_nt, "AttributeList"_nt, "attribute list end"_t, }
    ),

    // AttributesLight
    ctf::Rule("AttributesLight"_nt,
      {"RulePrecedence"_nt, },
      {"attributes"_t, "RulePrecedence"_nt, "attribute list end"_t, }
    ),

    // RulePrecedence
    ctf::Rule("RulePrecedence"_nt,
      {"precedence"_t, "terminal"_t, "NEWLINE"_t, },
      {"precedence"_t, "terminal"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, {1, }, {}, }
    ),

    // AttributeList
    ctf::Rule("AttributeList"_nt,
      {"Attribute"_nt, }
    ),
    ctf::Rule("AttributeList"_nt,
      {"Attribute"_nt, "AttributeList"_nt, }
    ),

    // Attribute
    ctf::Rule("Attribute"_nt,
      {"-"_t, "NEWLINE"_t, },
      {"attribute end"_t, }
    ),
    ctf::Rule("Attribute"_nt,
      {"IntList"_nt, "NEWLINE"_t, },
      {"IntList"_nt, "attribute end"_t, }
    ),

    // IntList
    ctf::Rule("IntList"_nt,
      {"integer"_t, }
    ),
    ctf::Rule("IntList"_nt,
      {"integer"_t, ","_t, },
      {"integer"_t, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, {}, }
    ),
    ctf::Rule("IntList"_nt,
      {"integer"_t, ","_t, "IntList"_nt, },
      {"integer"_t, "IntList"_nt, },
      ctf::vector<ctf::vector_set<size_t>>{{0, }, {}, }
    ),

  },
  "S"_nt,
  {
    ctf::PrecedenceSet{ctf::Associativity::NONE, {"NEWLINE"_t, }},
  }
);
