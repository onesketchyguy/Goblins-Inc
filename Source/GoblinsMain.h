#pragma once
#ifndef GOBLINS_MAIN
#define GOBLINS_MAIN

#include "../Engine.h"
#include "Map.h"

class GoblinsMain : public Engine::GameEngine
{
public:
	Engine::SpriteRenderer sprite;
	Engine::SpriteRenderer highlightSprite;

	Map map;

	bool debugging = false;

private:
	void Init() override 
	{ 
		SetTitle("Goblins inc.");
	}

	bool Start() override
	{
		map = Map(24, 24, this, "Sprites/Enviroment.png");

		highlightSprite = CreateSpriteObject();
		highlightSprite.LoadTexture("Sprites/highlightTile.png");

		sprite = CreateSpriteObject();
		sprite.LoadTexture("Sprites/worker.png");

		return true;
	}

	bool Update() override
	{
		map.Draw();

		sprite.Draw();

		auto pos = Input().GetMouse();
		int tileId = map.GetTile(pos.x, pos.y);

		if (tileId != -1)
		{
			highlightSprite.SetPosition(map.GetTilePosition(tileId));
			highlightSprite.Draw();
		}

		if (Input().GetMouseButton(MOUSE_BUTTON::MB_LEFT)) 
		{
			if (tileId != -1) 
			{
				map.ChangeTile(tileId, 1);
			}
		}

		if (Input().GetKeyPressed(SDLK_F2)) 
		{
			debugging = !debugging;
		}

		if (debugging)
		{
			DrawString(std::to_string(time.deltaTime), 20, Input().GetMouse().x - 20, 
				Input().GetMouse().y + 20, 0xFF, 0xFF, 0xFF);
			DrawString(std::to_string(time.GetFps()), 20, Input().GetMouse().x - 20, 
				Input().GetMouse().y - 20, 0xFF, 0xFF, 0xFF);
		}

		return true;
	}

	void Draw(Engine::EngineRenderer& renderer) override 
	{
		//renderer.SetPixel(Input().GetMouse().x, Input().GetMouse().y, 255, 255, 255);
	}
};

#endif // !GOBLINS_MAIN