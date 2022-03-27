#pragma once
#ifndef MAP_H
#define MAP_H

#include <iostream>
#include "GoblEngine.hpp"

namespace MAP 
{
	extern bool MAP_DEBUG_VERBOSE;

	struct TileData 
	{
		std::string name = "";

		// What can be placed on this
		std::string buildLayer = "";

		// What layer this is on
		std::string layer = "";

		bool canMultiPlace = false;
		bool linear = false;

		int sprIndex = 0;
	};

	class Map
	{
	private:
		Uint16 width = 0, height = 0;
		Uint32 mapLength = 0;

		gobl::Sprite* envTex = nullptr;
		IntVec2 sprSize{ 0,0 };

		std::vector<MAP::TileData> envObjects{};

		int* mapLayers;
		Uint64 sprLength = 0;

		gobl::GoblEngine* ge = nullptr;

	private: // XML stuff
		void LoadMapModData(const char* path);

	public: // Main map stuff
		Map() = default;
		void Destroy() { if (envTex != nullptr) delete envTex; }

		Map(gobl::GoblEngine* ge, int w, int h, const char* path);
		void Draw();

		Uint32 GetTileTypeCount() { return envObjects.size(); }

		std::string GetTypeName(int layerId) { return envObjects[layerId].name; }
		std::string GetTypeLayer(int layerId) { return envObjects[layerId].layer; }
		std::string GetTypeBuildable(int layerId) { return envObjects[layerId].buildLayer; }
		bool GetTypeMultiPlace(int layerId) { return envObjects[layerId].canMultiPlace; }
		bool GetTypeLinear(int layerId) { return envObjects[layerId].linear; }
		Uint32 GetTypeSprite(int layerId) { return envObjects[layerId].sprIndex; }

		void ChangeTile(int id, Uint32 index) { mapLayers[id] = index; }
		Uint32 GetTileLayer(int id) { return mapLayers[id]; }

		gobl::Sprite* GetTexture() { return envTex; }

		bool Overlaps(int id, int x, int y);
		int GetTile(int x, int y);
		int GetTileFromWorldPos(int x, int y);
		IntVec2 GetTileMapPos(int x, int y);
		IntVec2 GetTilePos(int id);
	};
}

#endif // !MAP_H