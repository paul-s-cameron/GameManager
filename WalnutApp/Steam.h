#pragma once
#include <unordered_map>
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <list>

#include "json.hpp"
#include "Walnut/Image.h"
#include "ManifestParser.hpp"

using namespace std;
using json = nlohmann::json;

namespace Steam
{
	inline string steam_path;

	inline json selected_game;

	inline json registered_games;

	inline unordered_map<string, shared_ptr<Walnut::Image>> game_images;

	void DetectInstalls(string);

	void LoadGameIcons();

	void RunGame(string appid);
}