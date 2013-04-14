Lundi
=====

[![Build Status](https://travis-ci.org/lundiorg/lundi.png?branch=master)](https://travis-ci.org/lundiorg/lundi)

###Lundi is easy-to-use, modern and straightforward API for integrating Lua in the C++ code.

[Lua programming language](http://lua.org) is a small and embeddable language written in pure ANSI C. As such, it doesn't provide an API that's common enough for C++ developers and has a lot of restrictions.
Lua C API operates on the *stack* concept to exchange data between the host and the embedded applications. It allows users to register C functions to Lua, but there have to meet certain requirements; namely they must take only one parameter, a pointer to `lua_State`, and manually take all the parameters from the stack and then put the results back.

There exist many C++ - Lua libraries, most known being probably OOLua and SWIG. While being quite nice pieces of code, they take approach of exposing whole C++ classes for Lua, so that C++ developer in theory can write code that's no different from normal C++.

With Lundi, we made sure that the design intentions of Lua are preserved. Rather than wrapping your code in ugly macros, it provides simple and consistent interface of interacting with virtual machine on level of basic operations. Whilst not everything works yet, you can already get a glimpse of how Lundi is intended to be used.

###Short, motivating examples

Let's take a look at simple variable getting/setting:

```cpp
// C version
lua_State * lua = luaL_newstate();
lua_getglobal(lua, "a");
int a = lua_tonumber(lua, -1);

// Lundi version
lua::state lua;
auto a = lua["a"];
```

In C version, numerous problems can appear. "a" might be nonexistent and of different type. You have to (obviously) pass a pointer to `lua_State` to every call. And then you have to ensure that `-1` is a correct stack index (the stack might overflow). On the other hand, C++ call is quite safe. It takes care of all these things for you, and if anything really bad happens, an exception will be trown, so you can easily take care of that.

Perhaps that wasn't the best example yet. Let's look at the functions. As I said earlier, you can bind C++ functions using `lua_CFunction` type. That's however far from convenient:

```cpp
void plop_xyz (bool x, double y, std::string const& z) {
    // do something with x,y,z
}

int lua_plop_xyz (lua_State *lua) {
    bool x (lua_toboolean(lua, -1));
    lua_pop(lua);
    double y (lua_tonumber(lua, -1));
    lua_pop(lua);
    std::string z (lua_tostring(lua, -1));
    lua_pop(lua);
    
    plop_xyz(x,y,z);
    
    return 0;
}

lua_pushcfunction(lua_plop_xyz);
lua_setglobal("foo");
```

That obviously lacks error checking and type checking, so the proper version would be even longer. Now, let's compare it to Lundi version:
```cpp
lua::state lua;
lua.register_function("foo", plop_xyz);
```

Yeah, that's pretty much magic. It parses the `plop_xyz` signature and generates the calls automatically. As an additional sparkle, it works with lambdas!
```cpp
lua.register_function("foo", [](bool x, double y, std::string const& z) { /* stuff */ });
```

Calling Lua functions from C++ is also easy:
```cpp
lua["luafoo"](true, 3.14, "hello");
```

The call is obviously variadic, so pass as many weird values as you might want to.

###Insides

Lundi uses C++11 Template Metaprogramming techniques and Boost libraries (most notably Fusion) heavily. Such techniques were required to make generating complicated structure of C calls on demand. Because of that, you will need quite modern C++ compiler to take use of it. We compiled it successfully under gcc-4.7 and 4.8, clang 3.2, Intel C++ Compiler 13.1, and, right now MSVC 2012 November CTP, althought the last one is subject to change.

###The name

*Lundi*, commonly known as "Monday" in French, has much older etymology. It used to mean "day of the moon"; that will probably make more sense if you check what Lua exactly means - it so happens it's "moon" in Portugeese. That way, we present you "The Day of Lua", as we believe that Lua is indeed a great language of the future, and combined with C++ will allow you to create applications and games that are safe, fast, flexible and extensible.
