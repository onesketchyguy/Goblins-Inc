#pragma once
#ifndef MAP_H
#define MAP_H

#include "../Engine.h"

class Map 
{
private:
	Uint16 width = 0, height = 0;
	IntVec2 sprSize{ 0,0 };

	Engine::SpriteRenderer* sprites = nullptr;

public:
	Map() = default;
	Map(int w, int h, Engine::GameEngine* ge, const char* path)
	{
		width = w;
		height = h;

		sprites = new Engine::SpriteRenderer[w * h];
		for (Uint32 i = 0; i < width * height; i++)
		{
			sprites[i] = ge->CreateSpriteObject();
			sprites[i].LoadTexture(path);

			sprSize = sprites[i].GetDimensions();

			int x = i % width;
			int y = i / width;

			sprites[i].SetPosition(sprSize.y * x, sprSize.y * y);
			sprites[i].SetDimensions(sprSize.y, sprSize.y);
		}
	}

	void Draw() 
	{
		for (Uint32 i = 0; i < width * height; i++)
		{
			sprites[i].Draw();
		}
	}

	void ChangeTile(int id, int index)
	{
		sprites[id].SetSpriteIndex(index);
	}
	
	int GetTile(int x, int y) 
	{
		if (x > width * sprSize.x || x < 0) return -1;
		if (y > height * sprSize.y || y < 0) return -1;

		for (Uint32 i = 0; i < width * height; i++)
		{
			if (sprites[i].Overlaps(x, y)) return i;
		}
	}

	IntVec2 GetTilePosition(int id) 
	{
		return sprites[id].GetPosition();
	}
};

#endif // !MAP_H