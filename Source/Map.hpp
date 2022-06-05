#pragma once
#ifndef MAP_H
#define MAP_H

#include "GoblEngine.hpp"
#include <iostream>
#include <unordered_map>

namespace MAP 
{
	const char SPRITE_ATT[9] = "sprIndex";
	const char PRICE_ATT[6] = "price";
	const char MULTI_PLACE_ATT[6] = "multi";
	const char LINEAR_ATT[7] = "linear";
	const char WORKABLE_ATT[9] = "workable";

	extern bool MAP_DEBUG_VERBOSE;

	struct TileData 
	{
	private:
		std::unordered_map<std::string, int> intAtts{};
		std::unordered_map<std::string, bool> boolAtts{};

	public:
		std::string name = "";

		// What can be placed on this
		std::string buildLayer = "";

		// What layer this is on
		std::string layer = "";

		void ClearIntAttribute(std::string val) { intAtts.erase(val); }
		void ClearBoolAttribute(std::string val) { boolAtts.erase(val); }

		int GetIntAttribute(std::string val)
		{
			if (intAtts.find(val) == intAtts.end())
			{
				if (MAP_DEBUG_VERBOSE) std::cout << "ERROR: int attribute; " << val << " not found." << std::endl;
				return 0;
			}
			else return intAtts[val];
		}

		bool GetBoolAttribute(std::string val)
		{
			if (boolAtts.find(val) == boolAtts.end())
			{
				if (MAP_DEBUG_VERBOSE) std::cout << "ERROR: bool attribute; " << val << " not found." << std::endl;
				return false;
			}
			else return boolAtts[val];
		}

		void SetIntAttribute(std::string name, int val) 
		{ 
			if (intAtts.find(name) == intAtts.end()) intAtts.emplace(name, val);
			else intAtts[name] = val;
		}
		void SetBoolAttribute(std::string name, bool val) 
		{
			if (boolAtts.find(name) == boolAtts.end()) boolAtts.emplace(name, val);
			else boolAtts[name] = val;
		}
	};

	class Map
	{
	private:
		Uint16 width = 0, height = 0;
		Uint32 mapLength = 0;

		std::vector<TileData> tiles{};
		gobl::Sprite* envTex = nullptr;

		std::vector<TileData> objects{};
		std::vector<gobl::Sprite*> objSprites{};

		Uint32* mapLayers = nullptr;
		Sint32* objLayers = nullptr;
		bool* colMap = nullptr;
		Uint64 sprLength = 0;

		std::vector<Uint32> workables{};

		gobl::GoblEngine* ge = nullptr;

	private: // XML stuff
		void LoadModData(const char* path);

	public: // Main map stuff
		Map() = default;
		void Destroy() 
		{
			if (envTex != nullptr) delete envTex; 

			delete mapLayers;
			delete objLayers;
			delete colMap;

			for (auto& s : objSprites) delete s;
		}

		Map(gobl::GoblEngine* ge, int w, int h, const char* path);
		void ResetTexture();
		void DrawTile(Uint32 x, Uint32 y);
		void Draw();
		void DrawRegion(Uint32 w, Uint32 h, int x, int y);
		void BlurDrawRegion(Uint32 w, Uint32 h, int offX, int offY);

		void UpdateObjects();

		void SetCollision(Uint32 index, bool value) { colMap[index] = value; }
		bool GetCollision(Uint32 index) { return colMap[index]; }

		const IntVec2 GetMapSize() { return { width, height }; }

		Uint32 GetTileTypeCount() { return tiles.size(); }
		Uint32 GetObjectCount() { return objSprites.size(); }

		MAP::TileData GetType(const Uint32 layerID) 
		{
			if (layerID < tiles.size()) return tiles[layerID];
			else if (layerID - tiles.size() < objects.size()) return objects[layerID - tiles.size()];

			return TileData{};
		}

		void SetObject(Uint32 id, Sint32 index);
		void SetTile(int id, Uint32 index);
		int GetEmptyWorkable();
		IntVec2 GetWorkable(int id);

		Uint32 GetTileLayer(int id) { return mapLayers[id]; }
		int GetObjectLayer(int id) { return objLayers[id]; }
		gobl::Sprite* GetTileTexture() { return envTex; }
		gobl::Sprite* GetTexture(const Uint32 index) { return objSprites[index]; }

		bool Overlaps(int id, int x, int y);
		int GetTile(int x, int y);
		int GetTileFromWorldPos(int x, int y);
		IntVec2 GetTileMapPos(int x, int y);
		IntVec2 GetClosestTileMapPos(int x, int y);
		IntVec2 GetTilePos(int id);
	};
}

#endif // !MAP_H