#include "Map.hpp"
#include "../libs/tinyxml2.h"

namespace MAP
{
	const std::string TEXTURE_PATH = "Sprites/";
	bool MAP_DEBUG_VERBOSE = false;

	// XML stuff
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
				else if (std::string(curAtt->Name()) == "linear" && element == "placable")
				{
					if (currAttValue == "true") tileData.linear = true;

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
	void Map::LoadMapModData(const char* path)
	{
		std::string texturePath = TEXTURE_PATH;

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

				if (MAP_DEBUG_VERBOSE || std::string(current->Name()) == "EnvironmentSprite")
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

								std::cout << "\t\tWidth attribute: " << sprSize.x << std::endl;
							}
							else if (std::string(curAtt->Name()) == "h")
							{
								sprSize.y = std::stoi(curAtt->Value());

								std::cout << "\t\tHeight attribute: " << sprSize.y << std::endl;
							}
							else if (std::string(curAtt->Name()) == "debug")
							{
								MAP_DEBUG_VERBOSE = std::string(curAtt->Value()) == "true";

								std::cout << "\t\tDebug attribute: " << sprSize.y << std::endl;
							}
							else if (std::string(curAtt->Name()) != "name")
							{
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
					tiles.push_back(tileData);

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
			envTex = ge->CreateSpriteObject(texturePath.c_str());
			envTex->SetDimensions(sprSize.x, sprSize.y);

		}
		else
		{
			std::cout << "ERROR: No environment mod found!" << std::endl;
		}
	}
	void Map::LoadObjModData(const char* path)
	{
		std::string texturePath = TEXTURE_PATH;

		ObjectData obj{};

		tinyxml2::XMLDocument doc;
		doc.LoadFile(path);

		// First get mod name
		auto currElement = doc.FirstChildElement();
		obj.name = std::string(currElement->FindAttribute("name")->Value());

		if (MAP_DEBUG_VERBOSE) std::cout << "ModObject: " << obj.name << std::endl;

		// Then get mod elements
		currElement = currElement->FirstChildElement();
		std::string elementName = std::string(currElement->Name());

		if (MAP_DEBUG_VERBOSE) std::cout << "\t" << elementName << " tag: " << std::endl;

		// Read attributes
		auto curAtt = currElement->FirstAttribute();
		if (curAtt != nullptr)
		{
			while (curAtt != nullptr)
			{
				std::string currAttValue = std::string(curAtt->Value());

				if (std::string(curAtt->Name()) == "name" && elementName == "sprite")
				{
					texturePath += currAttValue;

					if (MAP_DEBUG_VERBOSE)
						std::cout << "\t\tSprite location attribute: " << texturePath << std::endl;
				}

				curAtt = curAtt->Next();
			}
		}

		// Push the object to the stack
		obj.sprIndex = objSprites.size();
		objSprites.push_back(new gobl::Sprite());
		objSprites[obj.sprIndex] = ge->CreateSpriteObject(texturePath.c_str());
		objects.push_back(obj);
	}

	// Map stuff
	Map::Map(gobl::GoblEngine* ge, int w, int h, const char* path) : ge(ge), width(w), height(h)
	{
		mapLength = width * height;
		mapLayers = new int[mapLength];
		objLayers = new int[mapLength];

		for (Uint32 i = 0; i < mapLength; i++) mapLayers[i] = 0; // FIXME: Load old map data
		for (Uint32 i = 0; i < mapLength; i++) objLayers[i] = -1; // FIXME: Load old map data

		// FIXME: Provide an enumerator through the files

		// Load all the mods
		std::string envMods = path + std::string("Environment.xml");
		LoadMapModData(envMods.c_str());
		std::string objMods = path + std::string("StandardItems.xml");
		LoadObjModData(objMods.c_str());
	}

	void Map::ResetTexture() 
	{
		envTex->SetDimensions(sprSize);
		envTex->SetColorMod({ 255, 255, 255, 255 });
	}

	void Map::DrawTile(Uint32 x, Uint32 y) 
	{
		Uint64 i = i = y * width + x;

		// FIXME: Implement lights

		// Draw tiles
		envTex->SetSpriteIndex(GetTypeSprite(mapLayers[i]));
		envTex->SetPosition(envTex->GetScale().x * x, envTex->GetScale().y * y);
		envTex->DrawRelative(ge->GetCameraObject());

		// Draw items
		if (objLayers[i] >= 0)
		{
			Uint32 sprIndex = objects[objLayers[i]].sprIndex;
			objSprites[sprIndex]->SetPosition(envTex->GetScale().x * x, envTex->GetScale().y * y);
			objSprites[sprIndex]->DrawRelative(ge->GetCameraObject());
		}
	}

	// DEPRECATED: Far too inefficient to be worth using
	void Map::Draw()
	{
		for (size_t i = 0; i < mapLength; i++)
		{
			int x = i % width;
			int y = i / width;

			DrawTile(x, y);
		}
	}

	void Map::DrawRegion(Uint32 w, Uint32 h, int offX, int offY)
	{
		ResetTexture();
		if (offX < 0) offX = 0;
		if (offY < 0) offY = 0;

		for (Uint32 x = offX; x < offX + w; x++)
		{
			if (x >= width) continue;

			for (Uint32 y = offY; y < offY + h; y++)
			{
				if (y >= height) continue;

				DrawTile(x, y);
			}
		}
	}

	void Map::BlurDrawRegion(Uint32 w, Uint32 h, int offX, int offY)
	{
		DrawRegion(w, h, offX, offY);
	}

	bool Map::Overlaps(int id, int x, int y)
	{
		IntVec2 scale = envTex->GetScale();

		int tileX = scale.x * (id % width);
		int tileY = scale.y * (id / width);

		return (y >= tileY && y <= tileY + scale.y) && (x >= tileX && x <= tileX + scale.y);
	}

	int Map::GetTile(int x, int y)
	{
		if (x > width || x < 0) return -1;
		if (y > height || y < 0) return -1;

		return y * width + x;
	}

	int Map::GetTileFromWorldPos(int x, int y)
	{
		if (x > width * envTex->GetScale().x || x < 0) return -1;
		if (y > height * envTex->GetScale().y || y < 0) return -1;

		x -= x % envTex->GetScale().x;
		y -= y % envTex->GetScale().y;

		x /= envTex->GetScale().x;
		y /= envTex->GetScale().y;

		return y * width + x;
	}

	IntVec2 Map::GetTileMapPos(int x, int y)
	{
		if (x > width * envTex->GetScale().x || x < 0) return IntVec2{ -1, -1 };
		if (y > height * envTex->GetScale().y || y < 0) return IntVec2{ -1, -1 };

		x -= x % envTex->GetScale().x;
		y -= y % envTex->GetScale().y;

		x /= envTex->GetScale().x;
		y /= envTex->GetScale().y;

		return IntVec2{ x, y };
	}

	IntVec2 Map::GetClosestTileMapPos(int x, int y)
	{
		x -= x % envTex->GetScale().x;
		y -= y % envTex->GetScale().y;

		x /= envTex->GetScale().x;
		y /= envTex->GetScale().y;

		return IntVec2{ x, y };
	}

	IntVec2 Map::GetTilePos(int id)
	{
		return { envTex->GetScale().x * (id % width), envTex->GetScale().y * (id / width) };
	}
}