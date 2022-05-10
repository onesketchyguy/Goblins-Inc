#include "Map.hpp"
#include "../libs/tinyxml2.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <functional>

namespace MAP
{
	const std::string TEXTURE_PATH = "Sprites/";
	bool MAP_DEBUG_VERBOSE = false;

	// XML stuff
	void HandleAttributes(tinyxml2::XMLElement* currElement, MAP::TileData& tileData, std::string& texturePath)
	{
		std::string elementName = std::string(currElement->Name());

		if (elementName == "growable") tileData.SetBoolAttribute(elementName, true);

		// Read attributes
		auto curAtt = currElement->FirstAttribute();
		if (curAtt != nullptr)
		{
			while (curAtt != nullptr)
			{
				std::string currAttValue = std::string(curAtt->Value());
				std::string currAttName = std::string(curAtt->Name());

				if (currAttName == "layer")
				{
					if (elementName == "buildable")
					{
						tileData.buildLayer = currAttValue;

						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tLayer attribute: " << tileData.buildLayer << std::endl;
					}
					else if (elementName == "placable")
					{
						tileData.layer = currAttValue;

						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tLayer attribute: " << tileData.layer << std::endl;
					}
					else
					{
						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tLayer attribute found on unknown tag: " << curAtt->Value() << std::endl;
					}
				}
				else if (elementName == "placable" || elementName == "rotate" || currAttName == "collision")
				{
					if (currAttValue != "true" && currAttValue != "false")
					{
						std::cout << "\t\tAttribute " << currAttName << " unhandled! " << curAtt->Value() << std::endl;
					}
					else 
					{
						bool value = (currAttValue == "true");
						tileData.SetBoolAttribute(currAttName, value);

						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tAttribute " << currAttName << ": " << curAtt->Value() << std::endl;
					}
				}
				else if (currAttName == "debug")
				{
					MAP_DEBUG_VERBOSE = currAttValue == "true";

					std::cout << "\t\tDebug attribute: " << MAP_DEBUG_VERBOSE << std::endl;
				}
				else
				{
					// Try to add this attribute to the int list if possible
					try
					{
						tileData.SetIntAttribute(currAttName, std::stoi(curAtt->Value()));

						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tAttribute " << currAttName << ": " << tileData.GetIntAttribute(currAttName) << std::endl;
					}
					catch (std::exception&) // Ignore the exception error since it's expected for some attributes
					{
						// Handle the sprite location attribute
						if (elementName == "sprite")
						{
							texturePath += currAttValue;

							if (MAP_DEBUG_VERBOSE)
								std::cout << "\t\tLocation attribute: " << texturePath << std::endl;
						}
						else if (elementName == WORKABLE_ATT)
						{
							tileData.SetBoolAttribute(elementName, true);

							if (MAP_DEBUG_VERBOSE)
								std::cout << "\t\tWorkable attribute script: " << currAttValue << std::endl;
						}
						else 
						{
							// This attribute has no handler
							if (MAP_DEBUG_VERBOSE)
								std::cout << "\t\tUnknown and unhandled attribute: " << curAtt->Name() << ", " << curAtt->Value() << std::endl;
						}
					}
				}

				curAtt = curAtt->Next();
			}
		}
		else
		{
			tileData.SetBoolAttribute(elementName, true);
			std::cout << "\t\tWARNING: No " << elementName << " attributes found!" << std::endl;
		}
	}
	void HandleObjElements(tinyxml2::XMLElement* currElement, MAP::TileData& tileData, std::string& texturePath)
	{
		tileData.name = std::string(currElement->FindAttribute("name")->Value());

		HandleAttributes(currElement, tileData, texturePath);
		if (MAP_DEBUG_VERBOSE) std::cout << "ModObject: " << tileData.name << std::endl;

		// Then get mod elements
		currElement = currElement->FirstChildElement();
		while (currElement != nullptr) 
		{
			std::string elementName = std::string(currElement->Name());

			if (MAP_DEBUG_VERBOSE) std::cout << "\t" << elementName << " tag: " << std::endl;

			// Read attributes
			HandleAttributes(currElement, tileData, texturePath);

			currElement = currElement->NextSiblingElement();
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

								std::cout << "\t\tDebug attribute: " << MAP_DEBUG_VERBOSE << std::endl;
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
						std::string empty = "";
						HandleAttributes(curModElement, tileData, empty);

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
		tinyxml2::XMLDocument doc;
		doc.LoadFile(path);
		tinyxml2::XMLElement* currElement = doc.FirstChildElement();

		while (currElement != nullptr) 
		{
			std::string texturePath = TEXTURE_PATH;
			TileData obj{};

			HandleObjElements(currElement, obj, texturePath);

			// Push the object to the stack
			obj.SetIntAttribute(SPRITE_ATT, static_cast<int>(objSprites.size()));
			objSprites.push_back(new gobl::Sprite());
			objSprites[obj.GetIntAttribute(SPRITE_ATT)] = ge->CreateSpriteObject(texturePath.c_str());

			int dX = obj.GetIntAttribute("dimX");
			int dY = obj.GetIntAttribute("dimY");
			if (dX != 0 && dY != 0) objSprites[obj.GetIntAttribute(SPRITE_ATT)]->SetStaticDimensions(dX, dY);
			else objSprites[obj.GetIntAttribute(SPRITE_ATT)]->SetStaticDimensions(sprSize.x, sprSize.y);

			objects.push_back(obj);

			currElement = currElement->NextSiblingElement();
		}
	}

	// Map stuff
	Map::Map(gobl::GoblEngine* ge, int w, int h, const char* path) : ge(ge), width(w), height(h)
	{
		mapLength = width * height;
		mapLayers = new Uint32[mapLength];
		objLayers = new Sint32[mapLength];
		colMap = new bool[mapLength];

		std::cout << "Loading mods..." << std::endl;
		// Load all the mods
		for (const auto& file : std::filesystem::recursive_directory_iterator(path))
		{
			if (file.is_directory() || file.path().extension() != ".xml") continue;

			if (file.path().filename() == "Environment.xml")
			{
				std::cout << "Found environment file." << std::endl;
				std::string envMods = path + std::string("Environment.xml");
				LoadMapModData(envMods.c_str());
			}
			else 
			{
				std::string modPath = path + file.path().filename().string();
				std::cout << "-Loading mod: " << modPath << std::endl;
				LoadObjModData(modPath.c_str());
			}
		}

		std::cout << "Finished loading mods." << std::endl;
		std::cout << "Loading world..." << std::endl;

		// Find all the items that can be spawned at the creation of the world
		std::vector<int> initObjects{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }; // Create a list of empty objects to posative bias empty tiles
		int i = 0;
		for (auto& obj : objects)
		{
			if (obj.GetBoolAttribute("growable")) 
			{
				initObjects.push_back(i);
			}
			i++;
		}

		// FIXME: Load old map data
		for (Uint32 i = 0; i < mapLength; i++)
		{
			mapLayers[i] = 0;
			objLayers[i] = initObjects[rand() % initObjects.size()];
			colMap[i] = false;
		}
	}

	void Map::ResetTexture() 
	{
		envTex->SetDimensions(sprSize);
		envTex->SetColorMod(Color::WHITE);

		for (auto& spr : objSprites)
		{
			spr->SetSpriteIndex(0);
			spr->ResetDimensions();
			spr->SetColorMod(Color::WHITE);
		}
	}

	void Map::DrawTile(Uint32 x, Uint32 y) 
	{
		Uint64 i = i = y * width + x;

		// FIXME: Implement lights

		// Draw tiles
		envTex->SetSpriteIndex(GetType(mapLayers[i]).GetIntAttribute(SPRITE_ATT));
		envTex->SetPosition(envTex->GetScale().x * x, envTex->GetScale().y * y);

		if (gobl::GoblEngine::debugging) 
		{
			if (colMap[i]) envTex->SetColorMod(Color::RED);
			else envTex->SetColorMod(Color::LIGHT_BLUE);
		}

		envTex->DrawRelative(ge->GetCameraObject());

		// Draw items
		// FIXME: Move "objects" over to Objects with positions instead of being pure data in an array
		if (objLayers[i] >= 0)
		{
			Uint32 sprIndex = objects[objLayers[i]].GetIntAttribute(SPRITE_ATT);

			if (gobl::GoblEngine::debugging && objects[objLayers[i]].GetBoolAttribute(WORKABLE_ATT)) 
				objSprites[sprIndex]->SetColorMod(Color::GREEN);
			else objSprites[sprIndex]->SetColorMod(Color::WHITE);


			// FIXME: Update growables even when they aren't on screen
			if (objects[objLayers[i]].GetBoolAttribute("growable"))
			{
				const char GROW_INDEX = objects[objLayers[i]].GetIntAttribute("length") - 1; // Allow the modder to specify the index

				std::string growableName = "GrowthIndex" + std::to_string(i);
				int growableIndex = objects[objLayers[i]].GetIntAttribute(growableName);
				objSprites[sprIndex]->SetSpriteIndex(GROW_INDEX - growableIndex);

				if (growableIndex < GROW_INDEX)
				{
					std::string growthName = "growth" + std::to_string(i);
					int growthIndex = objects[objLayers[i]].GetIntAttribute(growthName);

					if (growthIndex >= objects[objLayers[i]].GetIntAttribute("rate")) // Allow the modder to specify the growth rate
					{
						objects[objLayers[i]].SetIntAttribute(growableName, growableIndex + 1);
						objects[objLayers[i]].SetIntAttribute(growthName, 0);

						if (growableIndex + 1 >= GROW_INDEX) objects[objLayers[i]].ClearIntAttribute(growthName);
					}
					else objects[objLayers[i]].SetIntAttribute(growthName, growthIndex + 1);
				}
			}

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

	int Map::GetEmptyWorkable() 
	{
		// Find an available workable
		for (auto& o : workables)
		{
			if (objects[objLayers[o]].GetBoolAttribute(WORKABLE_ATT) == false) continue;
			if (objects[objLayers[o]].GetBoolAttribute("inUse") == false) return o;
		}

		return -1;
	}

	IntVec2 Map::GetWorkable(int id) 
	{
		if (id != -1) 
		{
			// Check for if the workable is currently in use
			if (objects[objLayers[id]].GetBoolAttribute("inUse") == false) return GetTilePos(id);
		}

		// Find an available workable
		for (auto& o : workables)
		{
			if (objects[objLayers[o]].GetBoolAttribute(WORKABLE_ATT) == false) continue;
			if (objects[objLayers[o]].GetBoolAttribute("inUse") == false) 
				return IntVec2{ (Sint32(o) % width) * envTex->GetScale().x , (Sint32(o) / width) * envTex->GetScale().y };
		}

		return IntVec2{ 0, 0 };
	}
}