#ifndef XVITRA00_BASE_H
#define XVITRA00_BASE_H

#include <generic_types.h>
#include <ostream>

namespace bp {

class TranslationException {
protected:
    string msg_;

public:
    TranslationException(string msg) : msg_(msg) {}
    string what() { return msg_; }
};

/**
\brief Terminal representation with name and optional attribute.

Empty name denotes EOF terminal.
 */
class Terminal {
protected:
    string name_;
    string attribute_;

public:
    /**
    \brief Default constructor with empty name and attributes.
    */
    Terminal(const string &name = "", const string &atr = "")
        : name_(name), attribute_(atr)
    {
    }

    static Terminal EOI() { return Terminal(); }

    string &name() { return name_; }
    const string &name() const { return name_; }
    string &attribute() { return attribute_; }
    const string &attribute() const { return attribute_; }

    friend bool operator<(const Terminal &lhs, const Terminal &rhs)
    {
        return lhs.name() < rhs.name();
    }

    friend bool operator==(const Terminal &lhs, const Terminal &rhs)
    {
        return lhs.name() == rhs.name();
    }
};

class Nonterminal {
protected:
    string name_;

public:
    Nonterminal(const string &name = "") : name_(name) {}

    string &name() { return name_; }
    const string &name() const { return name_; }

    friend bool operator<(const Nonterminal &lhs, const Nonterminal &rhs)
    {
        return lhs.name() < rhs.name();
    }

    friend bool operator==(const Nonterminal &lhs, const Nonterminal &rhs)
    {
        return lhs.name() == rhs.name();
    }
};

struct Symbol {
    enum class Type {
        TERMINAL,
        NONTERMINAL,
    } type;

    Terminal terminal;
    Nonterminal nonterminal;
    Symbol() = default;
    Symbol(Type _type) : type(_type) {}
    Symbol(Terminal _terminal) : type(Type::TERMINAL), terminal(_terminal) {}
    Symbol(Nonterminal _nonterminal)
        : type(Type::NONTERMINAL), nonterminal(_nonterminal)
    {
    }
    ~Symbol() = default;

    static Symbol EOI()
    { // end of input
        return Symbol(Terminal::EOI());
    }

    friend bool operator<(const Symbol &lhs, const Symbol &rhs)
    {
        if (lhs.type != rhs.type)
            return false;
        switch (lhs.type) {
        case Symbol::Type::TERMINAL:
            return lhs.terminal < rhs.terminal;
        case Symbol::Type::NONTERMINAL:
            return lhs.nonterminal < rhs.nonterminal;
        default:
            return false;
        }
    }

    friend bool operator==(const Symbol &lhs, const Symbol &rhs)
    {
        if (lhs.type != rhs.type)
            return false;
        switch (lhs.type) {
        case Symbol::Type::TERMINAL:
            return lhs.terminal == rhs.terminal;
        case Symbol::Type::NONTERMINAL:
            return lhs.nonterminal == rhs.nonterminal;
        default:
            return true;
        }
    }

    friend bool operator!=(const Symbol &lhs, const Symbol &rhs)
    {
        return !(lhs == rhs);
    }
};

}

#endif