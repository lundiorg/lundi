
#include <iostream>
#include <string>
#include <exception>

#include "lundi.hpp"

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

namespace {

int plop_xyz(int x, int y, std::string z) {
    std::cout << x << " " << y << " " << z << std::endl;
    return 11;
}

// equivalent of Boost.Variant operator==
template<typename V, typename T>
inline bool equals(V const& variant, T const& value){
    return boost::get<T>(variant) && boost::get<T>(variant) == value; 
}

//[](std::string const& s) { std::cerr << "Lua ERROR : " << s << std::endl; }
void defaultErrorReporter(std::string const& s) {
    std::cerr << "Lua ERROR : " << s << std::endl; 
}

void exceptionErrorReporter(std::string const& s) {
    throw lua::exception(s);
}

}

TEST_CASE( "simple/set_global", "Check if the set_global works properly." ) {
    lua::state lua(&exceptionErrorReporter);

    lua.set_global("a", 9);
    REQUIRE_NOTHROW( lua.eval("if a ~= 9 then error('wrong value') end") );

    lua.set_global("d", "hello");
    REQUIRE_NOTHROW( lua.eval("if d ~= 'hello' then error('expected \\'hello\\', got '.. tostring(d)) end") );

    lua.set_global("e", std::string("hello"));
    REQUIRE_NOTHROW( lua.eval("if d ~= 'hello' then error('expected \\'hello\\', got '.. tostring(d)) end") );

    lua.set_global("f", true);
    REQUIRE_NOTHROW( lua.eval("if f ~= true then error('wrong value') end") );
}

TEST_CASE( "simple/get_global", "Tests if the get_global function works properly." ) {
    lua::state lua(&defaultErrorReporter);

    lua.eval("a = 9");
    lua::variant a = lua.get_global("a");
    REQUIRE( a == lua::variant(9.0) );

    lua.eval("b = nil");
    lua::variant b = lua.get_global("b");
    REQUIRE( b == lua::variant(lua::nil()) );

    lua.eval("d = 'hello'");
    lua::variant d = lua.get_global("d");
    REQUIRE( d == lua::variant("hello") );

    lua.eval("e = true");
    auto e = lua.get_global("e");
    REQUIRE( e == lua::variant(true) );
}

TEST_CASE( "simple/addition", "" ) {
    lua::state lua(&defaultErrorReporter);

    lua.set_global("b", 0.2);
    lua.eval("c = 9 + b");
    auto c =  lua.get_global("c");

    CAPTURE(c);

    REQUIRE( equals(c, 9.2) );
}

TEST_CASE( "simple/if", "" ) {
    lua::state lua(&defaultErrorReporter);

    std::string program = "if true then f = 0.1 else f = 'test' end";
    lua.eval(program);
    auto f = lua.get_global("f");

    REQUIRE( equals(f, 0.1) );
}

TEST_CASE( "simple/call", "Lua function is called with a few parameters from C++" ) {
    lua::state lua(&exceptionErrorReporter);

    REQUIRE_NOTHROW( lua.eval("function foo() end") );
    REQUIRE_NOTHROW( lua.call("foo") );
}

TEST_CASE( "simple/evalStream", "The VM evaluates a stream's contents using a reader" ) {
    lua::state lua(&exceptionErrorReporter);

    std::stringstream script;
    int g = 9;
    script << "g = " << g << ";";

    REQUIRE_NOTHROW( lua.eval(script) );
    REQUIRE( equals(lua.get_global("g"), 9.0) );
}

TEST_CASE( "simple/callWithParameters", "Lua function is called with a few parameters from C++" ) {
    lua::state lua(&exceptionErrorReporter);

    REQUIRE_NOTHROW( lua.eval("function my_add(i, j, k) return i + j + k end") );
    REQUIRE_NOTHROW( lua.call("my_add", 3, 6, 4) );
}

TEST_CASE( "simple/callCppFunction", "Desc" ) {
    lua::state lua(&exceptionErrorReporter);

    lua.register_function("plop_xyz", plop_xyz);
    lua.eval("x = plop_xyz(2, 6, 'hello')");
    std::cout << lua.get_global("x") << std::endl;

    REQUIRE_NOTHROW ( );
}

TEST_CASE( "simple/callLambda", "A C++ lambda is exposed to lua and called") {
    lua::state lua(&exceptionErrorReporter);
    
    int x = 0;

    lua.register_function("foo", [&x]{ x = 1; });

    lua.eval("foo()");

    REQUIRE( x == 1 );
}

TEST_CASE( "advanced/callLambdaReturns", "Checks for lambdas returning values") {
    lua::state lua(&exceptionErrorReporter);

    lua.register_function("a", []{ return 42; });
    //lua.register_function("b", []{ return 42u; });
    lua.register_function("c", []{ return 3.14; });
    lua.register_function("d", []{ return 6.28f; });
    lua.register_function("e", []{ return "lol"; });
    lua.register_function("f", []{ return true; });
    lua.register_function("g", []{ return std::string("str"); });
}

TEST_CASE( "advanced/callLambda2", "A C++ lambda is exposed to lua and called") {
    lua::state lua(&exceptionErrorReporter);

    int x = 0;
    lua.register_function("set_x", [&](int new_x){
        x = new_x;
        return 0;
    });

    lua.eval("set_x(9)");
    REQUIRE( x == 9 );
}

TEST_CASE( "negative/basicError", "Check if error handling works correctly" ) {
    lua::state lua(&exceptionErrorReporter);

    REQUIRE_THROWS( lua.eval("nil[5]") );
}

