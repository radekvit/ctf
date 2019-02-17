#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include "../include/ctf.hpp"

std::ostream& operator<<(std::ostream& os, const Symbol& s) { return os << s.to_string(); }

std::ostream& operator<<(std::ostream& os, const Location& l) { return os << l.to_string(); }
