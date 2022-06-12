#include "GoblinsMain.hpp"
#include "GoblinObj.hpp"

using namespace gobl;

long long GoblinsMain::money = 0;

IntVec2 viewArea{};
IntVec2 startMouse{};
IntVec2 startCell{};
bool highlighting = false;
Uint32 tileTypeIndex = -1;

Color validPlacementColor = { 0, 0, 0, 150 };
Color invalidPlacementColor = { 0, 0, 0, 75 };

std::vector<GoblinObj*> goblins{};

void GoblinsMain::HireGoblin() 
{
	GoblinObj* goblin = new GoblinObj();
	goblin->GenerateSprite(this);
	goblin->AssignMap(&map);

	goblins.push_back(goblin);
}

// FIXME: Make a time manager
unsigned char hour = 0;

// Tools
bool CanPlace(const MAP::TileData& a, const MAP::TileData& b) { return a.buildLayer != "" && a.buildLayer == b.layer; }

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

void GoblinsMain::DrawSelectedObject()
{
	if (tileTypeIndex >= 0 && tileTypeIndex < map.GetTileTypeCount() + map.GetObjectCount())
	{
		int x = 800;

		map.GetTileTexture()->SetSpriteIndex(0);
		map.GetTileTexture()->SetColorMod(Color{ 0, 0, 0, 150 })->SetPosition(x - 12, 24)->SetScale(2.1f)->Draw();

		if (tileTypeIndex < map.GetTileTypeCount()) 
		{
			DrawString(map.GetType(tileTypeIndex).name, x, 5);
			if (debugging)
			{
				DrawString(map.GetType(tileTypeIndex).buildLayer, x + 50, 25);
				DrawString(map.GetType(tileTypeIndex).layer, x + 50, 45);
			}

			map.GetTileTexture()->SetSpriteIndex(map.GetType(tileTypeIndex).GetIntAttribute(MAP::SPRITE_ATT));

			if (map.GetTileTexture()->Overlaps(InputManager::GetMouse()))
			{
				map.GetTileTexture()->SetScale(2.05f);

				if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = -1;
			}

			map.GetTileTexture()->SetColorMod(Color::WHITE)->SetPosition(x - 10, 25)->SetScale(2.0f)->Draw();
		}
		else
		{
			DrawString(map.GetType(tileTypeIndex).name, x, 5);
			DrawString("$" + std::to_string(map.GetType(tileTypeIndex).GetIntAttribute(MAP::PRICE_ATT)),
				x - 50, 20);
			if (debugging)
			{
				DrawString(map.GetType(tileTypeIndex).buildLayer, x + 50, 25);
				DrawString(map.GetType(tileTypeIndex).layer, x + 50, 45);
			}

			Uint32 texIndex = tileTypeIndex - map.GetTileTypeCount();
			map.GetTexture(texIndex)->SetPosition(x - 10, 25)->SetScale(2.0f);

			if (map.GetTexture(texIndex)->Overlaps(InputManager::GetMouse()))
			{
				if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) tileTypeIndex = -1;
			}

			map.GetTexture(texIndex)->SetColorMod(Color::WHITE)->Draw();
		}
	}
}

bool GoblinsMain::DrawObjectOptions() 
{
	Uint32 objectLength = map.GetObjectCount();
	Uint32 highLighting = -1;

	int x = 850;
	int y = 100;

	for (Uint32 i = 0; i < objectLength; i++)
	{
		// Allow highlighting of each item
		map.GetTexture(i)->SetPosition(x, y + (50 * i))->SetScale(1.5f);

		if (map.GetTexture(i)->Overlaps(InputManager::GetMouse())) 
		{
			highLighting = i;

			if (InputManager::GetMouseButtonUp(MB_LEFT)) tileTypeIndex = map.GetTileTypeCount() + i;
		}
	}

	for (Uint32 i = 0; i < objectLength; i++)
	{
		Uint8 mouseOver = highLighting == -1 ? 0 : (highLighting == i ? 1 : 2);

		// Draw each item
		auto tex = map.GetTexture(i);
		tex->SetPosition(x, y + (50 * i));

		switch (mouseOver)
		{
		case 0:
			tex->SetScale(1.3f)->SetColorMod(Color{ 200, 200, 200, 200 });
			break;
		case 1:
			tex->SetScale(1.5f)->SetColorMod(Color{ 255, 255, 255, 255 });
			break;
		case 2:
			tex->SetScale(1.25f)->SetColorMod(Color{ 150, 150, 150, 150 });
			break;
		default:
			break;
		}

		tex->Draw();
	}

	map.ResetTexture();

	return highLighting != -1;
}

bool GoblinsMain::DrawTileOptions()
{
	auto mapTexture = map.GetTileTexture();
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
		int s = map.GetType(i).GetIntAttribute(MAP::SPRITE_ATT);
		mapTexture->SetSpriteIndex(s);

		if (highLighting != -1)
		{
			if (highLighting == i)
			{
				// Mouse over this
				mapTexture->SetColorMod({ 200, 200, 200, 255 })->SetPosition(x, y - 5 + (50 * i))->SetScale(1.6f);

				DrawString(map.GetType(i).name, x + 80, y + 10 + (50 * i));

				if (debugging)
				{
					DrawString(map.GetType(i).buildLayer, x + 80, y + 25 + (50 * i));
					DrawString(map.GetType(i).layer, x + 80, y + 50 + (50 * i));
				}
			}
			else mapTexture->SetColorMod({ 100, 100, 100, 255 })->SetPosition(x + 5, y + (50 * i))->SetScale(1.2f);
		}
		else mapTexture->SetColorMod({ 150, 150, 150, 150 })->SetPosition(x, y + (50 * i))->SetScale(1.4f);

		mapTexture->Draw();
	}

	if (debugging) 
	{
		DrawOutlinedString(std::to_string(tileTypeIndex), 0, 40, 20, 3U);
		if (highLighting != -1)
			DrawOutlinedString(std::to_string(highLighting), 20, 40, 20, 3U, 255U, 255U, 0U);
	}

	DrawSelectedObject();

	map.ResetTexture();

	return highLighting != -1 || DrawObjectOptions();
}

void GoblinsMain::HandlePlaceItems()
{
	Vec2 worldMouse = GetCamera() + InputManager::GetMouse();

	if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_RIGHT))
	{
		highlighting = false;
		tileTypeIndex = -1;

		return;
	}

	if (highlighting == false)
		startCell = map.GetTileMapPos(static_cast<int>(worldMouse.x), static_cast<int>(worldMouse.y));


	IntVec2 finalCell = map.GetTileMapPos(static_cast<int>(worldMouse.x), static_cast<int>(worldMouse.y));
	if (InputManager::GetMouseButtonDown(MOUSE_BUTTON::MB_LEFT)) highlighting = true;
	if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) highlighting = false;

	if (finalCell.x != -1 && finalCell.y != -1)
	{
		short lenX = 0;
		short lenY = 0;

		if (tileTypeIndex < map.GetTileTypeCount())
		{
			// Place tiles
			if (map.GetType(tileTypeIndex).GetBoolAttribute(MAP::MULTI_PLACE_ATT) == true)
			{
				lenX = finalCell.x - startCell.x;
				lenY = finalCell.y - startCell.y;

				if (map.GetType(tileTypeIndex).GetBoolAttribute(MAP::LINEAR_ATT) == true)
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

					if (CanPlace(map.GetType(tileLayer), map.GetType(tileTypeIndex)))
					{
						if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT) == false)
							highlightSprite.SetColorMod(validPlacementColor);
						else map.SetTile(id, tileTypeIndex);
					}
					else highlightSprite.SetColorMod(invalidPlacementColor);

					highlightSprite.SetPosition(map.GetTilePos(id));
					highlightSprite.DrawRelative(GetCameraObject());
				}
			}
		}
		else
		{
			// Place items

			int id = map.GetTile(finalCell.x, finalCell.y);
			highlightSprite.SetPosition(map.GetTilePos(id));

			Uint32 index = tileTypeIndex - map.GetTileTypeCount();

			if (CanPlace(map.GetType(map.GetTileLayer(id)), map.GetType(tileTypeIndex)))
			{
				if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT) == false)
				{
					highlightSprite.SetColorMod(validPlacementColor);
					map.GetTexture(index)->SetColorMod(Color::WHITE);
				}
				else map.SetObject(id, index);
			}
			else
			{
				highlightSprite.SetColorMod(invalidPlacementColor);
				map.GetTexture(index)->SetColorMod(Color::RED);
			}

			map.GetTexture(index)->SetPosition(map.GetTilePos(id))->DrawRelative(GetCameraObject());
			highlightSprite.DrawRelative(GetCameraObject());
		}
	}
}

void GoblinsMain::HandlePickupItems() 
{
	Vec2 worldMouse = GetCamera() + InputManager::GetMouse();

	if (highlighting == true) return;

	if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT))
	{
		IntVec2 finalCell = map.GetTileMapPos(static_cast<int>(worldMouse.x), static_cast<int>(worldMouse.y));

		if (finalCell.x != -1 && finalCell.y != -1) 
		{
			// Clicked on a tile

			int id = map.GetTile(finalCell.x, finalCell.y);
			auto tile = map.GetType(map.GetObjectLayer(id) + map.GetTileTypeCount());
			bool pickup = tile.GetBoolAttribute("pickup");

			//std::cout << tile.name << " id(" << map.GetObjectLayer(id) << ")" << " Pickup: " << (pickup ? "true" : "false") << std::endl;

			// If tile is a pickup, delete it and add it to the inventory
			if (pickup) 
			{
				map.SetObject(id, -1);

				// FIXME: Add item to inventory
				// FIXME: Run an interaction script
			}

		}
	}
}

// Basic function
bool GoblinsMain::Start()
{
	map = MAP::Map(this, 64, 64, "Mods/");
	map.UpdateObjects();

	CreateSpriteObject(highlightSprite, "Sprites/highlightTile.png");
	highlightSprite.SetColorMod(Color::BLACK);
	CreateSpriteObject(title, "Sprites/Title_HighRes.png");
	title.SetScale(0.9f);

	CreateSpriteObject(moneySprite, "Sprites/moneySign.png");
	moneySprite.SetPosition(0, 5);

	CreateSpriteObject(testSprite, "Sprites/MissingTexture.png");

	GetEngineLogo()->SetPosition(GetScreenWidth() - GetEngineLogo()->GetDimensions().x, 
		GetScreenHeight() - GetEngineLogo()->GetDimensions().y);

	CreateSpriteObject(button, "Sprites/slider.png");
	button.SetDimensions(20, 12);
	button.SetScale(8.0f);
	button.ModScale(static_cast<int>(0.2f * button.GetScale().x), 0);

	testSwitch.SetupSprite(this, "Sprites/slider.png");
	testSwitch.GetSprite().SetScale(4.0f);
	testSwitch.SetPosition({ 900, 50 });

	CreateSpriteObject(hireNewGoblin, "Sprites/worker.png");
	hireNewGoblin.SetPosition(GetScreenWidth() - 64, GetScreenHeight() - 64);
	hireNewGoblin.SetScale(2.0f);
	hireNewGoblin.SetAlpha(100);

	// Calculate viewarea
	viewArea = map.GetTileMapPos(GetScreenWidth(), GetScreenHeight());
	viewArea.x += 1;
	viewArea.y += 2;

	// Load audio
	GetAudio()->LoadMusic("Sounds/Music/hopeblooming.wav");
	GetAudio()->PlayMusic();

	return true;
}

void GoblinsMain::DrawWorld(bool blur)
{
	IntVec2 cell = map.GetClosestTileMapPos(static_cast<int>(GetCamera().x), static_cast<int>(GetCamera().y));

	if (blur == false) map.DrawRegion(viewArea.x, viewArea.y, cell.x, cell.y);
	else map.BlurDrawRegion(viewArea.x, viewArea.y, cell.x, cell.y);

	//testSprite.SetPosition(map.GetEmptyWorkable(-1));
	//testSprite.DrawRelative(GetCameraObject());
}

bool GoblinsMain::Update()
{
	auto mousePos = InputManager::GetMouse();
	Vec2 worldMouse = GetCamera() + InputManager::GetMouse();

	bool blur = quitToMenu || currScene == Scene::MainMenu;
	DrawWorld(blur);

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

	HandlePickupItems();

	// FIXME: Move map.UpdateObjects() to the end or start of a day
	// DEBUG: This should update the map objects
	if (hour >= 250)
	{
		map.UpdateObjects();
		hour = 0;
	}

	if (InputManager::GetMouseButtonUp(1)) {
		GetAudio()->PlaySound("Sounds/Blop.wav");
	}

	// FIXME: Make a time manager
	hour++;

	// Goblin management
	for (unsigned int i = 0; i < goblins.size(); i++) 
	{
		goblins.at(i)->Update();
	}

	// FIXME: Use a hiring manager
	// --Start goblin hiring process
	if (hireNewGoblin.Overlaps(mousePos)) 
	{
		hireNewGoblin.SetAlpha(255);

		if (InputManager::GetMouseButtonUp(MOUSE_BUTTON::MB_LEFT)) 
		{
			// Hire a new goblin
			HireGoblin();
		}
	}
	else 
	{
		hireNewGoblin.SetAlpha(100);
	}

	hireNewGoblin.Draw();
	// --End goblin hiring process

	if (testSwitch.GetActive() && DrawTileOptions() == false)
	{
		if (tileTypeIndex != -1) HandlePlaceItems();
	}
	else if (testSwitch.GetActive() == false) 
	{
		tileTypeIndex = -1;
		highlighting = false;
	}

	if (tileTypeIndex == -1) 
	{
		// DEBUG: Moves a unit to a selected position
		/*if (InputManager::GetMouseButtonUp(MB_LEFT))
			goblin.MoveTo(worldMouse);*/
	}

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

	// Display the users money
	std::string moneyStr = std::to_string(money);
	auto num = money / 100;
	auto den = money % 100;
	moneyStr = std::to_string(num) + ".";
	if (den < 10) moneyStr += "0" + std::to_string(den);
	else moneyStr += std::to_string(den);
	
	moneySprite.SetColorMod(Color::GREEN);
	moneySprite.Draw();
	DrawOutlinedString(moneyStr, 30, 4, 30, 3U);

	MoveCamera(camMove.x, camMove.y);
	//MoveZoom(InputManager::GetMouseWheel());
	//DrawOutlinedString(std::to_string(GetCameraObject()->zoom), 50, 90, 20, 3U);

	return true;
}

void GoblinsMain::Draw(gobl::GoblRenderer& renderer)
{
	// FIXME: This is non-functional
}