#pragma once
#ifndef GOBLINS_MAIN
#define GOBLINS_MAIN

#include "../GoblEngine.hpp"
#include "Map.h"

enum Scene : Uint8
{
	MainMenu = 0,
	Game = 1,
	Paused = 2,
};

class GoblinsMain : public gobl::GoblEngine
{
public:
	gobl::Sprite title;
	gobl::Sprite button;
	gobl::Sprite* mapTiles;

	gobl::Sprite sprite;
	gobl::Sprite highlightSprite;

	Map map;

	bool debugging = false;

	// FIXME: Provide a proper scene system
	Scene currScene = Scene::MainMenu;

	bool quittingApp = false;
	bool quitToMenu = false;

	int texturesLoaded = 0;
	const int texturesToLoad = 5;
	Uint32 tileTypeIndex = 0;

	void DrawButton(std::string buttonText, IntVec2 pos, bool& clicked)
	{
		button.SetPosition(pos);
		button.Draw();

		Color c = { 100, 100, 100 };
		if (button.Overlaps(Input().GetMouse().x, Input().GetMouse().y))
		{
			c.r = c.g = c.b = 0;

			if (Input().GetMouseButton(MOUSE_BUTTON::MB_LEFT))
			{
				clicked = true;
			}
		}

		DrawString(buttonText, button.GetPosition().x + (button.GetScale().x / 2) - 12 * buttonText.size(),
			button.GetPosition().y + (button.GetScale().y / 2) - 20, 50, c.r, c.g, c.b);
	}

	void DrawValidate(std::string prompt, bool& yes, bool& no) 
	{
		DrawOutlinedString(prompt, 300, 300, 50, 2);

		DrawButton("Confirm", { 200, 400 }, yes);
		DrawButton("Cancel", { 500, 400 }, no);
	}

	void DrawTileOptions() 
	{
		auto mapTexture = map.GetTexture();

		for (Uint32 i = 0; i < map.GetTileTypeCount(); i++)
		{
			mapTexture->SetSpriteIndex(i);
			mapTexture->SetPosition(800, 100 + (50 * i));
			mapTexture->SetScale(1.5f);

			if (mapTexture->Overlaps(Input().GetMouse())) 
			{
				mapTexture->SetScale(1.6f);

				if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = i;

				DrawString(map.GetLayerInfo(i), 850, 100 + (50 * i), 20);
			}

			mapTexture->Draw();
		}

		if (tileTypeIndex != -1) 
		{
			mapTexture->SetSpriteIndex(tileTypeIndex);
			mapTexture->SetPosition(790, 25);
			mapTexture->SetScale(2.0f);

			if (mapTexture->Overlaps(Input().GetMouse()))
			{
				mapTexture->SetScale(2.05f);

				if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = -1;
			}

			DrawString(map.GetLayerInfo(tileTypeIndex), 20, 800, 5);
			mapTexture->Draw();
		}
	}

	IntVec2 startCell;
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

		GetEngineLogo()->SetPosition(933, 637);

		CreateSpriteObject(button, "Sprites/slider.png");
		button.SetDimensions(20, 12);
		button.SetScale(8.0f);
		button.ModScale(static_cast<int>(0.2f * button.GetScale().x), 0);

		return true;
	}

	bool Update() override
	{
		auto mousePos = Input().GetMouse();

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
			map.Draw();

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
			map.Draw();

			title.Draw();
			GetEngineLogo()->SetAlpha(50);
			GetEngineLogo()->Draw();
			auto pos = GetEngineLogo()->GetPosition();

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

		DrawTileOptions();
		map.Draw();
		sprite.Draw();

		if (tileTypeIndex != -1) 
		{
			if (highlighting)
			{
				IntVec2 finalCell = map.GetTilePos(Input().GetMouse().x, Input().GetMouse().y);
				if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) highlighting = false;

				if (finalCell.x != -1 && finalCell.y != -1)
				{
					short lenX = finalCell.x - startCell.x;
					short lenY = finalCell.y - startCell.y;

					for (int y = 0; y < abs(lenY) + 1; y++)
					{
						short dirY = lenY > 0 ? y : -y;

						for (int x = 0; x < abs(lenX) + 1; x++)
						{
							short dirX = lenX > 0 ? x : -x;

							int id = map.GetTile(finalCell.x - dirX, finalCell.y - dirY);

							if (highlighting == true)
							{
								highlightSprite.SetPosition(map.GetTilePosition(id));
								highlightSprite.Draw();
							}
							else map.ChangeTile(id, tileTypeIndex);
						}
					}
				}

			}
			else
			{
				startCell = map.GetTilePos(Input().GetMouse().x, Input().GetMouse().y);
				int tileId = map.GetTile(startCell.x, startCell.y);

				if (tileId != -1)
				{
					if (debugging) DrawString(std::to_string(map.GetTileLayer(tileId)), 20, mousePos.x + 40, mousePos.y - 20);

					if (Input().GetMouseButtonDown(MOUSE_BUTTON::MB_LEFT)) highlighting = true;

					highlightSprite.SetPosition(map.GetTilePosition(tileId));
					highlightSprite.Draw();
				}
			}
		}


		if (Input().GetKeyPressed(SDLK_ESCAPE)) quitToMenu = true; // FIXME: Pause instead of just quitting

		if (Input().GetKeyPressed(SDLK_F3)) 
		{
			debugging = !debugging;
		}

		if (debugging)
		{
			DrawString(std::to_string(time.deltaTime), Input().GetMouse().x - 20, 
				Input().GetMouse().y + 20, 20, 0xFF, 0xFF, 0xFF);
			DrawString(std::to_string(time.GetFps()), Input().GetMouse().x - 20, 
				Input().GetMouse().y - 20, 20, 0xFF, 0xFF, 0xFF);
		}

		return true;
	}

	void Draw(gobl::GoblRenderer& renderer) override 
	{
		//renderer.SetPixel(Input().GetMouse().x, Input().GetMouse().y, 255, 255, 255);
		renderer.ClearScreen();
	}

	bool Exit() override
	{
		map.Destroy();

		return true;
	}
};

#endif // !GOBLINS_MAIN