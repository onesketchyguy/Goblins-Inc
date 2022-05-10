#include "GoblinObj.hpp"
#include "GoblinsMain.hpp"
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

	Vec2 newPos = pos;
	newPos.MoveTowards(targetPos, moveSpd * Clock::GetDeltaTime());

	int mapIndex = map->GetTileFromWorldPos(static_cast<int>(newPos.x), static_cast<int>(newPos.y));
	if (map->GetCollision(mapIndex) == false)
	{
		pos = newPos;
	}
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

void GoblinObj::Update()
{
	if (inWorkHours)
	{
		HandleEvents();

		if (workableId == -1)
		{
			// FIXME: Save the ID for future use
			workableId = map->GetEmptyWorkable();

			if (workableId != -1)
			{
				IntVec2 deskPos = map->GetWorkable(workableId);
				MoveTo(Vec2{ float(deskPos.x), float(deskPos.y) });
			}
		}
		else
		{
			if (atHome)
			{
				// Go to work
				SetAtHome(false);
				IntVec2 deskPos = map->GetWorkable(workableId);
				MoveTo(Vec2{ float(deskPos.x), float(deskPos.y) });
			}
			else if (events.empty()) // We have no tasks, add one
			{
				// FIXME: Use the workable to determine what task to do

				auto action = [](GoblinObj* obj)
				{
					// FIXME: Work on a task
					obj->taskProgress++;

					if (obj->taskProgress >= 200)
					{
						// Provide income on task completion

						// FIXME: Set amount based on workable task
						GoblinsMain::money++;

						obj->EndCurrentTask();
					}
				};

				events.push(action);
			}
		}
	}
	else
	{
		// Go home and do nothing
		SetAtHome(true);
		MoveTo(Vec2{ 0.0f, 0.0f });
	}

	sprite.SetPosition(pos);
	sprite.DrawRelative(gobl::GoblEngine::GetCameraObject());
}