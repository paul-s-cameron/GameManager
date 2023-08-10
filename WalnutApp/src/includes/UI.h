#pragma once
#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"
#include "json.hpp"

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
private:
	char filter[256];
	int iconSize = 120;
	bool header = false;
	const int horizontalPadding = 8;

	ImVec2 buttonSize = { (float)iconSize, (float)iconSize };
};