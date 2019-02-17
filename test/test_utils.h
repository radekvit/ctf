#ifndef CTF_TEST_UTILS_H
#define CTF_TEST_UTILS_H

inline constexpr int c_streq(const char* a, const char* b) {
	while (*a != '\0' && *b != '\0') {
		if (*a != *b) return false;
		++a;
		++b;
	}
	return *a == *b;
}

#endif