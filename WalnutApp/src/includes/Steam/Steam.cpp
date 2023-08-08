#include <unordered_map>
#include <filesystem>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
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
			LoadFromAcf(path);
			return;
		}

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

		// Iterate through libraryfolders.vdf
		for (auto& [key, value] : libraryData.items()) {
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
				}
			}
		}
	}

	void LoadGameIcons()
	{
		for (const auto& [drive, _] : registered_games.items()) {
			for (const auto& [game, manifest] : registered_games[drive].items())
			{
				string library_cache = "C:\\Program Files (x86)\\Steam\\appcache\\librarycache\\";
				string thumbnail_path = library_cache + (string)manifest["appid"] + "_library_600x900.jpg";
				if (fs::exists(thumbnail_path)) {
					game_images[manifest["name"]] = make_shared<Walnut::Image>(thumbnail_path);
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