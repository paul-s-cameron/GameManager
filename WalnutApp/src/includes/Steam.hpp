#pragma once

using json = nlohmann::json;

namespace Steam
{
	inline std::string				m_steamPath;
	inline std::string				m_sExecutable = "steam.exe";
	inline const char*				m_pSteamRegistry = "Software\\Valve\\Steam";
	inline bool						m_bNeverCheckIfRunning = false;
	inline std::string				m_imageSuffix = "_library_600x900.jpg";
	inline json						m_steamUserData = json::object();
	inline std::vector<std::string> m_steamGameAccounts = {};
	inline std::vector<std::string> m_steamAccounts = {};

	bool Init();

	std::string GetLoginUser();

	bool IsRememberPasswordChecked();

	bool IsRunning();

	void Start(std::string m_sArgs = "");

	void Exit();

	std::string GetSteamID3(const std::string& steamid64);

	void DetectInstalls(std::string path);

	void LoadFromAcf(std::string path);

	void LoadGameIcons();

	void LoadUserData();

	void RemoveGame(std::string drive, std::string game);

	void RunGame(std::string appid);

	void OpenSteamDB(std::string appid);

	void SelectGame(std::string drive, std::string game);
}