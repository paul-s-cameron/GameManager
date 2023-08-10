#pragma once

using json = nlohmann::json;

class GameInfoWindow
{
public:
	GameInfoWindow() = default;

	void Render();

	void DisplayAccount();

	void ManifestDebug();
private:
	int account = 0;
};

class GameGrid
{
public:
	GameGrid() = default;

	void Render();

	void RenderGameGrid(ImVec2 buttonSize, const char* filter);

	void GamePopupMenu(std::string drive, json manifest);

	int GetIconSize() { return iconSize; }

	void SetIconSize(float size) { iconSize = size; }
private:
	char filter[256];
	int iconSize = 120;
	bool header = false;
	const int horizontalPadding = 8;

	ImVec2 buttonSize = { (float)iconSize, (float)iconSize };
};