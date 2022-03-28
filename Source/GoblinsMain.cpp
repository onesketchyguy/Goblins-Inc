#include "GoblinsMain.hpp"

using namespace gobl;

IntVec2 viewArea{};
IntVec2 startMouse{};
IntVec2 startCell{};

// Input
bool GetMouseCam(bool handEmpty)
{
	return InputManager::GetMouseButton(MOUSE_BUTTON::MB_MIDDLE) || 
		InputManager::GetMouseButton(MOUSE_BUTTON::MB_RIGHT) && handEmpty ||
		InputManager::GetMouseButton(MOUSE_BUTTON::MB_LEFT | MOUSE_BUTTON::MB_RIGHT) ||
		InputManager::GetKey(SDLK_SPACE) && InputManager::instance->GetMouseButton(MOUSE_BUTTON::MB_LEFT);
}

// Draw UI
void GoblinsMain::DrawButton(std::string buttonText, IntVec2 pos, bool& clicked)
{
	button.SetPosition(pos);
	button.Draw();

	Color c = { 100, 100, 100 };
	if (button.Overlaps(InputManager::GetMouse().x, InputManager::GetMouse().y))
	{
		c.r = c.g = c.b = 0;

		if (InputManager::GetMouseButton(MOUSE_BUTTON::MB_LEFT))
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

bool GoblinsMain::DrawTileOptions()
{
	auto mapTexture = map.GetTexture();
	Uint32 highLighting = -1;

	int x = 800;
	int y = 100;

	for (Uint32 i = 0; i < map.GetTileTypeCount(); i++)
	{
		mapTexture->SetPosition(x, y + (50 * i));
		mapTexture->SetScale(1.6f);

		if (mapTexture->Overlaps(InputManager::GetMouse()))
		{
			if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = i;

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
				// Mouse over this
				mapTexture->SetPosition(x, y - 5 + (50 * i));
				mapTexture->SetScale(1.6f);
				mapTexture->SetColorMod({ 200, 200, 200, 255 });

				DrawString(map.GetTypeName(i), x + 80, y + 10 + (50 * i));

				if (debugging)
				{
					DrawString(map.GetTypeBuildable(i), x + 80, y + 25 + (50 * i));
					DrawString(map.GetTypeLayer(i), x + 80, y + 50 + (50 * i));
				}
			}
			else
			{
				mapTexture->SetColorMod({ 100, 100, 100, 255 });
				mapTexture->SetPosition(x + 5, y + (50 * i));
				mapTexture->SetScale(1.2f);
			}
		}
		else
		{
			mapTexture->SetColorMod({ 150, 150, 150, 150 });
			mapTexture->SetPosition(x, y + (50 * i));
			mapTexture->SetScale(1.4f);
		}


		mapTexture->Draw();
	}

	if (tileTypeIndex >= 0 && tileTypeIndex < map.GetTileTypeCount())
	{
		DrawString(map.GetTypeName(tileTypeIndex), x, 5);

		if (debugging)
		{
			DrawString(map.GetTypeBuildable(tileTypeIndex), x + 50, 25);
			DrawString(map.GetTypeLayer(tileTypeIndex), x + 50, 45);
		}

		mapTexture->SetSpriteIndex(map.GetTypeSprite(tileTypeIndex));

		mapTexture->SetPosition(x - 12, 24);
		mapTexture->SetScale(2.1f);
		mapTexture->SetColorMod({ 0, 0, 0, 255 });
		mapTexture->Draw();

		if (mapTexture->Overlaps(InputManager::GetMouse()))
		{
			mapTexture->SetScale(2.05f);

			if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = -1;
		}

		mapTexture->SetPosition(x - 10, 25);
		mapTexture->SetScale(2.0f);
		mapTexture->SetColorMod({ 255, 255, 255, 255 });
		mapTexture->Draw();
	}

	if (debugging) 
	{
		DrawOutlinedString(std::to_string(tileTypeIndex), 0, 40, 20, 3U);
		if (highLighting != -1)
			DrawOutlinedString(std::to_string(highLighting), 20, 40, 20, 3U, 255U, 255U, 0U);
	}

	map.ResetTexture();

	return highLighting != -1;
}

// Basic function
bool GoblinsMain::Start()
{
	map = MAP::Map(this, 64, 64, "Mods/Environment.xml");

	CreateSpriteObject(highlightSprite, "Sprites/highlightTile.png");
	highlightSprite.SetColorMod({ 0, 0, 0 });
	CreateSpriteObject(sprite, "Sprites/worker.png");
	CreateSpriteObject(title, "Sprites/Title_HighRes.png");
	title.SetScale(0.9f);

	GetEngineLogo()->SetPosition(933, 637);

	CreateSpriteObject(button, "Sprites/slider.png");
	button.SetDimensions(20, 12);
	button.SetScale(8.0f);
	button.ModScale(static_cast<int>(0.2f * button.GetScale().x), 0);

	testSwitch.SetupSprite(this, "Sprites/slider.png");
	testSwitch.GetSprite().SetScale(4.0f);
	testSwitch.SetPosition({ 900, 50 });

	// Calculate viewarea
	viewArea = map.GetTileMapPos(GetScreenWidth(), GetScreenHeight());
	viewArea.x += 1;
	viewArea.y += 2;

	return true;
}

Color validPlacementColor = { 0, 0, 0, 150 };
Color invalidPlacementColor = { 0, 0, 0, 75 };

void GoblinsMain::DrawWorld(bool blur)
{
	IntVec2 cell = map.GetClosestTileMapPos(static_cast<int>(GetCamera().x), static_cast<int>(GetCamera().y));

	if (blur == false) map.DrawRegion(viewArea.x, viewArea.y, cell.x, cell.y);
	else map.BlurDrawRegion(viewArea.x, viewArea.y, cell.x, cell.y);
}

bool GoblinsMain::Update()
{
	auto mousePos = InputManager::GetMouse();
	auto worldMouse = Vec2{ mousePos.x + GetCamera().x, mousePos.y + GetCamera().y };

	if (quitToMenu || currScene == Scene::MainMenu)
	{
		// BLUR
		DrawWorld(true);
	}
	else
	{
		// UNBLUR
		DrawWorld();
	}

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
			InputManager::instance->SetEatInput(10); // Eat 10 frames of input
		}

		if (quitButton) quittingApp = true;

		return true;
	}

	sprite.DrawRelative(GetCameraObject());

	if (testSwitch.GetActive() && DrawTileOptions() == false)
	{
		if (tileTypeIndex != -1)
		{
			if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_RIGHT))
			{
				highlighting = false;
				tileTypeIndex = -1;

				return true;
			}

			if (highlighting == false)
			{
				startCell = map.GetTileMapPos(static_cast<int>(worldMouse.x), static_cast<int>(worldMouse.y));
			}


			IntVec2 finalCell = map.GetTileMapPos(static_cast<int>(worldMouse.x), static_cast<int>(worldMouse.y));
			if (InputManager::GetMouseButtonDown(MOUSE_BUTTON::MB_LEFT)) highlighting = true;
			if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) highlighting = false;

			if (finalCell.x != -1 && finalCell.y != -1)
			{
				short lenX = 0;
				short lenY = 0;

				if (map.GetTypeMultiPlace(tileTypeIndex) == true)
				{
					lenX = finalCell.x - startCell.x;
					lenY = finalCell.y - startCell.y;

					if (map.GetTypeLinear(tileTypeIndex) == true)
					{
						if (abs(lenX) > abs(lenY)) lenY = 0; else lenX = 0;
					}
				}

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
							if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT) == false)
								highlightSprite.SetColorMod(validPlacementColor);
							else map.ChangeTile(id, tileTypeIndex);
						}
						else highlightSprite.SetColorMod(invalidPlacementColor);

						highlightSprite.SetPosition(map.GetTilePos(id));
						highlightSprite.DrawRelative(GetCameraObject());
					}
				}
			}
		}
	}
	else tileTypeIndex = -1;

	// Draw UI
	testSwitch.Update();

	if (InputManager::GetKeyPressed(SDLK_ESCAPE)) quitToMenu = true; // FIXME: Pause instead of just quitting

	float spd = 150.0f;
	Vec2 camMove = {};

	if (GetMouseCam(tileTypeIndex == -1))
	{
		camMove.x -= mousePos.x - startMouse.x;
		camMove.y -= mousePos.y - startMouse.y;
	}

	startMouse = mousePos;

	if (InputManager::GetKey(SDLK_RIGHT)) camMove.x += spd * time.fDeltaTime;
	if (InputManager::GetKey(SDLK_LEFT)) camMove.x -= spd * time.fDeltaTime;
	if (InputManager::GetKey(SDLK_UP)) camMove.y -= spd * time.fDeltaTime;
	if (InputManager::GetKey(SDLK_DOWN)) camMove.y += spd * time.fDeltaTime;

	MoveCamera(camMove.x, camMove.y);
	//MoveZoom(Input().GetMouseWheel() * time.deltaTime);

	return true;
}