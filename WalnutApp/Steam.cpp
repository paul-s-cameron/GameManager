#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>

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

					// Find games images
					string library_cache = "C:\\Program Files (x86)\\Steam\\appcache\\librarycache\\";
					string thumbnail_path = library_cache + (string)manifestData["appid"] + "_library_600x900.jpg";
					if (fs::exists(thumbnail_path)) {
						cout << "Found image for " << manifestData["name"] << endl;
						game_images[manifestData["name"]] = make_shared<Walnut::Image>(thumbnail_path);
					}
					else {
						cout << "No image for " << manifestData["name"] << endl;
					}

					cout << "Added " << manifestData["name"] << " to registered_games with appid: " << manifestData["appid"] << endl;
				}
			}
		}
	}
}