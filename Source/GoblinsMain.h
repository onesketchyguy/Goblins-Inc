#pragma once
#ifndef GOBLINS_MAIN
#define GOBLINS_MAIN

#include "../Engine.h"
#include "Map.h"

enum Scene : Uint8
{
	Splash = 0,
	MainMenu = 1,
	Game = 2,
	Paused = 3,
};

class GoblinsMain : public gobl::GoblEngine
{
public:
	gobl::SpriteRenderer title;
	gobl::SpriteRenderer button;

	gobl::SpriteRenderer sprite;
	gobl::SpriteRenderer highlightSprite;

	Map map;

	bool debugging = false;

	// FIXME: Provide a proper scene system
	Scene currScene = Scene::MainMenu;

	bool quittingApp = false;
	bool quitToMenu = false;

	int texturesLoaded = 0;
	const int texturesToLoad = 5;

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

	int startX, startY;
	bool highlighting = false;

private:
	void Init() override { SetTitle("Goblins inc."); }

	bool Start() override
	{
		map = Map(this, 24, 24, "Mods/Environment.xml");

		CreateSpriteObject(highlightSprite, "Sprites/highlightTile.png");
		CreateSpriteObject(sprite, "Sprites/worker.png");
		CreateSpriteObject(title, "Sprites/Title_HighRes.png");
		title.SetScale(0.9f);

		CreateSpriteObject(button, "Sprites/slider.png");
		button.LoadTexture("Sprites/slider.png");
		button.SetDimensions(18, 12);
		button.SetSpriteIndex(1);

		button.SetScale(10);

		return true;
	}

	bool Update() override
	{
		map.Draw();

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
				// FIXME: Unload non-menu content
				currScene = Scene::MainMenu;
				quitToMenu = false;
			}

			// FIXME: instead of not drawing anything we should draw the background but fuzzy
			return true;
		}

		if (currScene == Scene::MainMenu)
		{
			title.Draw();

			bool startButton = false;
			bool quitButton = false;

			DrawButton("Start", { 0, 420 }, startButton);
			DrawButton("Quit", { 0, 540 }, quitButton);

			if (startButton) 
			{
				// FIXME: Unload menu content
				currScene = Scene::Game;
				Input().SetEatInput(10); // Eat 10 frames of input
			}

			if (quitButton) quittingApp = true;

			return true;
		}

		sprite.Draw();

		auto pos = Input().GetMouse();
		int tileId = map.GetTile(pos.x, pos.y);

		if (tileId != -1)
		{
			if (debugging) DrawString(std::to_string(map.GetTileLayer(tileId)), 20, pos.x + 40, pos.y - 20);

			if (highlighting) 
			{
				int lenX = Input().GetMouse().x - startX;
				int lenY = Input().GetMouse().y - startY;

				for (int y = 0; y < abs(lenY); y++)
				{
					for (int x = 0; x < abs(lenX); x++)
					{
						int dirX = lenX > 0 ? x : -x;
						int dirY = lenY > 0 ? y : -y;

						int id = map.GetTile(pos.x - dirX, pos.y - dirY);
						highlightSprite.SetPosition(map.GetTilePosition(id));
						highlightSprite.Draw();
					}
				}
			}
			else 
			{
				highlightSprite.SetPosition(map.GetTilePosition(tileId));
				highlightSprite.Draw();
			}

			if (Input().GetMouseButtonDown(MOUSE_BUTTON::MB_LEFT))
			{
				startX = Input().GetMouse().x;
				startY = Input().GetMouse().y;

				highlighting = true;
			}

			if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT))
			{
				int lenX = Input().GetMouse().x - startX;
				int lenY = Input().GetMouse().y - startY;

				for (int y = 0; y < abs(lenY); y++)
				{
					for (int x = 0; x < abs(lenX); x++)
					{
						int dirX = lenX > 0 ? x : -x;
						int dirY = lenY > 0 ? y : -y;

						int id = map.GetTile(pos.x - dirX, pos.y - dirY);
						map.ChangeTile(id, 1);
					}
				}

				highlighting = false;
			}
		}

		if (Input().GetKeyPressed(SDLK_ESCAPE)) quitToMenu = true; // FIXME: Pause instead of just quitting

		if (Input().GetKeyPressed(SDLK_F3)) 
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

	void Draw(gobl::GoblRenderer& renderer) override 
	{
		//renderer.SetPixel(Input().GetMouse().x, Input().GetMouse().y, 255, 255, 255);
		renderer.ClearScreen();
	}
};

#endif // !GOBLINS_MAIN