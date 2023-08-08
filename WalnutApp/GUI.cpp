#pragma once
#include "GUI.h"

using namespace std;
using namespace utils;
using namespace Steam;
using json = nlohmann::json;

namespace GUI
{
	void RenderGameGrid(ImVec2 buttonSize)
	{
		if (registered_games.empty() || game_images.empty())
			return;
		int buttonsPerRow = static_cast<int>(ImGui::GetContentRegionAvail().x / (buttonSize.x + 8));
		int remainingWidth = ImGui::GetContentRegionAvail().x - (buttonsPerRow * buttonSize.x + 8);
		float padding = remainingWidth / (buttonsPerRow - 1);

		int row = 0;
		for (const auto& [game, manifest] : registered_games.items()) {
			if (row != 0 && row % buttonsPerRow != 0)
				ImGui::SameLine(0, padding);

			if (game_images.count(manifest["name"]) > 0)
			{
				ImGui::ImageButton(game_images[manifest["name"]]->GetDescriptorSet(), buttonSize, { 0, 0 }, { 1, 1 }, 0);
			}
			else
			{
				ImGui::Button(SplitTextWithNewlines(game, buttonSize.x).c_str(), buttonSize);
			}

			row++;
		}
	}
}