#include <iostream>

#include "lundi.hpp"

int main() {
  lua::state lua;

  lua.eval("a = 9");
  std::cout << lua.get_global("a") << std::endl;

  lua.set_global("b", 2.);
  lua.eval("c = a + b");
  std::cout << lua.get_global("c") << std::endl;

  lua.set_global("d", "hello");
  std::cout << lua.get_global("d") << std::endl;

  lua.set_global("e", true);
  std::cout << lua.get_global("e") << std::endl;

  std::string program = "if e then f = a else f = d end";
  lua.eval(program);
  std::cout << lua.get_global("f") << std::endl;

  lua.set_global("e", false);
  lua.eval(program);
  std::cout << lua.get_global("f") << std::endl;
}
