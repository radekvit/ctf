#include <catch.hpp>

#include <list>
#include <vector>

#include "../src/ctf_base.hpp"
#include "../src/ctf_generic_types.hpp"

using ctf::tstack;
using ctf::vector_set;
using ctf::bit_set;

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
  tstack<std::string> stack2;

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

    // interface testing; emplace push
    stack2.push(156, 'x');
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
    auto it = stack.search('c', [](auto& lhs, auto& rhs) { return lhs > rhs; });
    REQUIRE(*it == 'd');
  }
}

TEST_CASE("tstack replace", "[tstack]") {
  tstack<int> stack{-1, 5, 0, 9, 9};
  tstack<int>::ReplaceResult expected;

  SECTION("iterator is end()") {
    expected = {stack.end(), stack.end()};
    REQUIRE(stack.replace(stack.end(), vector<int>{}) == expected);
    REQUIRE(stack.size() == 5);
  }

  SECTION("insert empty string") {
    auto it = stack.begin();
    ++it;
    ++it;
    expected = {it, it};
    REQUIRE(stack.replace(++stack.begin(), vector<int>{}) == expected);
    REQUIRE(stack.size() == 4);
    REQUIRE(stack.top() == -1);
  }

  SECTION("insert multiple element string") {
    auto it = stack.begin();
    ++it;
    expected.end = it;
    auto result = stack.replace(stack.begin(), vector<int>{1, 2, 3});
    expected.begin = stack.begin();
    REQUIRE(result == expected);
    REQUIRE(stack.size() == 7);
    REQUIRE(*stack.begin() == 1);
  }

  SECTION("searched replace") {
    auto it = stack.replace(9, std::list<int>{10, 11});
    REQUIRE(*it == 10);
    ++(++it);
    REQUIRE(*it == 9);
    REQUIRE(stack.size() == 6);
  }

  SECTION("searched replace from point") {
    auto it = stack.end();
    --it;
    it = stack.replace(9, std::vector<int>{11}, it);
    ++it;
    REQUIRE(it == stack.end());
  }

  SECTION("searched replace last") {
    auto it = stack.replace_last(9, std::vector<int>{10, 11});
    REQUIRE(it == stack.end());
    --it;
    REQUIRE(*it == 11);
    REQUIRE(stack.size() == 6);
  }

  SECTION("searched replace last from point") {
    auto it = stack.end();
    --it;
    --it;
    it = stack.replace_last(9, std::vector<int>{10, 11}, it);
    REQUIRE(it != stack.end());
    ++it;
    REQUIRE(it == stack.end());
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

TEST_CASE("bit_set basic operations", "[bit_set]") {
  bit_set s(6);
  const bit_set& sr = s;

  REQUIRE(s.empty());
  REQUIRE(s.none());
  REQUIRE(!s.any());
  REQUIRE(!s.all());
  REQUIRE_THROWS_AS(s.test(6), std::out_of_range);
  REQUIRE_NOTHROW(!s.test(5));
  REQUIRE_NOTHROW(!s.test(0));
  REQUIRE(!s.test(5));
  REQUIRE(!s.test(4));
  REQUIRE(!s.test(0));
  auto ref = s[4];
  REQUIRE(!ref);
  REQUIRE(~ref);
  ref = true;
  REQUIRE(static_cast<bool>(ref));
  REQUIRE(!~ref);
  REQUIRE(!s.empty());
  REQUIRE(!s.none());
  REQUIRE(s.any());
  REQUIRE(!s.all());
  REQUIRE(!s.test(5));
  REQUIRE(s.test(4));
  REQUIRE(!s.test(0));
  ref = false;
  REQUIRE(s.empty());
  REQUIRE(s.none());
  REQUIRE(!s.any());
  REQUIRE(!s.all());
  REQUIRE(!s.test(5));
  REQUIRE(!s.test(4));
  REQUIRE(!s.test(0));

  bit_set s1(128);

}