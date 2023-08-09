#pragma once
#include "Walnut/Image.h"
#include "../json.hpp"
#include "public/steam/steam_api.h"

using namespace std;
using json = nlohmann::json;

namespace Steam
{
	inline string image_suffix = "_library_600x900.jpg";
	inline string steam_path;
	inline vector<string> account_ids = {};
	inline CSteamID active_account;

	inline json selected_game;
	inline json registered_games;
	inline json user_data = json::object();

	inline unordered_map<string, shared_ptr<Walnut::Image>> game_images;

	void DetectInstalls(string path);

	void LoadFromAcf(string path);

	void LoadGameIcons();

	void LoadUserData();

	void RemoveGame(string drive, string game);

	void RunGame(string appid);

	void OpenSteamDB(string appid);

	void SelectGame(string drive, string game);
}