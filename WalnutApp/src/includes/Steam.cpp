#pragma once
#include "Includes.hpp"
#include "ManifestParser.hpp"

using namespace std;
using namespace m_parser;
namespace fs = filesystem;
using json = nlohmann::json;

namespace Steam
{
	bool Init()
	{
		m_steamPath = Utils::Registry::GetString(HKEY_CURRENT_USER, m_pSteamRegistry, "SteamPath");
		m_steamPath = Utils::cleansePath(m_steamPath);
		return !m_steamPath.empty();
	}

	string GetLoginUser()
	{
		return Utils::Registry::GetString(HKEY_CURRENT_USER, m_pSteamRegistry, "AutoLoginUser");
	}

	bool IsRememberPasswordChecked()
	{
		return (Utils::Registry::GetDWORD(HKEY_CURRENT_USER, m_pSteamRegistry, "RememberPassword") == 1U);
	}

	bool IsRunning()
	{
		if (m_bNeverCheckIfRunning)
			return false;
		return (Utils::GetProcessID(&m_sExecutable[0]) != 0U);
	}

	void Start(string m_sArgs)
	{
		m_sArgs.insert(0, "-noreactlogin ");

		cout << m_sArgs << endl;

		string m_sSteamExe = m_steamPath + "\\" + m_sExecutable + " " + m_sArgs;

		STARTUPINFOA m_sStartupInfo;
		ZeroMemory(&m_sStartupInfo, sizeof(m_sStartupInfo));
		m_sStartupInfo.cb = sizeof(STARTUPINFOA);

		PROCESS_INFORMATION m_pProcessInfo;

		if (!CreateProcessA(0, &m_sSteamExe[0], 0, 0, 0, 0, 0, 0, &m_sStartupInfo, &m_pProcessInfo))
			return;

		CloseHandle(m_pProcessInfo.hProcess);
		CloseHandle(m_pProcessInfo.hThread);
	}

	void Exit()
	{
		if (!IsRunning()) return;

		Start("-exitsteam");

		while (IsRunning())
			Sleep(500);
	}

	string GetSteamID3(const string& steamid64) {
		string steamid3 = to_string(stoull(steamid64) & 0xFFFFFFFF);
		return steamid3;
	}

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
		stringstream buffer;
		buffer << libraryFile.rdbuf();

		// Convert contents to json
		istringstream inputStream(buffer.str());
		json libraryData = parseJson(inputStream);

		m_steamPath = Utils::cleansePath(libraryData["0"]["path"].get<string>());

		// Iterate through libraryfolders.vdf
		for (auto& [key, value] : libraryData.items()) {
			static const regex numberRegex("^[-+]?[0-9]*\\.?[0-9]+$");
			if (!regex_match(string(key), numberRegex))
				continue;
			string library_path = value["path"].get<string>();
			library_path = Utils::cleansePath(library_path);
			library_path += "\\steamapps";

			// Load games from libraries path
			LoadFromAcf(library_path);
		}
	}

	void LoadFromAcf(string path)
	{
		// Get drive letter from path
		string drive = Utils::upperString(path.substr(0, 2));

		if (registered_games.count("all_games") == 0)
			registered_games["all_games"] = json::object();
		if (registered_games["all_games"].count(drive) == 0)
			registered_games["all_games"][drive] = json::object();

		if (!fs::exists(path))
			return;

		for (const auto& entry : fs::directory_iterator(path)) {
			if (fs::is_regular_file(entry)) {
				if (entry.path().filename().extension() == ".acf") {
					// Read manifest
					ifstream manifestFile(entry.path().string());
					if (!manifestFile.is_open())
						continue;
					cout << "Opened " << entry.path().string() << endl;

					// Read the entire file content into the buffer
					stringstream buffer;
					buffer << manifestFile.rdbuf();

					// Convert contents to json
					istringstream inputStream(buffer.str());
					json manifestData = parseJson(inputStream);

					if (registered_games["all_games"][drive].count("Steam") == 0)
						registered_games["all_games"][drive]["Steam"] = json::object();

					// Check if registered_games already contains this game
					if (find(registered_games["all_games"][drive]["Steam"].begin(), registered_games["all_games"][drive]["Steam"].end(), manifestData["name"]) != registered_games["all_games"][drive]["Steam"].end())
						continue;

					//registered_games[manifestData["name"]] = manifestData;
					registered_games["all_games"][drive]["Steam"][manifestData["name"]] = manifestData;
				}
			}
		}
	}

	void LoadGameIcons()
	{
		for (const auto& [drive, platforms] : registered_games["all_games"].items()) {
			for (const auto& [platform, games] : platforms.items())
			{
				// Load steam icons
				if (string(platform) == "Steam")
				{
					for (const auto& [game, manifest] : registered_games["all_games"][drive]["Steam"].items())
					{
						string thumbnail_path = m_steamPath + "\\appcache\\librarycache\\" + (string)manifest["appid"] + m_imageSuffix;
						if (fs::exists(thumbnail_path)) {
							game_images[manifest["name"]] = make_shared<Walnut::Image>(thumbnail_path);
						}
						else if (game_images.find(manifest["name"]) != game_images.end())
							game_images.erase(manifest["name"]);
					}
				}
			}
		}
	}

	void LoadUserData()
	{
		if (!fs::exists(m_steamPath + "\\config\\loginusers.vdf"))
			return;

		// Get account data
		ifstream userInfoFile(m_steamPath + "\\config\\loginusers.vdf");
		if (userInfoFile.is_open())
		{
			stringstream buffer;
			buffer << userInfoFile.rdbuf();
			istringstream inputStream(buffer.str());
			json parsedData = parseJson(inputStream);

			// Initialize m_steamUserData
			for (const auto& [uuid, data] : parsedData.items())
			{
				string steamID3 = GetSteamID3(uuid);
				string account_name = data["AccountName"];
				m_steamUserData[account_name] = json::object();
				m_steamUserData[account_name]["steamID64"] = uuid;
				m_steamUserData[account_name]["steamID3"] = steamID3;
			}

			for (const auto& [username, data] : m_steamUserData.items())
			{
				string user_path = m_steamPath + "\\userdata\\" + string(data["steamID3"]);
				if (fs::exists(user_path))
				{
					string localconfig_path = user_path + "\\config\\localconfig.vdf";
					if (!fs::exists(localconfig_path))
						continue;

					// Read localconfig.vdf
					ifstream localconfigFile(localconfig_path);
					if (!localconfigFile.is_open())
						continue;

					// Read the entire file content into the buffer
					stringstream buffer;
					buffer << localconfigFile.rdbuf();

					// Convert contents to json
					istringstream inputStream(buffer.str());
					json localconfigData = parseJson(inputStream);

					m_steamUserData[username]["apps"] = json::array();
					for (const auto& [appid, _] : localconfigData["Software"]["Valve"]["steam"]["apps"].items())
						m_steamUserData[username]["apps"].push_back(appid);

					// Add to m_steamAcounts
					m_steamAccounts.push_back(username);

					cout << "Loaded " << username << " with " << m_steamUserData[username]["apps"].size() << " apps" << endl;
				}
				else cout << "User path " << user_path << " does not exist" << endl;
			}
		}
	}

	void RemoveGame(string drive, string game)
	{
		registered_games["all_games"][drive]["Steam"].erase(game);
		if (game_images.find(game) != game_images.end())
			game_images.erase(game);

		try {
			registered_games["favorite"][drive]["Steam"].erase(game);
			if (registered_games["favorite"][drive]["Steam"].empty())
				registered_games["favorite"][drive].erase("Steam");
			if (registered_games["favorite"][drive].empty())
				registered_games["favorite"].erase(drive);
		}
		catch (const exception& e) {
			cout << "Game not in favorites" << endl;
		}
	}

	void RunGame(string appid)
	{
		// Get selected account for this game
		string currentAccount = GetLoginUser();

		if (string(selected_game["selected_account"]) != currentAccount)
		{
			// Sign in to the selected account
			cout << "Signing in to " << selected_game["selected_account"] << endl;

			// Shutdown steam
			Exit();

			bool m_bSomeRegisterFailed = false;
			if (!Utils::Registry::WriteString(HKEY_CURRENT_USER, m_pSteamRegistry, "AutoLoginUser", string(selected_game["selected_account"])))
				m_bSomeRegisterFailed = true;

			if (!Utils::Registry::WriteDWORD(HKEY_CURRENT_USER, m_pSteamRegistry, "RememberPassword", 1U))
				m_bSomeRegisterFailed = true;

			if (m_bSomeRegisterFailed)
				cout << "Failed to write to registry" << endl;
			else
				Start(string("-silent -applaunch " + appid));
		}
		else
		{
			string command = "\"" + m_steamPath + "\\" + m_sExecutable + "\" -applaunch " + appid;
			system(command.c_str());
		}
	}

	void OpenSteamDB(string appid)
	{
		string url = "https://steamdb.info/app/" + appid;
		ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	void SelectGame(string drive, string game)
	{
		cout << "Selecting game " << game << " on drive " << drive << endl;
		m_steamGameAccounts.clear();
		selected_game = registered_games["all_games"][drive]["Steam"][game];
		selected_game["drive"] = drive;
		selected_game["platform"] = "Steam";

		for (const auto& [username, userData] : m_steamUserData.items())
		{
			for (const auto& appid : userData["apps"])
			{
				// Skip if this account doesn't have the selected game
				if (appid != selected_game["appid"])
					continue;

				// Add account to list of Steam accounts that have this game
				cout << "Found account " << username << endl;
				m_steamGameAccounts.push_back(username);

				// Set selected account to the first one found
				if (selected_game.find("selected_account") == selected_game.end())
					selected_game["selected_account"] = username;
			}
		}
	}
}