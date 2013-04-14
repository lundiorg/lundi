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
    ])
    env.Append(CPPFLAGS=["-Wall", "-g", "-std=c++11"])
    env.Append(LIBPATH=[lua_path+"/lib"])
else:
    env = Environment(ENV = os.environ)
    env.Append(CPPFLAGS=["-Wall", "-g", "-std=c++11"])

test = env.Program("test.cpp", LIBS="lua5.1")