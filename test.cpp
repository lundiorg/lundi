
#include <iostream>
#include <string>
#include <exception>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform_view.hpp>
#include <boost/phoenix/phoenix.hpp>

#include "lundi.hpp"

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

namespace {

    typedef boost::mpl::vector<signed int, double, bool, std::string, lua::nil> basic_types;
    //typedef boost::mpl::vector<double> basic_types;

    namespace c_funs {

        // left for historical purposes
        int plop_xyz(int x, int y, std::string z) {
            std::cout << x << " " << y << " " << z << std::endl;
            return 11;
        }

        // the naming pattern goes:
        // {return_type}_{number_of_args}

        // basic empty functions
        namespace basic {
            template<typename A>
            void void_unary(A a) { }
            template<typename A>
            void void_binary_same(A a, A b) { }
            template<typename A, typename B>
            void void_binary_diff(A a, B b) { }

            // function without parameters
            template<typename Ret>
            Ret nonparam() { return Ret{}; }

            // functions returning nonparameter
            template<typename A>
            A same_unary(A a) { return a; }
            template<typename A>
            A same_binary_same(A a, A b) { return a; }
            template<typename A, typename B>
            A first_binary_diff(A a, B b) { return a; }
            template<typename A, typename B>
            B second_binary_diff(A a, B b) { return b; }
        }

        // functions with aggregates
        /*namespace aggregate{
            template<class Aggr, typename A>
            void void_unary(Aggr<A> a) { }
            // TODO add more
        }*/
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

template<typename F>
struct calls {
    lua::state& state;
    F& f;

    calls(lua::state& state, F& f) : state(state), f(f) {}

    template<typename... Sig>
    void operator() (Sig...) {
        state.register_function("foo", f<Sig...>);
    }
};

struct void_calls {
    void_calls(lua::state& state) : state(state) {}
    lua::state& state;

    template<typename A>
    void operator() (A) {
        state.register_function("foo", c_funs::basic::void_unary<A>);
        state.register_function("foo", c_funs::basic::void_binary_same<A>);
    }
};

TEST_CASE( "coverage/void_call", "Check if basic calls in form of void(T) compile.") {
    lua::state lua (&exceptionErrorReporter);
    namespace mpl = boost::mpl;

    mpl::for_each<basic_types>(void_calls(lua));

    //lua.register_function("foo", c_funs::basic::void_unary<double>);
    
    // that will work for functors.
    //mpl::for_each<basic_types>(calls<decltype(c_funs::basic::void_unary)>(lua, c_funs::basic::void_unary));
}

TEST_CASE(" coverage/lambda_call", "Check if basic calls in form of void <lambda>(T) compile.") {
    lua::state lua(&exceptionErrorReporter);

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

    lua.register_function("plop_xyz", c_funs::plop_xyz);
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

