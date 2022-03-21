#include "GoblinsMain.hpp"

using namespace gobl;

// Input
bool GetMouseCam()
{
	return InputManager::instance->GetMouseButton(MOUSE_BUTTON::MB_MIDDLE) ||
		InputManager::instance->GetMouseButton(MOUSE_BUTTON::MB_LEFT | MOUSE_BUTTON::MB_RIGHT) ||
		InputManager::instance->GetKey(SDLK_SPACE) && InputManager::instance->GetMouseButton(MOUSE_BUTTON::MB_LEFT);
}

// Draw UI
void GoblinsMain::DrawButton(std::string buttonText, IntVec2 pos, bool& clicked)
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

void GoblinsMain::DrawValidate(std::string prompt, bool& yes, bool& no)
{
	DrawOutlinedString(prompt, 300, 300, 50, 4);

	DrawButton("Confirm", { 200, 400 }, yes);
	DrawButton("Cancel", { 500, 400 }, no);
}

void GoblinsMain::DrawTileOptions()
{
	auto mapTexture = map.GetTexture();
	mapTexture->SetUseCamera(false);
	Uint32 highLighting = -1;

	int x = 800;
	int y = 100;

	for (Uint32 i = 0; i < map.GetTileTypeCount(); i++)
	{
		mapTexture->SetPosition(x, y + (50 * i));
		mapTexture->SetScale(1.6f);

		if (mapTexture->Overlaps(Input().GetMouse()))
		{
			if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = i;

			highLighting = i;
		}
	}

	for (Uint32 i = 0; i < map.GetTileTypeCount(); i++)
	{
		int s = map.GetTypeSprite(i);
		mapTexture->SetSpriteIndex(s);

		if (highLighting != -1)
		{
			if (highLighting == i)
			{
				mapTexture->SetPosition(x, y - 5 + (50 * i));
				mapTexture->SetScale(1.6f);

				DrawString(map.GetTypeName(i), x + 80, y + 10 + (50 * i));

				if (debugging)
				{
					DrawString(map.GetTypeBuildable(i), x + 80, y + 15 + (50 * i));
					DrawString(map.GetTypeLayer(i), x + 80, y + 30 + (50 * i));
				}
			}
			else
			{
				mapTexture->SetPosition(x + 5, y + (50 * i));
				mapTexture->SetScale(1.2f);
			}
		}
		else
		{
			mapTexture->SetPosition(x, y + (50 * i));
			mapTexture->SetScale(1.4f);
		}

		mapTexture->Draw();
	}

	if (tileTypeIndex != -1)
	{
		mapTexture->SetSpriteIndex(map.GetTypeSprite(tileTypeIndex));
		mapTexture->SetPosition(x - 10, 25);
		mapTexture->SetScale(2.0f);

		if (mapTexture->Overlaps(Input().GetMouse()))
		{
			mapTexture->SetScale(2.05f);

			if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = -1;
		}

		DrawString(map.GetTypeName(tileTypeIndex), x, 5);

		if (debugging)
		{
			DrawString(map.GetTypeBuildable(tileTypeIndex), x + 50, 25);
			DrawString(map.GetTypeLayer(tileTypeIndex), x + 50, 45);
		}

		mapTexture->Draw();
	}

	mapTexture->SetUseCamera(true);
}

// Basic function
bool GoblinsMain::Start()
{
	map = MAP::Map(this, 24, 24, "Mods/Environment.xml");

	CreateSpriteObject(highlightSprite, "Sprites/highlightTile.png", true);
	highlightSprite.SetColorMod({ 0, 0, 0 });
	CreateSpriteObject(sprite, "Sprites/worker.png", true);
	CreateSpriteObject(title, "Sprites/Title_HighRes.png");
	title.SetScale(0.9f);

	GetEngineLogo()->SetPosition(933, 637);

	CreateSpriteObject(button, "Sprites/slider.png");
	button.SetDimensions(20, 12);
	button.SetScale(8.0f);
	button.ModScale(static_cast<int>(0.2f * button.GetScale().x), 0);

	return true;
}

bool GoblinsMain::Update()
{
	if (Input().GetKeyPressed(SDLK_F3)) debugging = !debugging;

	auto mousePos = Input().GetMouse();
	auto worldMouse = Vec2{ mousePos.x + GetCamera().x, mousePos.y + GetCamera().y };

	if (debugging) 
	{
		DrawString("y" + std::to_string(mousePos.y), -90 + mousePos.x + 20, -90 + mousePos.y);
		DrawString("x" + std::to_string(mousePos.x), -90 + mousePos.x, -90 + mousePos.y - 20);
	}

	if (quitToMenu || currScene == Scene::MainMenu)
	{
		// BLUR
	}
	else
	{
		// UNBLUR
	}


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

	sprite.Draw();

	if (tileTypeIndex != -1)
	{
		if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_RIGHT)) 
		{
			highlighting = false;
			tileTypeIndex = -1;

			return true;
		}

		if (highlighting)
		{
			IntVec2 finalCell = map.GetTileMapPos(static_cast<int>(worldMouse.x), static_cast<int>(worldMouse.y));
			if (Input().GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) highlighting = false;

			if (finalCell.x != -1 && finalCell.y != -1)
			{
				short lenX = 0;
				short lenY = 0;

				if (map.GetTypeMultiPlace(tileTypeIndex) == true)
				{
					lenX = finalCell.x - startCell.x;
					lenY = finalCell.y - startCell.y;

					if (map.GetTypeLinear(tileTypeIndex) == true)
						if (abs(lenX) > abs(lenY)) lenY = 0; else lenX = 0;
				}

				highlightSprite.SetAlpha(255);

				for (int y = 0; y < abs(lenY) + 1; y++)
				{
					short dirY = lenY > 0 ? y : -y;

					for (int x = 0; x < abs(lenX) + 1; x++)
					{
						short dirX = lenX > 0 ? x : -x;

						int id = map.GetTile(finalCell.x - dirX, finalCell.y - dirY);
						Uint32 tileLayer = map.GetTileLayer(id);

						if (map.GetTypeBuildable(tileLayer) == map.GetTypeLayer(tileTypeIndex))
						{
							if (highlighting == true)
							{
								highlightSprite.SetPosition(map.GetTilePos(id));
								highlightSprite.Draw();
							}
							else map.ChangeTile(id, tileTypeIndex);
						}
						else highlightSprite.SetAlpha(150);
					}
				}
			}

		}
		else
		{
			startCell = map.GetTileMapPos(static_cast<int>(worldMouse.x), static_cast<int>(worldMouse.y));
			int tileId = map.GetTile(startCell.x, startCell.y);

			if (tileId != -1)
			{
				Uint32 tileLayer = map.GetTileLayer(tileId);

				if (debugging)
				{
					DrawString(std::to_string(map.GetTileLayer(tileId)), mousePos.x + 40, mousePos.y - 20);
					DrawString(map.GetTypeBuildable(map.GetTileLayer(tileId)), mousePos.x + 40, mousePos.y - 40);
					DrawString(map.GetTypeLayer(map.GetTileLayer(tileId)), mousePos.x + 40, mousePos.y - 60);
				}

				if (Input().GetMouseButtonDown(MOUSE_BUTTON::MB_LEFT)) highlighting = true;

				if (map.GetTypeBuildable(tileLayer) == map.GetTypeLayer(tileTypeIndex))
					highlightSprite.SetAlpha(0xFF);
				else highlightSprite.SetAlpha(50);

				highlightSprite.SetPosition(map.GetTilePos(tileId));
				highlightSprite.Draw();
			}
		}
	}

	if (Input().GetKeyPressed(SDLK_ESCAPE)) quitToMenu = true; // FIXME: Pause instead of just quitting

	// Draw UI
	DrawTileOptions();

	float spd = 150.0f;
	Vec2 camMove = {};

	if (GetMouseCam())
	{
		camMove.x -= mousePos.x - startMouse.x;
		camMove.y -= mousePos.y - startMouse.y;
	}

	startMouse = mousePos;

	if (Input().GetKey(SDLK_RIGHT)) camMove.x += spd * time.fDeltaTime;
	if (Input().GetKey(SDLK_LEFT)) camMove.x -= spd * time.fDeltaTime;
	if (Input().GetKey(SDLK_UP)) camMove.y -= spd * time.fDeltaTime;
	if (Input().GetKey(SDLK_DOWN)) camMove.y += spd * time.fDeltaTime;

	MoveCamera(camMove.x, camMove.y);
	//MoveZoom(Input().GetMouseWheel() * time.deltaTime);

	if (debugging)
	{
		DrawString(std::to_string(time.deltaTime), 0, 0);
		DrawString(std::to_string(time.GetFps()), 0, 20);
	}

	return true;
}