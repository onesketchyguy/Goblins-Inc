#pragma once
#ifndef MAP_H
#define MAP_H

#include "../GoblEngine.hpp"
#include "../libs/tinyxml2.h"
using namespace tinyxml2;

bool MAP_DEBUG_VERBOSE = false;

class Map 
{
private:
	Uint16 width = 0, height = 0;
	Uint32 mapLength = 0;

	gobl::Sprite* envTex;

	std::map<int, std::string> envObjects{};

	int* mapLayers;
	Uint64 sprLength = 0;

private: // XML stuff
	void LoadMapModData(gobl::GoblEngine* ge, const char* path)
	{
		std::string texturePath = "Sprites/";
		std::vector<int> indexs{};
		IntVec2 sprSize{ 0,0 };

		XMLDocument doc;
		doc.LoadFile(path);

		auto current = doc.FirstChildElement();

		if (current != nullptr)
		{
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

					// Read each element of that mod object
					while (curModElement != nullptr)
					{
						if (MAP_DEBUG_VERBOSE)
							std::cout << "\t" << curModElement->Name() << " tag: " << std::endl;

						// Handle the sprite tag
						if (std::string(curModElement->Name()) == "sprite")
						{
							// Read attributes
							auto curAtt = curModElement->FirstAttribute();
							if (curAtt != nullptr)
							{
								while (curAtt != nullptr)
								{
									if (std::string(curAtt->Name()) == "i") {
										int sprIndex = std::stoi(curAtt->Value());

										if (MAP_DEBUG_VERBOSE)
											std::cout << "\t\tIndex attribute: " << curAtt->Name() << ", " << sprIndex << std::endl;
										envObjects[sprIndex] = title;
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
								std::cout << "ERROR: No sprite attributes provided!" << std::endl;
							}
						}

						// FIXME: Provide info for buildable tag and others

						// Move to next element
						curModElement = curModElement->NextSiblingElement();
					}
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

public: // Main map stuff
	Map() = default;
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
		for (Uint32 i = 0; i < mapLength; i++)
		{
			int x = i % width;
			int y = i / width;

			envTex->SetSpriteIndex(mapLayers[i]);
			envTex->SetPosition(envTex->GetScale().x * x, envTex->GetScale().y * y);
			envTex->Draw();
		}
	}

	void ChangeTile(int id, int index)
	{
		mapLayers[id] = index;
	}
	
	int GetTileLayer(int id)
	{
		return mapLayers[id];
	}

	bool Overlaps(int id, int x, int y)
	{
		auto scale = envTex->GetScale();
		//auto scale = _envTex.GetScale();

		int tileX = scale.x * (id % width);
		int tileY = scale.y * (id / width);

		return (y >= tileY && y <= tileY + scale.y) && (x >= tileX && x <= tileX + scale.y);
	}

	int GetTile(int x, int y) 
	{
		if (x > width * envTex->GetScale().x || x < 0) return -1;
		if (y > height * envTex->GetScale().y || y < 0) return -1;

		x -= x % envTex->GetScale().x;
		y -= y % envTex->GetScale().y;

		x /= envTex->GetScale().x;
		y /= envTex->GetScale().y;

		return y * width + x;
	}

	IntVec2 GetTilePosition(int id) 
	{
		return { envTex->GetScale().x * (id % width), envTex->GetScale().y * (id / width) };
	}
};

#endif // !MAP_H