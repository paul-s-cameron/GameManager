#pragma once
#include "Steam.h"

using namespace std;
using namespace m_parser;
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
					ifstream manifestFile(entry.path().string());
					if (!manifestFile.is_open())
						continue;
					cout << "Opened " << entry.path().string() << endl;

					// Read the entire file content into the buffer
					std::stringstream buffer;
					buffer << manifestFile.rdbuf();

					// Convert contents to json
					istringstream inputStream(buffer.str());
					json manifestData = parseJson(inputStream);

					// Check if registered_games already contains this game
					if (find(registered_games.begin(), registered_games.end(), manifestData["name"]) != registered_games.end())
						continue;

					registered_games[manifestData["name"]] = manifestData;
				}
			}
		}
	}

	void LoadGameIcons()
	{
		for (const auto& [game, manifest] : registered_games.items()) {
			string library_cache = "C:\\Program Files (x86)\\Steam\\appcache\\librarycache\\";
			string thumbnail_path = library_cache + (string)manifest["appid"] + "_library_600x900.jpg";
			if (fs::exists(thumbnail_path)) {
				game_images[manifest["name"]] = make_shared<Walnut::Image>(thumbnail_path);
			}
		}
	}

	void RunGame(string appid)
	{
		string command = "\"" + steam_path + "\" -applaunch " + appid;
		system(command.c_str());
	}
}