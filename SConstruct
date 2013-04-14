# scons 2.7
import platform
import os

# set up your MinGW path here
mingw_path = "C:/DEV/MinGW32-4.8/mingw32/"
# set your Boost path here
boost_path = "C:/DEV/boost_1_52_0/"
# set your Boost path here
lua_path = "C:/DEV/Lua5.1"

if platform.system() == 'Windows':
    env = Environment(tools = ['mingw'], ENV = os.environ)
    env.PrependENVPath('PATH', mingw_path + "bin/")
    env.PrependENVPath('LIB', mingw_path + "lib/")
    
    # these aren't needed on *ix systems, since libraries are allready
    # in some sort of /usr/lib/
    env.Append(CPPPATH=[
        boost_path,
        lua_path + "/include",
        "include"
    ])
    env.Append(CPPFLAGS=["-Wall", "-g", "-std=c++11"])
    env.Append(LIBPATH=[lua_path+"/lib"])
else:
    env = Environment(ENV = os.environ)
    # required on Travis because of gcc4.6 and clang 3.1
    env.Append(CPPFLAGS=["-Wall", "-g", "-std=c++0x"])
    env.Append(CPPPATH=[
        "/usr/include/lua5.1/",
        "include"
    ])

test = env.Program("test.cpp", LIBS="lua5.1")