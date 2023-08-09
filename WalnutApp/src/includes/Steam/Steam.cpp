#include <unordered_map>
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <regex>
#include <list>

#include "Steam.h"
#include "../utils.h"
#include "../ManifestParser.hpp"

using namespace std;
using namespace utils;
using namespace m_parser;
namespace fs = filesystem;
using json = nlohmann::json;

namespace Steam
{
	// Add detected games to registered_games
	void DetectInstalls(string path)
	{
		string libraryfolders_path = path + "\\libraryfolders.vdf";
		if (!fs::exists(libraryfolders_path)) {
			strcpy(message, "libraryfolders.vdf not found");
			LoadFromAcf(path);
			return;
		}
		strcpy(message, "libraryfolders.vdf found");

		// Get drive letter from path
		string drive_letter = path.substr(0, 2);
		cout << drive_letter << endl;

		// Read libraryfolders.vdf
		ifstream libraryFile(libraryfolders_path);
		if (!libraryFile.is_open())
			return;

		// Read the entire file content into the buffer
		std::stringstream buffer;
		buffer << libraryFile.rdbuf();

		// Convert contents to json
		istringstream inputStream(buffer.str());
		json libraryData = parseJson(inputStream);

		steam_path = cleansePath(libraryData["0"]["path"].get<string>());

		// Iterate through libraryfolders.vdf
		for (auto& [key, value] : libraryData.items()) {
			static const regex numberRegex("^[-+]?[0-9]*\\.?[0-9]+$");
			if (!regex_match(string(key), numberRegex))
				continue;
			string library_path = value["path"].get<string>();
			library_path = cleansePath(library_path);
			library_path += "\\steamapps";

			LoadFromAcf(library_path);
		}
	}

	void LoadFromAcf(string path)
	{
		// Get drive letter from path
		string drive_letter = path.substr(0, 2);
		cout << drive_letter << endl;

		if (!fs::exists(path))
		{
			strcpy(message, (string("Path does not exist: ") + path).c_str());
			return;
		}

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

					if (registered_games.count(drive_letter) == 0)
						registered_games[drive_letter] = json::object();

					// Check if registered_games already contains this game
					if (find(registered_games[drive_letter].begin(), registered_games[drive_letter].end(), manifestData["name"]) != registered_games[drive_letter].end())
						continue;

					//registered_games[manifestData["name"]] = manifestData;
					registered_games[drive_letter][manifestData["name"]] = manifestData;

					strcpy(message, "Game added");
				}
			}
		}
	}

	void LoadGameIcons()
	{
		for (const auto& [drive, _] : registered_games.items()) {
			for (const auto& [game, manifest] : registered_games[drive].items())
			{
				string thumbnail_path = steam_path + "\\appcache\\librarycache\\" + (string)manifest["appid"] + "_library_600x900.jpg";
				if (fs::exists(thumbnail_path)) {
					game_images[manifest["name"]] = make_shared<Walnut::Image>(thumbnail_path);
					strcpy(message, "Game icon loaded");
				}
			}
		}
	}

	void RemoveGame(string drive, string game)
	{
		registered_games[drive].erase(game);
		if (game_images.find(game) != game_images.end())
			game_images.erase(game);
	}

	void RunGame(string appid)
	{
		string command = "\"" + steam_path + "\" -applaunch " + appid;
		system(command.c_str());
	}

	void OpenSteamDB(string appid)
	{
		string url = "https://steamdb.info/app/" + appid;
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}
}