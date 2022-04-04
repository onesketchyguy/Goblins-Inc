#include "GoblinObj.hpp"
#include <stdio.h>

void GoblinObj::HandleEvents()
{
	if (events.empty() == true || doingTask == true) 
	{
		if (doingTask == true) 
		{
			// Handle current event until it is completed
			events.front()(this);
			if (doingTask == false) events.pop();
		}
		return;
	}

	if (timer <= 0)
	{
		timer = TIMER_COUNT;

		if (gobl::GoblEngine::debugging) std::cout << "Goblin-" << id << ": Doing event.." << std::endl;

		// Dequeue an event and execute it
		doingTask = true;
	}
	else timer -= Clock::GetDeltaTime() * speed;
}

void GoblinObj::MoveToTarget()
{
	IntVec2 dim = sprite.GetDimensions();

	if (targetPos.x > pos.x) 
	{
		// Face right
		sprite.SetFlipped(true);
	}
	else if (targetPos.x < pos.x)
	{
		// Face left
		sprite.SetFlipped(false);
	}

	pos.x = lerp(pos.x, targetPos.x, Clock::GetDeltaTime());
	pos.y = lerp(pos.y, targetPos.y, Clock::GetDeltaTime());
}

void GoblinObj::MoveTo(Vec2 pos) 
{
	targetPos = pos;

	auto move = [](GoblinObj* obj)
	{
		obj->MoveToTarget();

		if (obj->ReachedTarget()) obj->EndCurrentTask();
	};

	events.push(move);
}

void GoblinObj::RunTestEvents()
{
	auto e1 = [](GoblinObj* obj)
	{
		std::cout << "Executing from event!" << std::endl;
		obj->EndCurrentTask();
	};

	auto e2 = [](GoblinObj* obj)
	{
		std::cout << "Executing from event again!" << std::endl;
		obj->EndCurrentTask();
	};
	
	events.push(e1);
	events.push(e2);
}