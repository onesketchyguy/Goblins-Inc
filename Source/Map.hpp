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

	struct ObjectData
	{
		std::string name = "";
		int sprIndex = 0;
	};

	class Map
	{
	private:
		Uint16 width = 0, height = 0;
		Uint32 mapLength = 0;

		std::vector<TileData> tiles{};
		gobl::Sprite* envTex = nullptr;
		IntVec2 sprSize{ 0,0 };

		std::vector<ObjectData> objects{};
		std::vector<gobl::Sprite*> objSprites{};

		Uint32* mapLayers;
		Sint32* objLayers;
		Uint64 sprLength = 0;

		gobl::GoblEngine* ge = nullptr;

	private: // XML stuff
		void LoadMapModData(const char* path);
		void LoadObjModData(const char* path);

	public: // Main map stuff
		Map() = default;
		void Destroy() 
		{
			if (envTex != nullptr) delete envTex; 

			delete mapLayers;
			delete objLayers;

			for (auto& s : objSprites) delete s;
		}

		Map(gobl::GoblEngine* ge, int w, int h, const char* path);
		void ResetTexture();
		void DrawTile(Uint32 x, Uint32 y);
		void Draw();
		void DrawRegion(Uint32 w, Uint32 h, int x, int y);
		void BlurDrawRegion(Uint32 w, Uint32 h, int offX, int offY);

		const IntVec2 GetMapSize() { return { width, height }; }

		Uint32 GetTileTypeCount() { return tiles.size(); }
		Uint32 GetObjectCount() { return objSprites.size(); }
		gobl::Sprite* GetObjTexture(const Uint32 index) { return objSprites[index]; }

		void SetObject(Uint32 id, Sint32 index) { objLayers[id] = index; };

		MAP::TileData GetType(const Uint32 layerID) { return tiles[layerID]; }

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