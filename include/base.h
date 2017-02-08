#ifndef XVITRA00_BASE_H
#define XVITRA00_BASE_H

#include <generic_types.h>
#include <ostream>

namespace bp {

class TranslationException
{
    string msg_;
public:
    TranslationException(string msg):
        msg_(msg)
    {
    }
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
    Terminal() = default;
    Terminal(const Terminal &) = default;
    Terminal(Terminal &&) = default;
    Terminal(const string &name) : name_(name) {}
    Terminal(const string &name, const string &str)
        : name_(name), attribute_(str)
    {
    }
    ~Terminal() = default;

    const string &name() const { return name_; }

    friend bool operator<(const Terminal &lhs,
                          const Terminal &rhs)
    {
        return lhs.name() < rhs.name();
    }

    friend bool operator==(const Terminal &lhs,
                           const Terminal &rhs)
    {
        return lhs.name() == rhs.name();
    }
};

class Nonterminal {
protected:
    string name_;

public:
    Nonterminal() = default;
    Nonterminal(const Nonterminal &) = default;
    Nonterminal(Nonterminal &&) = default;
    Nonterminal(const string &name) : name_(name) {}
    ~Nonterminal() = default;

    const string &name() const { return name_; }

    friend bool operator<(const Nonterminal &lhs,
                          const Nonterminal &rhs)
    {
        return lhs.name() < rhs.name();
    }

    friend bool operator==(const Nonterminal &lhs,
                           const Nonterminal &rhs)
    {
        return lhs.name() == rhs.name();
    }
};

struct Symbol {
    enum class Type {
        TERMINAL,
        NONTERMINAL,
        EOI,
    } type;

    Terminal terminal;
    Nonterminal nonterminal;
    Symbol() = delete;
    Symbol(Type _type) : type(_type) {}
    Symbol(Terminal _terminal) : type(Type::TERMINAL), terminal(_terminal)
    {
    }
    Symbol(Nonterminal _nonterminal)
        : type(Type::NONTERMINAL), nonterminal(_nonterminal)
    {
    }
    ~Symbol() = default;

    static Symbol EOI() { //end of input
        return Symbol(Type::EOI);
    }

    void print(std::ostream &o) const
    {
        switch (type) {
        case Type::TERMINAL:
            o << terminal.name();
            return;
        case Type::NONTERMINAL:
            o << nonterminal.name();
            return;
        case Type::EOI:
            o << "\\$";
            return;
        default:
            return;
        }
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