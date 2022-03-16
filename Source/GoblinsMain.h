#pragma once
#ifndef GOBLINS_MAIN
#define GOBLINS_MAIN

#include "../Engine.h"
#include "Map.h"

class GoblinsMain : public Engine::GameEngine
{
public:
	Engine::SpriteRenderer title;
	Engine::SpriteRenderer button;

	Engine::SpriteRenderer sprite;
	Engine::SpriteRenderer highlightSprite;

	Map map;

	bool debugging = false;

	// FIXME: Provide a scene system
	bool mainMenu = true;

	bool quittingApp = false;
	bool quitToMenu = false;

	void DrawButton(std::string buttonText, IntVec2 pos, bool& clicked)
	{
		button.SetPosition(pos);
		button.Draw();

		Color c = { 100,100,100 };
		if (button.Overlaps(Input().GetMouse().x, Input().GetMouse().y))
		{
			c.r = c.g = c.b = 0;

			if (Input().GetMouseButton(MOUSE_BUTTON::MB_LEFT))
			{
				clicked = true;
			}
		}

		DrawString(buttonText, 50, button.GetPosition().x + (button.GetScale().x / 2) - 12 * buttonText.size(),
			button.GetPosition().y + (button.GetScale().y / 2) - 20, c.r, c.g, c.b);
	}

	void DrawValidate(std::string prompt, bool& yes, bool& no) 
	{
		DrawString(prompt, 50, 300, 300, 0xFF, 0xFF, 0xFF);

		DrawButton("Confirm", {300, 400}, yes);
		DrawButton("Cancel", { 500, 400 }, no);
	}

private:
	void Init() override { SetTitle("Goblins inc."); }

	bool Start() override
	{
		map = Map(24, 24, this, "Sprites/Enviroment.png");

		highlightSprite = CreateSpriteObject();
		highlightSprite.LoadTexture("Sprites/highlightTile.png");

		sprite = CreateSpriteObject();
		sprite.LoadTexture("Sprites/worker.png");

		title = CreateSpriteObject();
		title.LoadTexture("Sprites/Title_HighRes.png");
		title.SetScale(0.9);

		button = CreateSpriteObject();
		button.LoadTexture("Sprites/slider.png");
		button.SetDimensions(18, 12);
		button.SetSpriteIndex(1);

		button.SetScale(10);

		return true;
	}

	bool Update() override
	{
		if (quittingApp)
		{
			bool cancelButton = false;
			bool quitButton = false;
			DrawValidate("Exit to Windows?", quitButton, cancelButton);

			if (cancelButton) quittingApp = false;
			if (quitButton) return false;

			return true;
		}


		if (quitToMenu)
		{
			bool cancelButton = false;
			bool quitButton = false;
			DrawValidate("Quit to menu?", quitButton, cancelButton);

			if (cancelButton) quitToMenu = false;
			if (quitButton)
			{
				mainMenu = true;
				quitToMenu = false;
			}

			// FIXME: instead of not drawing anything we should draw the background but fuzzy
			return true;
		}

		if (mainMenu) 
		{
			title.Draw();

			bool startButton = false;
			bool quitButton = false;

			DrawButton("Start", { 0, 420 }, startButton);
			DrawButton("Quit", { 0, 540 }, quitButton);

			if (startButton) 
			{
				// FIXME: use scenes
				mainMenu = false;
			}

			if (quitButton) quittingApp = true;

			return true;
		}

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

		if (Input().GetKeyPressed(SDLK_ESCAPE))
		{
			// FIXME: use scenes
			quitToMenu = true;
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