#include "../src/generic_types.hpp"
#include <catch.hpp>
#include "../src/base.hpp"

#include <list>
#include <vector>

using ctf::tstack;

using std::vector;
using std::list;

// targeting prime path coverage
TEST_CASE("tstack construction", "[tstack]") {
  tstack<int> empty{};
  REQUIRE(empty.size() == 0);

  tstack<char> chars{'a', 'b', 'c'};
  REQUIRE(chars.size() == 3);
  REQUIRE(chars.top() == 'a');
}

TEST_CASE("tstack basic", "[tstack]") {
  tstack<char> stack{'a', 'b', 'c', 'd', 'e'};

  SECTION("Push, pop, top, empty.") {
    REQUIRE(stack.empty() == false);
    stack.push('x');
    REQUIRE(stack.top() == 'x');
    REQUIRE(stack.size() == 6);
    stack.push('y');
    REQUIRE(stack.size() == 7);
    REQUIRE(stack.pop() == 'y');
    REQUIRE(stack.pop() == 'x');
    REQUIRE(stack.top() == 'a');
    REQUIRE(stack.pop() == 'a');
    REQUIRE(stack.pop() == 'b');
    REQUIRE(stack.pop() == 'c');
    REQUIRE(stack.pop() == 'd');
    REQUIRE(stack.size() == 1);
    REQUIRE(stack.pop() == 'e');
    REQUIRE(stack.empty() == true);
  }

  SECTION("Clearing stack makes it empty") {
    stack.clear();
    REQUIRE(stack.empty() == true);
    REQUIRE(stack.size() == 0);
  }
}

TEST_CASE("tstack search", "[tstack]") {
  tstack<char> stack{'a', 'b', 'c', 'a', 'd'};

  SECTION("searching empty stack") {
    stack.clear();

    REQUIRE(stack.search('a') == stack.end());
  }

  SECTION("searching for element on top") {
    REQUIRE(stack.search('a') == stack.begin());
    REQUIRE(stack.size() == 5);
  }

  SECTION("searching for second element") {
    auto it = stack.search('b');
    REQUIRE(*it == 'b');
  }

  SECTION("custom predicate") {
    auto it = stack.search('c', [](auto &lhs, auto &rhs) { return lhs > rhs; });
    REQUIRE(*it == 'd');
  }
}

TEST_CASE("tstack replace", "[tstack]") {
  tstack<int> stack{-1, 5, 0, 9};

  SECTION("iterator is end()") {
    REQUIRE(stack.replace(stack.end(), vector<int>{}) == stack.end());
    REQUIRE(stack.size() == 4);
  }

  SECTION("insert empty string") {
    auto it = stack.begin();
    ++it;
    ++it;
    REQUIRE(stack.replace(++stack.begin(), vector<int>{}) == it);
    REQUIRE(stack.size() == 3);
    REQUIRE(stack.top() == -1);
  }

  SECTION("insert multiple element string") {
    auto it = stack.replace(stack.begin(), vector<int>{1, 2, 3});
    REQUIRE(it == stack.begin());
    REQUIRE(stack.size() == 6);
    REQUIRE(*stack.begin() == 1);
  }

  SECTION("searched replace") {
    auto it = stack.replace(9, std::list<int>{10, 11});
    REQUIRE(*it == 10);
    REQUIRE(stack.size() == 5);
  }
}

TEST_CASE("tstack swap", "[tstack]") {
  tstack<char> stack1{};
  tstack<char> stack2{'x', 't'};
  stack1.swap(stack2);

  REQUIRE(stack1.size() == 2);
  REQUIRE(stack2.size() == 0);
  REQUIRE(stack1.top() == 'x');
}

TEST_CASE("make_set") {
  vector<int> vec{4, 3, 4, 2, 5, 1, 3};

  ctf::make_set(vec);

  REQUIRE(vec.size() == 5);
  int last = -1;
  for (int i : vec) {
    REQUIRE(i > last);
    last = i;
  }
}

TEST_CASE("is_in") {
  vector<int> vec{1, 2, 5, 6, 10};

  REQUIRE(ctf::is_in(vec, 6) == true);
  REQUIRE(ctf::is_in(vec, 3) == false);
}

TEST_CASE("set_union") {
  vector<int> vec1{1, 2, 5, 6};
  vector<int> vec2{2, 3, 4, 6};

  vector<int> result = ctf::set_union(vec1, vec2);

  REQUIRE(result.size() == 6);
  int last = -1;
  for (int i : result) {
    REQUIRE(i > last);
    last = i;
  }
}

TEST_CASE("modify set") {
  vector<int> vec1{1, 2, 5, 6};
  vector<int> vec2{2, 3, 4, 6};
  vector<int> vec3{1};

  vector<int> uni = ctf::set_union(vec1, vec2);

  REQUIRE(ctf::modify_set(vec1, vec3) == false);
  REQUIRE(vec1.size() == 4);

  REQUIRE(ctf::modify_set(vec1, vec2) == true);
  REQUIRE(vec1.size() == 6);
  REQUIRE(vec2.size() == 4);

  REQUIRE(vec1 == uni);
}

TEST_CASE("reverse") {
  using ctf::reverse;

  list<int> container{1, 8, -1};
  const vector<int> const_container{0, 5, 99};

  REQUIRE(reverse(container).begin() == container.rbegin());
  REQUIRE(reverse(container).end() == container.rend());
  REQUIRE(reverse(container).rbegin() == container.begin());
  REQUIRE(reverse(container).rend() == container.end());

  REQUIRE(reverse(const_container).begin() == const_container.rbegin());
  REQUIRE(reverse(const_container).end() == const_container.rend());
  REQUIRE(reverse(const_container).rbegin() == const_container.begin());
  REQUIRE(reverse(const_container).rend() == const_container.end());
}

TEST_CASE("transform") {
  list<int> l{1, 5, 6, 9};

  vector<int> v{ctf::transform<list<int>, vector<int>>(l)};

  REQUIRE(v.size() == l.size());
  auto vit = v.begin();
  auto lit = l.begin();
  while (vit != v.end()) {
    REQUIRE(*vit == *lit);
    ++vit;
    ++lit;
  }
}