#pragma once
#ifndef GOBLIN_OBJ_H
#define GOBLIN_OBJ_H

#include "GoblEngine.hpp"
#include "Map.hpp"
#include <unordered_map>
#include <queue>

class GoblinObj
{
private:
	uint32_t id;

	Vec2 pos{};
	Vec2 targetPos{};
	gobl::Sprite sprite;

	std::unordered_map<std::string, uint8_t> skills{};

	int workableId = -1;

	float speed = 1.0f;
	float moveSpd = 0.5f;

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
		ge->CreateSpriteObject(sprite, "Sprites/worker.png");
	}

	GoblinObj() 
	{
		id = rand();
	}
};

#endif