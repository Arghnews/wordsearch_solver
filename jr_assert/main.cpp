#include <string>
#include <fmt/format.h>
#include <jr_assert/jr_assert.h>
#include <utility>

int main()
{
  std::string s{"A string"};
  auto f = []() { std::cout << "hi\n"; return 42; };
  JR_ASSERT(f() == 42);
  JR_ASSERT(1 + 1 == 2, "Should not fire: {}, {}", s, f());
  JR_ASSERT(1 + 1 != 2, "Fires: {}, {}", s, f());
  auto [a, b] = std::pair<int, int>{3, 4};
  JR_ASSERT(a == 3);
}
