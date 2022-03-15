#pragma once
#ifndef GOBLINS_MAIN
#define GOBLINS_MAIN

#include "../Engine.h"

class GoblinsMain : public Engine::GameEngine
{
public:
	Engine::SpriteRenderer sprite;
	Engine::SpriteRenderer sprite2;

private:
	void Init() override 
	{ 
		SetTitle("Goblins inc.");
	}

	bool Start() override
	{
		sprite = CreateSpriteObject();
		sprite.LoadTexture("testImg.png");

		// Does not work
		sprite2 = CreateSpriteObject("testImg.png");

		return true;
	}

	bool Update() override
	{
		sprite.Draw();
		sprite2.Draw();

		if (Input().GetMouseButton(MOUSE_BUTTON::MB_LEFT)) 
		{
			auto pos = Input().GetMouse();
			sprite.SetPosition(pos.x, pos.y);
		}

		DrawString(std::to_string(time.deltaTime), 20, Input().GetMouse().x - 20, Input().GetMouse().y + 20, 0xFF, 0xFF, 0xFF);

		return true;
	}

	void Draw(Engine::EngineRenderer& renderer) override 
	{
		//renderer.SetPixel(Input().GetMouse().x, Input().GetMouse().y, 255, 255, 255);
		renderer.QueueString(std::to_string(time.GetFps()), 20, Input().GetMouse().x - 20, Input().GetMouse().y - 20, 0xFF, 0xFF, 0xFF);
	}
};

#endif // !GOBLINS_MAIN