#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>

#include "json.hpp"
#include "Steam.h"

using namespace std;
namespace fs = filesystem;
using json = nlohmann::json;

namespace Steam
{
	// Add detected games to registered_games
	void DetectInstalls(string path)
	{
		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry)) {
				if (entry.path().filename().extension() == ".acf") {
					// Read manifest
					ifstream manifest_file(entry.path().string());
					json manifest_json;

					// Check if registered_games already contains this game
					//if (find(registered_games.begin(), registered_games.end(), appid) != registered_games.end())
					//	continue;

					//registered_games[appid] = {
					//	{"name", manifest_json["name"].get<string>()},
					//	{"path", entry.path().string()}
					//};
				}
			}
		}
	}
}