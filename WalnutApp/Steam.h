#pragma once

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

namespace Steam
{
	inline json registered_games;

	void DetectInstalls(string);
	json ParseManifest(string);
}