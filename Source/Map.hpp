#pragma once
#ifndef MAP_H
#define MAP_H

#include <iostream>
#include "GoblEngine.hpp"
#include "../libs/tinyxml2.h"

bool MAP_DEBUG_VERBOSE = false;

namespace MAP 
{
	struct TileData 
	{
		std::string name = "";

		// What can be placed on this
		std::string buildLayer = "";

		// What layer this is on
		std::string layer = "";

		bool canMultiPlace = false;

		int sprIndex = 0;
	};
}

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

private: // XML stuff
	void HandleAttributes(tinyxml2::XMLElement* currElement, MAP::TileData& tileData)
	{
		std::string element = std::string(currElement->Name());

		if (MAP_DEBUG_VERBOSE) std::cout << "\t" << element << " tag: " << std::endl;

		// Read attributes
		auto curAtt = currElement->FirstAttribute();
		if (curAtt != nullptr)
		{
			while (curAtt != nullptr)
			{
				std::string currAttValue = std::string(curAtt->Value());

				if (std::string(curAtt->Name()) == "i" && element == "sprite")
				{
					tileData.sprIndex = std::stoi(curAtt->Value());

					if (MAP_DEBUG_VERBOSE)
						std::cout << "\t\tIndex attribute: " << tileData.sprIndex << std::endl;
				}
				else if (std::string(curAtt->Name()) == "layer")
				{
					if (element == "buildable") 
					{
						tileData.buildLayer = currAttValue;

						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tLayer attribute: " << tileData.buildLayer << std::endl;
					}
					else if (element == "placable")
					{
						tileData.layer = currAttValue;

						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tLayer attribute: " << tileData.layer << std::endl;
					}
					else 
					{
						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tLayer Attribute found on unknown tag: " << curAtt->Value() << std::endl;
					}
				}
				else if (std::string(curAtt->Name()) == "multi" && element == "placable")
				{
					if (currAttValue == "true") tileData.canMultiPlace = true;

					if (MAP_DEBUG_VERBOSE)
						std::cout << "\t\tMulti-place Attribute: " << curAtt->Value() << std::endl;
				}
				else
				{
					if (MAP_DEBUG_VERBOSE)
						std::cout << "\t\tUnknown Attribute: " << curAtt->Name() << ", " << curAtt->Value() << std::endl;
				}

				curAtt = curAtt->Next();
			}
		}
		else
		{
			std::cout << "ERROR: No " << element << " attributes found!" << std::endl;
		}
	}

	void LoadMapModData(gobl::GoblEngine* ge, const char* path)
	{
		std::string texturePath = "Sprites/";

		tinyxml2::XMLDocument doc;
		doc.LoadFile(path);

		auto current = doc.FirstChildElement();
		if (current != nullptr)
		{
			int id = 0;

			// Read each mod object
			while (current != nullptr)
			{
				const char* title = current->FindAttribute("name")->Value();

				if (MAP_DEBUG_VERBOSE)
					std::cout << current->Name() << ": " << title << std::endl;

				// Set sprite data
				if (std::string(current->Name()) == "EnvironmentSprite")
				{
					texturePath += std::string(title);

					auto curAtt = current->FirstAttribute();
					if (curAtt != nullptr)
					{
						while (curAtt != nullptr)
						{
							if (std::string(curAtt->Name()) == "w")
							{
								sprSize.x = std::stoi(curAtt->Value());

								if (MAP_DEBUG_VERBOSE)
									std::cout << "\t\tWidth attribute: " << curAtt->Name() << ", " << sprSize.x << std::endl;
							}
							else if (std::string(curAtt->Name()) == "h")
							{
								sprSize.y = std::stoi(curAtt->Value());

								if (MAP_DEBUG_VERBOSE)
									std::cout << "\t\tHeight attribute: " << curAtt->Name() << ", " << sprSize.y << std::endl;
							}
							else if(std::string(curAtt->Name()) != "name")
							{
								if (MAP_DEBUG_VERBOSE)
									std::cout << "\t\tUnknown Attribute: " << curAtt->Name() << ", " << curAtt->Value() << std::endl;
							}

							curAtt = curAtt->Next();
						}
					}
					else
					{
						std::cout << "ERROR: No sprite attributes provided!" << std::endl;
					}
				}
				else 
				{
					// Handle element data
					auto curModElement = current->FirstChildElement();

					MAP::TileData tileData;
					tileData.name = std::string(title);

					// Read each element of that mod object
					while (curModElement != nullptr)
					{
						HandleAttributes(curModElement, tileData);

						// Move to next element
						curModElement = curModElement->NextSiblingElement();
					}

					// Provide info for buildable tag and others
					envObjects.push_back(tileData);

					id++;
				}

				// Move on to the next mod block
				current = current->NextSiblingElement();

				if (MAP_DEBUG_VERBOSE) std::cout << std::endl;
			}

			if (MAP_DEBUG_VERBOSE)
				std::cout << "\n\nFinished loading map mods." << std::endl;

			// Only create one new sprite for the entire map texture
			// Create the sprite object
			envTex = ge->CreateSpriteObject(texturePath.c_str(), true);
			envTex->SetDimensions(sprSize.x, sprSize.y);

		}
		else
		{
			std::cout << "ERROR: No environment mod found!" << std::endl;
		}
	}

public: // Main map stuff
	Map() = default;
	void Destroy() 
	{
		if (envTex != nullptr) delete envTex;
	}

	Map(gobl::GoblEngine* ge, int w, int h, const char* path)
	{
		width = w;
		height = h;
		mapLength = width * height;
		mapLayers = new int[mapLength];

		for (Uint32 i = 0; i < mapLength; i++) mapLayers[i] = 0; // FIXME: Load old map data

		// Load all the mods
		LoadMapModData(ge, path);
	}

	void Draw() 
	{
		envTex->SetDimensions(sprSize.x, sprSize.y);

		for (Uint32 i = 0; i < mapLength; i++)
		{
			int x = i % width;
			int y = i / width;

			envTex->SetSpriteIndex(GetTypeSprite(mapLayers[i]));
			envTex->SetPosition(envTex->GetScale().x * x, envTex->GetScale().y * y);
			envTex->Draw();
		}
	}

	Uint32 GetTileTypeCount() { return envObjects.size(); }

	std::string GetTypeName(int layerId) { return envObjects[layerId].name; }
	std::string GetTypeLayer(int layerId) { return envObjects[layerId].layer; }
	std::string GetTypeBuildable(int layerId) { return envObjects[layerId].buildLayer; }
	bool GetTypeMultiPlace(int layerId) { return envObjects[layerId].canMultiPlace; }
	Uint32 GetTypeSprite(int layerId) { return envObjects[layerId].sprIndex; }

	void ChangeTile(int id, Uint32 index) { mapLayers[id] = index; }
	Uint32 GetTileLayer(int id) { return mapLayers[id]; }

	bool Overlaps(int id, int x, int y)
	{
		auto scale = envTex->GetScale();

		int tileX = scale.x * (id % width);
		int tileY = scale.y * (id / width);

		return (y >= tileY && y <= tileY + scale.y) && (x >= tileX && x <= tileX + scale.y);
	}

	int GetTile(int x, int y)
	{
		if (x > width || x < 0) return -1;
		if (y > height || y < 0) return -1;

		return y * width + x;
	}

	int GetTileFromWorldPos(int x, int y) 
	{
		if (x > width * envTex->GetScale().x || x < 0) return -1;
		if (y > height * envTex->GetScale().y || y < 0) return -1;

		x -= x % envTex->GetScale().x;
		y -= y % envTex->GetScale().y;

		x /= envTex->GetScale().x;
		y /= envTex->GetScale().y;

		return y * width + x;
	}

	gobl::Sprite* GetTexture() { return envTex; }

	IntVec2 GetTileMapPos(int x, int y)
	{
		if (x > width * envTex->GetScale().x || x < 0) return IntVec2{ -1, -1 };
		if (y > height * envTex->GetScale().y || y < 0) return IntVec2{ -1, -1 };

		x -= x % envTex->GetScale().x;
		y -= y % envTex->GetScale().y;

		x /= envTex->GetScale().x;
		y /= envTex->GetScale().y;

		return IntVec2{ x, y };
	}

	IntVec2 GetTilePos(int id) 
	{
		return { envTex->GetScale().x * (id % width), envTex->GetScale().y * (id / width) };
	}
};

#endif // !MAP_H