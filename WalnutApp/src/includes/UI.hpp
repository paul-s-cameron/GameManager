#pragma once

using json = nlohmann::json;

class GameGrid
{
public:
	GameGrid() = default;

	void Render();

	void RenderGameGrid(ImVec2 buttonSize, const char* filter);

	void RenderFavoriteGames(const char* filter, int buttonsPerRow, float padding, ImVec2 buttonSize);

	void RenderAllGames(const char* filter, int buttonsPerRow, float padding, ImVec2 buttonSize);

	void RenderPopupMenu(string drive, string game, string appid);

	int GetIconSize() { return iconSize; }

	void SetIconSize(float size) { iconSize = size; }
private:
	int iconSize = 120;
	bool header = false;
	char filter[256] = "";
	const int horizontalPadding = 8;

	ImVec2 buttonSize = { (float)iconSize, (float)iconSize };
};

class GameInfoWindow
{
public:
	GameInfoWindow() = default;

	void Render();

	void DisplayAccount();

	void LastUpdate();

	void ManifestDebug();
private:
	int account = 0;
};

class AccountInfo
{
public:
	AccountInfo() = default;

	void Render();
private:
	int account = 0;
	vector<string> accounts;
	string current_user = Steam::GetLoginUser();
};