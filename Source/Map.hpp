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
		void ResetTexture();
		void Draw();
		void DrawRegion(Uint32 w, Uint32 h, int x, int y);
		void BlurDrawRegion(Uint32 w, Uint32 h, int offX, int offY);

		const IntVec2 GetMapSize() { return { width, height }; }

		Uint32 GetTileTypeCount() { return envObjects.size(); }

		std::string GetTypeName(const Uint32 layerId) { return envObjects[layerId].name; }
		std::string GetTypeLayer(const Uint32 layerId) { return envObjects[layerId].layer; }
		std::string GetTypeBuildable(const Uint32 layerId) { return envObjects[layerId].buildLayer; }
		bool GetTypeMultiPlace(const Uint32 layerId) { return envObjects[layerId].canMultiPlace; }
		bool GetTypeLinear(const Uint32 layerId) { return envObjects[layerId].linear; }
		Uint32 GetTypeSprite(const Uint32 layerId) { return envObjects[layerId].sprIndex; }

		void ChangeTile(int id, Uint32 index) { mapLayers[id] = index; }
		Uint32 GetTileLayer(int id) { return mapLayers[id]; }

		gobl::Sprite* GetTexture() { return envTex; }

		bool Overlaps(int id, int x, int y);
		int GetTile(int x, int y);
		int GetTileFromWorldPos(int x, int y);
		IntVec2 GetTileMapPos(int x, int y);
		IntVec2 GetClosestTileMapPos(int x, int y);
		IntVec2 GetTilePos(int id);
	};
}

#endif // !MAP_H