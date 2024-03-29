#include "Map.hpp"
#include "../libs/tinyxml2.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <functional>

namespace MAP
{
	const std::string TEXTURE_PATH = "Sprites/";
	const std::string GROWABLE_TAG = "growable";
	const std::string GROWTH_INDEX = "GrowthIndex";
	const std::string LENGTH_TAG = "length";
	bool MAP_DEBUG_VERBOSE = false;

	IntVec2 sprSize{ 0,0 };

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

				// Set the map sprite data
				if (elementName == "EnvironmentSprite")
				{
					if (currAttName == "name")
					{
						texturePath += currAttValue;
					}

					if (currAttName == "w")
					{
						sprSize.x = std::stoi(currAttValue);

						std::cout << "\t\tWidth attribute: " << sprSize.x << std::endl;
					}
					else if (currAttName == "h")
					{
						sprSize.y = std::stoi(currAttValue);

						std::cout << "\t\tHeight attribute: " << sprSize.y << std::endl;
					}
					else if (currAttName != "name")
					{
						std::cout << "\t\tUnknown Attribute: " << currAttName << ", " << currAttValue << std::endl;
					}

					curAtt = curAtt->Next();
					continue;
				}

				// FIXME: Refactor build layers to be clearer
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
							std::cout << "\t\tAttribute " << currAttName << ": " << currAttValue << std::endl;
					}
				}
				else if (elementName == "onclick")
				{
					if (currAttName == "script")
					{
						tileData.SetStrAttribute(elementName, currAttValue);

						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t\tAttribute " << currAttName << ": " << currAttValue << std::endl;
					}
					else 
					{
						std::cout << "\t\tElement: " << elementName << " written but not used! " << std::endl;
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
						if (elementName == "sprite" && currAttName == "name")
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
	void Map::LoadModData(const char* path)
	{
		tinyxml2::XMLDocument doc;
		doc.LoadFile(path);
		tinyxml2::XMLElement* currElement = doc.FirstChildElement();

		while (currElement != nullptr) 
		{
			std::string texturePath = TEXTURE_PATH;
			TileData obj{};
			std::string empty = "";
			std::string currName = std::string(currElement->Name());

			if (currName == "EnvironmentSprite")
			{
				std::cout << "Environment sprite found!" << std::endl;

				// Only create one new sprite for the entire map texture
				// Create the sprite object

				if (envTex == nullptr) 
				{
					HandleAttributes(currElement, obj, texturePath);

					envTex = ge->CreateSpriteObject(texturePath.c_str());
					envTex->SetDimensions(sprSize.x, sprSize.y);
				}
				else 
				{
					std::cerr << "\tWARNING! Unable to load environment sprite, the sprite has already been loaded elsewhere." << std::endl;
				}
			}
			else if (currName == "SoundTrack")
			{
				std::cout << "Sountrack found!" << std::endl;
				std::string elementName = std::string(currElement->Name());

				auto curAtt = currElement->FirstAttribute();
				while (curAtt != nullptr)
				{
					if (std::string(curAtt->Name()) == "name") 
					{
						ge->GetAudio()->LoadMusic(std::string(curAtt->Value()).c_str());
						std::cout << "\t" << std::string(curAtt->Value()).c_str() << std::endl;
					}

					curAtt = curAtt->Next();
				}
			}
			else if (currName == "EnvironmentObject")
			{
				//std::cout << "Tile object found!" << std::endl;
				HandleObjElements(currElement, obj, empty);

				// Provide info for buildable tag and others
				tiles.push_back(obj);
			}
			else 
			{
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
			}

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

		// Load all the mods
		std::cout << "Loading mods..." << std::endl;

		for (const auto& file : std::filesystem::recursive_directory_iterator(path))
		{
			if (file.is_directory() || file.path().extension() != ".xml") continue;
			std::string modPath = path + file.path().filename().string();

			std::cout << "#\tLoading mod: " << modPath << std::endl;
			LoadModData(modPath.c_str());
		}

		std::cout << "Finished loading mods." << std::endl;
		std::cout << "Loading world..." << std::endl;


		// Create a list of empty objects to posative bias empty tiles
		std::vector<int> initObjects{ };
		for (unsigned char i = 0; i < 20; i++) initObjects.push_back(-1);

		// Find all the items that can be spawned at the creation of the world
		for (unsigned int i = 0; i < objects.size(); i++)
			if (objects[i].GetBoolAttribute(GROWABLE_TAG)) initObjects.push_back(i);

		// FIXME: Load old map data
		for (Uint32 i = 0; i < mapLength; i++)
		{
			mapLayers[i] = 0;
			objLayers[i] = initObjects[rand() % initObjects.size()];

			if (objLayers[i] > 0 && objects[objLayers[i]].GetBoolAttribute(GROWABLE_TAG))
			{
				const char GROW_INDEX = objects[objLayers[i]].GetIntAttribute(LENGTH_TAG) - 1; // Allow the modder to specify the index

				std::string growableName = GROWTH_INDEX + std::to_string(i);
				objects[objLayers[i]].SetIntAttribute(growableName, rand() % GROW_INDEX);
			}

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
		Uint64 i = y * width + x;

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

			if (objects[objLayers[i]].GetBoolAttribute(GROWABLE_TAG))
			{
				const char GROW_INDEX = objects[objLayers[i]].GetIntAttribute(LENGTH_TAG) - 1; // Allow the modder to specify the index

				std::string growableName = GROWTH_INDEX + std::to_string(i);
				int growableIndex = objects[objLayers[i]].GetIntAttribute(growableName);

				objSprites[sprIndex]->SetSpriteIndex(GROW_INDEX - growableIndex);
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
		// FIXME: Actually blur the region please
		DrawRegion(w, h, offX, offY);
	}

	void Map::UpdateObjects() 
	{
		for (Uint32 x = 0; x < width; x++)
		{
			for (Uint32 y = 0; y < height; y++)
			{
				Uint64 i = y * width + x;

				// Update items
				// FIXME: Move "objects" over to Objects with positions instead of being pure data in an array
				if (objLayers[i] >= 0)
				{
					// FIXME: Find a better way to update growables than one at a time
					if (objects[objLayers[i]].GetBoolAttribute(GROWABLE_TAG))
					{
						const char GROW_INDEX = objects[objLayers[i]].GetIntAttribute(LENGTH_TAG) - 1; // Allow the modder to specify the index

						std::string growableName = GROWTH_INDEX + std::to_string(i);
						int growableIndex = objects[objLayers[i]].GetIntAttribute(growableName);

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
				}
			}
		}
	}

	// ---------- Accessors  ---------------

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

	// --------- Mutators -----------------

	void Map::SetObject(Uint32 id, Sint32 index)
	{
		if (index >= 0 && objects[index].GetBoolAttribute(WORKABLE_ATT))
		{
			workables.push_back(id);
		}
		else
		{
			// FIXME: if finds workable in list, remove it
		}

		objLayers[id] = index;
	};

	void Map::SetTile(int id, Uint32 index)
	{

		// FIXME: GetType is returning a tile not an object
		mapLayers[id] = index;

		if (objLayers[id] >= 0) 
		{
			auto t = GetType(objLayers[id] + GetTileTypeCount());
			if (t.layer.length() > 0 && t.layer != GetType(mapLayers[id]).buildLayer)
				SetObject(id, -1); // FIXME: Provide a refund for items that cost money
			else std::cout << t.name << " is valid placement: " << t.buildLayer << " " << GetType(mapLayers[id]).layer << std::endl;
		}

		colMap[id] = GetType(index).GetBoolAttribute("collision");
	}
}