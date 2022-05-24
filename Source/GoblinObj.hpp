#pragma once
#ifndef GOBLIN_OBJ_H
#define GOBLIN_OBJ_H

#include "GoblEngine.hpp"
#include "Map.hpp"
#include <unordered_map>
#include <queue>
#include "CompositeSprite.hpp"

class GoblinObj
{
private:
	uint32_t id;

	Vec2 pos{};
	Vec2 targetPos{};
	cSpr::CompositeSprite sprite;

	std::unordered_map<std::string, uint8_t> skills{};

	int workableId = -1;

	float speed = 1.0f;
	float moveSpd = 25.0f;

	const float TIMER_COUNT = 1.0f;
	float timer = TIMER_COUNT;

	bool atHome = true;
	bool inWorkHours = true;
	bool doingTask = false;

	// FIXME: Add events list
	std::queue<void(*)(GoblinObj* obj)> events{};

	void HandleEvents();

	MAP::Map* map = nullptr;

public: // Accessors
	const Vec2 GetPos() { return pos; };
	const Vec2 GetTargetPos() { return targetPos; };
	bool ReachedTarget() 
	{
		bool arrived = Vec2::GetDistance(pos, targetPos) <= 0.2f;
		if (arrived) pos = targetPos;
		return arrived;
	}

public: // Mutators
	void SetAtHome(bool v) { atHome = v;  }
	void AssignMap(MAP::Map* map) { this->map = map; }

public:
	unsigned char taskProgress = 0;

	void Update();

	void MoveToTarget();
	void MoveTo(Vec2 pos);
	void RunTestEvents();
	void EndCurrentTask() 
	{ 
		taskProgress = 0;
		if (gobl::GoblEngine::debugging) std::cout << "Goblin-" << id << ": Finished task." << std::endl;
		doingTask = false;
	}

	void GenerateSprite(gobl::GoblEngine* ge)
	{
		sprite.SetEngine(ge);
		// FIXME: Allow modder to input the goblins sprites
		std::string* dirs = new std::string[]{"Sprites/Worker_Eyes.png", "Sprites/Worker_Mouth.png", "Sprites/Worker_Nose.png", "Sprites/Worker_Hair.png", "Sprites/Worker_Head.png" };
		unsigned int len = 5;
		unsigned int sprLen = 5; // FIXME: Allow modder to specify the number of goblin sprites
		sprite.CreateSprites(dirs, len);
		delete[] dirs;

		sprite.SetDimensions({ 32,32 });
		sprite.SetReverseRenderOrder(true);

		int* indexs = new int[len];
		for (unsigned int i = 0; i < len; i++)
			indexs[i] = rand() % sprLen; // FIXME: Use a separate sprite length for each goblin sprite element

		sprite.SetSprites(indexs);
		delete[] indexs;
	}

	GoblinObj() 
	{
		id = rand();
	}
};

#endif