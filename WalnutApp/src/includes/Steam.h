#pragma once
#include "Walnut/Image.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

namespace Steam
{
	inline string image_suffix = "_library_600x900.jpg";
	inline string steam_path;

	inline vector<string> steamIds = {};
	inline json steamUserData = json::object();

	void DetectInstalls(string path);

	void LoadFromAcf(string path);

	void LoadGameIcons();

	void LoadUserData();

	void RemoveGame(string drive, string game);

	void RunGame(string appid);

	void OpenSteamDB(string appid);

	void SelectGame(string drive, string game);
}