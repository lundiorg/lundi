#pragma once

#include <lua.hpp>

namespace lua {

#if LUA_VERSION_NUM >= 502
inline
int lua_load(lua_State *L, lua_Reader reader, void *dt, const char *chunkname) {
	return lua_load(L, reader, dt, chunkname, 0);
}

#endif

}
