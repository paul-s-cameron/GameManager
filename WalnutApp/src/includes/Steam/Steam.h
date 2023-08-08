#pragma once
#include "../json.hpp"
#include "Walnut/Image.h"

using namespace std;
using json = nlohmann::json;

namespace Steam
{
	inline string steam_path;

	inline json selected_game;

	inline json registered_games;

	inline unordered_map<string, shared_ptr<Walnut::Image>> game_images;

	void DetectInstalls(string path);

	void LoadFromAcf(string path);

	void LoadGameIcons();

	void RemoveGame(string drive, string game);

	void RunGame(string appid);

	void OpenSteamDB(string appid);
}