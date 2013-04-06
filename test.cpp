#include <iostream>

#include "lundi.hpp"

int plop_xyz(int x, int y, std::string z) {
  std::cout << x << " " << y << " " << z << std::endl;
  return 11;
}

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

  lua.eval("function my_add(i, j, k) return i + j + k end");
  std::cout << lua.call("my_add", 3, 6, 4) << std::endl;

  lua.register_function("plop_xyz", plop_xyz);
  lua.eval("x = plop_xyz(2, 6, \"hello\")");
  std::cout << lua.get_global("x") << std::endl;
}
