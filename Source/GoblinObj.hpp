#pragma once
#ifndef GOBLIN_OBJ_H
#define GOBLIN_OBJ_H

#include "GoblEngine.hpp"
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

	float speed = 1.0f;

	const float TIMER_COUNT = 1.0f;
	float timer = TIMER_COUNT;

	bool inWorkHours = true;
	bool doingTask = false;

	// FIXME: Add events list
	std::queue<void(*)(GoblinObj* obj)> events{};

	void HandleEvents();

public: // Accessors
	const Vec2 GetPos() { return pos; };
	const Vec2 GetTargetPos() { return targetPos; };
	bool ReachedTarget() 
	{
		bool arrived = Vec2::GetDistance(pos, targetPos) <= 0.2f;
		if (arrived) pos = targetPos;
		return arrived;
	}

public:
	void Update() 
	{
		if (inWorkHours) 
		{
			HandleEvents();
		}
		else 
		{
			// Go home and do nothing
		}

		sprite.SetPosition(pos);
		sprite.DrawRelative(gobl::GoblEngine::GetCameraObject());
	}

	void MoveToTarget();
	void MoveTo(Vec2 pos);
	void RunTestEvents();
	void EndCurrentTask() 
	{ 
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