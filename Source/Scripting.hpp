#pragma once
#ifndef SCRIPTING_HPP
#define SCRIPTING_HPP
#include "GoblEngine.hpp"
#include "Map.hpp"

namespace gobl
{
	class LuaMachine 
	{
	public:
		LuaMachine();
		~LuaMachine();

	public:
		void SetEngine(gobl::GoblEngine* ge);
		void SetMap(MAP::Map* map);

		void TestLua();
		void RunScript(const char* dir);
		void RunScriptFunction(const char* dir, const char* func);
		//void RegisterFunction(const char* name, void* func); // FIXME: make this work

		void RegisterCommonFunctions();
	};
}

#endif // !SCRIPTING_HPP