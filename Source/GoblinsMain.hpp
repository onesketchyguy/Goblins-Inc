#pragma once
#ifndef GOBLINS_MAIN
#define GOBLINS_MAIN

#include "GoblEngine.hpp"
#include "Map.hpp"

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

	gobl::Switch testSwitch;

	gobl::Sprite testSprite;
	gobl::Sprite moneySprite;
	gobl::Sprite hireNewGoblin; // DEBUG: Using this for a temporary solution for hiring goblins

	gobl::Sprite highlightSprite;
	gobl::Sprite bulldozerSprite;

	MAP::Map map;

	static long long money;

	// FIXME: Provide a proper scene system
	Scene currScene = Scene::MainMenu;

	// FIXME: Provide a state machine for the users current menu
	bool quittingApp = false;
	bool quitToMenu = false;

	// FIXME: Load in the textures overtime instead of all at once
	//int texturesLoaded = 0;
	//const int texturesToLoad = 5;

	// FIXME: Use a goblin manager
	void HireGoblin();

	void HandlePlaceItems();
	void HandlePickupItems();
	void DrawImageButton(gobl::Sprite& spr, IntVec2 pos, bool& clicked);
	void DrawButton(std::string buttonText, IntVec2 pos, bool& clicked);
	void DrawValidate(std::string prompt, bool& yes, bool& no);
	void DrawSelectedObject();
	bool DrawTileOptions();
	bool DrawObjectOptions();
	void DrawWorld(bool blur = false);

private:
	void Init() override { SetTitle("Goblins inc."); }
	bool Start() override;
	bool Update() override;
	void Draw(gobl::GoblRenderer& renderer) override;
	bool Exit() override
	{
		map.Destroy();

		return true;
	}
};

#endif // !GOBLINS_MAIN