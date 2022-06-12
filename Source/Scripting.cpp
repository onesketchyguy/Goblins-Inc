#include "Scripting.hpp"
#include <iostream>

// Import lua files
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

lua_State* L = nullptr;
gobl::GoblEngine* ge = nullptr;
MAP::Map* map = nullptr;

// Helper functions
bool CheckLua(lua_State* L, int r) 
{
	if (r != LUA_OK)
	{
		// Error
		std::string errMsg = lua_tostring(L, -1);
		std::cout << "\tLuaERROR: " << errMsg << std::endl;
		return false;
	}
	return true;
}

bool CheckLuaFile(lua_State* L, const char* dir)
{
	if (!CheckLua(L, luaL_dofile(L, dir)))
	{
		return false;
	}
	return true;
}

// Public lua machine functions
gobl::LuaMachine::LuaMachine()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	RegisterCommonFunctions();
}

gobl::LuaMachine::~LuaMachine()
{
	lua_close(L);
}

void gobl::LuaMachine::TestLua()
{
	// Local Lua testing
	std::string cmd = "a = 7 + 11 + math.sin(3)";

	if (CheckLua(L, luaL_dostring(L, cmd.c_str())))
	{
		// Do things
		lua_getglobal(L, "a");
		if (lua_isnumber(L, -1))
		{
			float a_in_cpp = (float)lua_tonumber(L, -1);
			std::cout << "\tLua Print: " << a_in_cpp << std::endl;
		}
	}

	// File lua testing
	if (CheckLuaFile(L, "Mods/test.lua"))
	{
		// Do things
		lua_getglobal(L, "b");
		if (lua_isnumber(L, -1))
		{
			float a_in_cpp = (float)lua_tonumber(L, -1);
			std::cout << "\tLua Print: " << a_in_cpp << std::endl;
		}

		lua_getglobal(L, "DoAThing");
		CheckLua(L, lua_pcall(L, 0, 0, 0));
	}
}

void gobl::LuaMachine::RunScript(const char* dir)
{
	CheckLuaFile(L, dir);
}

void gobl::LuaMachine::RunScriptFunction(const char* dir, const char* func)
{
	if (CheckLuaFile(L, dir))
	{
		// Do things
		lua_getglobal(L, func);
		CheckLua(L, lua_pcall(L, 0, 0, 0));
	}
}

int lua_PlaySound(lua_State* L) 
{
	const char* v = lua_tostring(L, 1);
	ge->GetAudio()->PlaySound(v);
	return 0;
}

int lua_SetTile(lua_State* L)
{
	int id = 0; // FIXME: Get the id from the lua code
	map->SetTile(id, -1);
	return id;
}

void gobl::LuaMachine::RegisterCommonFunctions()
{
	lua_register(L, "PlaySound", lua_PlaySound);
}

void gobl::LuaMachine::SetEngine(gobl::GoblEngine* _ge) { ge = _ge; };
void gobl::LuaMachine::SetMap(MAP::Map* _map) { map = _map; }