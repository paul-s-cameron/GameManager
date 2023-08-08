#pragma once

#include "includes.h"

using namespace std;
using json = nlohmann::json;

namespace Steam
{
	inline json registered_games;

	inline unordered_map<string, shared_ptr<Walnut::Image>> game_images;

	void DetectInstalls(string);
}