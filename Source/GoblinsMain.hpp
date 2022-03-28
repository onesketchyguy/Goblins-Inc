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

	gobl::Sprite sprite;
	gobl::Sprite highlightSprite;

	MAP::Map map;

	// FIXME: Provide a proper scene system
	Scene currScene = Scene::MainMenu;

	// FIXME: Provide a state machine for the users current menu
	bool quittingApp = false;
	bool quitToMenu = false;

	int texturesLoaded = 0;
	const int texturesToLoad = 5;
	Uint32 tileTypeIndex = -1;

	void DrawButton(std::string buttonText, IntVec2 pos, bool& clicked);
	void DrawValidate(std::string prompt, bool& yes, bool& no);
	bool DrawTileOptions();
	void DrawWorld();

	IntVec2 startMouse{};
	IntVec2 startCell{};
	int startX, startY;
	bool highlighting = false;

private:
	void Init() override { SetTitle("Goblins inc."); }

	bool Start() override;

	bool Update() override;

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