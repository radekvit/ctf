#ifndef CTF_OUTPUT_UTILITIES_HPP
#define CTF_OUTPUT_UTILITIES_HPP

#include "ctf_generic_types.hpp"

namespace ctf::output {

namespace color {
inline const string black = "\033[30m";
inline const string red = "\033[31m";
inline const string green = "\033[32m";
inline const string yellow = "\033[33m";
inline const string blue = "\033[34m";
inline const string magenta = "\033[35m";
inline const string cyan = "\033[36m";
inline const string white = "\033[37m";
}  // namespace color

namespace background {
inline const string black = "\033[40m";
inline const string red = "\033[41m";
inline const string green = "\033[42m";
inline const string yellow = "\033[43m";
inline const string blue = "\033[44m";
inline const string magenta = "\033[45m";
inline const string cyan = "\033[46m";
inline const string white = "\033[47m";
}  // namespace background

namespace style {
inline const string bold = "\033[1m";
inline const string boldOff = "\033[21m";

inline const string underline = "\033[4m";
inline const string underlineOff = "\033[24m";

inline const string inverse = "\033[7m";
inline const string inverseOff = "\033[27m";
}  // namespace style

inline string reset = "\033[0m";

}  // namespace ctf::output

#endif