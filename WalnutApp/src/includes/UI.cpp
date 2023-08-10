#include "globals.h"
#include "Utils.h"
#include "Steam.h"
#include "Steam.h"
#include "UI.h"

using json = nlohmann::json;

void DisplayJSON(const json& jsonData) {
	for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
		if (it->is_object() || it->is_array()) {
			if (ImGui::TreeNode(it.key().c_str())) {
				DisplayJSON(*it);
				ImGui::TreePop();
			}
		}
		else {
			ImGui::Text("%s: %s", it.key().c_str(), it->dump().c_str());
		}
	}
}

bool ButtonCenter(const char* label, const ImVec2& size_arg)
{
	ImGuiButtonFlags flags = ImGuiButtonFlags_None;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	ImVec2 pos2;
	pos2.x = pos.x + size.x;
	pos2.y = pos.y + size.y;
	const ImRect bb(pos, pos2);
	//  const ImRect bb(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::RenderNavHighlight(bb, id);
	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	float font_height = g.FontSize;

	vector<string> textArray = Utils::SplitTextWithNewlines(label, size_arg.x);

	ImVec2 targetline_size; // Font size for each line [pixel]
	ImVec2 text_start; // Character start position for each line
	int offset_x, offset_y; // Offset from top left of button
	ImDrawList* DrawList = ImGui::GetWindowDrawList();
	ImVec2 padding_regular = g.Style.TouchExtraPadding;
	ImVec2 padding_for_resize = g.IO.ConfigWindowsResizeFromEdges ? g.WindowsHoverPadding : padding_regular;

	for (int i = 0; i < textArray.size(); i++) {
		targetline_size = ImGui::CalcTextSize(textArray[i].c_str(), NULL, true);
		offset_x = (size.x - targetline_size.x) / 2.0;
		if (targetline_size.y != 0) {
			offset_y = targetline_size.y * i + padding_for_resize.y;
		}
		else {
			offset_y = font_height * i + padding_for_resize.y;
		}
		text_start.x = bb.Min.x + offset_x;
		text_start.y = bb.Min.y + offset_y;
		DrawList->AddText(text_start, ImGui::GetColorU32(ImGuiCol_Text), textArray[i].c_str());
	}

	return pressed;
}

// GameGrid
void GameGrid::Render()
{
	if (ImGui::Begin("Game Browser", false))
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 270);
		ImGui::InputTextWithHint("##Input", "Search", filter, IM_ARRAYSIZE(filter));
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 120);
		ImGui::SliderInt("##IconSizeSlider", &iconSize, 60, 360);
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 10);
		if (ImGui::Checkbox("Landscape", &header))
		{
			if (!header)
				Steam::image_suffix = "_library_600x900.jpg";
			else
				Steam::image_suffix = "_header.jpg";
			Steam::LoadGameIcons();
		}
		ImGui::PopItemWidth();
		if (!header)
			buttonSize = { (float)iconSize, (float)(iconSize * 1.5) };
		else
			buttonSize = { (float)(iconSize * 2.139), (float)iconSize };
		ImGui::BeginChild("##GameGrid", { 0, 0 }, true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
		RenderGameGrid(buttonSize, filter);
		ImGui::EndChild();
	}
	ImGui::End();
}

void GameGrid::RenderGameGrid(ImVec2 buttonSize, const char* filter)
{
	if (registered_games.empty())
		return;

	// Calculate grid dimensions
	int buttonsPerRow = static_cast<int>(ImGui::GetContentRegionAvail().x / (buttonSize.x + horizontalPadding));
	if (buttonsPerRow < 2) return; // Not enough space to render anything
	int remainingWidth = ImGui::GetContentRegionAvail().x - (buttonsPerRow * buttonSize.x);
	float padding = remainingWidth / (buttonsPerRow - 1);

	// Scale font size based on button size
	ImFont* font = ImGui::GetFont();
	float originalFontSize = font->Scale;
	float scale = buttonSize.x / 120;
	font->Scale = scale;

	json steam_games = registered_games["Steam"];
	for (const auto& [drive, _] : steam_games.items())
	{
		string title = drive + " (" + to_string(steam_games[drive].size()) + ")";
		if (!ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			continue;

		ImGui::SetCurrentFont(font);
		int row = 0;
		for (auto it = steam_games[drive].begin(); it != steam_games[drive].end(); ) {
			json manifest = *(it++);
			string game = manifest["name"];

			// Skip if filter is set and game name doesn't match
			if (strcmp(filter, "") != 0 && Utils::lowerString(game).find(Utils::lowerString(filter)) == string::npos)
				continue;

			// Add padding between buttons
			if (row != 0 && row % buttonsPerRow != 0)
				ImGui::SameLine(0, padding);

			if (game_images.count(manifest["name"]) > 0)
			{
				// Detect if mouse is hovering over button, and dim if so
				ImVec2 buttonPos = ImGui::GetCursorScreenPos();
				float dim = 1;
				if (ImGui::IsMouseHoveringRect(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y))) dim = 0.8;

				// Draw Image button
				if (ImGui::ImageButton(game_images[manifest["name"]]->GetDescriptorSet(), buttonSize, { 0, 0 }, { 1, 1 }, 0, { 0, 0, 0, 0 }, { 1, 1, 1, dim }))
					Steam::SelectGame(drive, manifest["name"]);
			}
			else
			{
				// Draw Text button
				if (ButtonCenter(game.c_str(), buttonSize))
					Steam::SelectGame(drive, manifest["name"]);
			}

			// Draw popup menu
			GamePopupMenu(drive, manifest);
			row++;
		}

		// Reset font scale
		font->Scale = originalFontSize;
		ImGui::SetCurrentFont(font);
	}
}

void GameGrid::GamePopupMenu(string drive, json manifest)
{
	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1)) {
		ImGui::OpenPopup(string(manifest["appid"]).c_str());
	}
	if (ImGui::BeginPopup(string(manifest["appid"]).c_str()))
	{
		if (ImGui::MenuItem("Launch"))
			Steam::RunGame(manifest["appid"]);
		if (ImGui::MenuItem("SteamDB"))
			Steam::OpenSteamDB(manifest["appid"]);
		if (ImGui::MenuItem("Hide"))
			Steam::RemoveGame(drive, manifest["name"]);
		ImGui::EndPopup();
	}
}

// GameInfoWindow
void GameInfoWindow::Render()
{
	if (ImGui::Begin("Game Info", false, ImGuiWindowFlags_NoScrollbar))
	{
		if (!selected_game.empty())
		{
			// Get game icon
			ImVec2 iconSize;
			shared_ptr<Walnut::Image> game_icon;

			// Check if game icon exists and assign an image
			if (game_images.count(selected_game["name"]) != 0)
				game_icon = game_images[selected_game["name"]];
			else
				game_icon = default_thumbnail;

			// Render game icon at center
			// Check if the game_icon is in portrait or landscape
			if (game_icon->GetWidth() < game_icon->GetHeight())
				iconSize = { 120, 190 };
			else
				iconSize = { 406, 190 };
					
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (iconSize.x / 2));
			ImGui::Image(game_icon.get()->GetDescriptorSet(), iconSize);
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (ImGui::CalcTextSize(string(selected_game["name"]).c_str()).x / 2));
			ImGui::Text(string(selected_game["name"]).c_str());

			ImGui::Separator();

			float buttonWidth = ImGui::GetContentRegionAvail().x / 2 - 5;
			if (ImGui::Button("Play", { buttonWidth, 50 }))
				Steam::RunGame(selected_game["appid"]);

			ImGui::SameLine(0, 5);

			if (ImGui::Button("SteamDB", { buttonWidth, 50 }))
			{
				Steam::OpenSteamDB(selected_game["appid"]);
			}

			DisplayAccount();

			//ManifestDebug();
		}
	}
	ImGui::End();
}

void GameInfoWindow::DisplayAccount()
{
	string currentAccountName = string(Steam::steamUserData[selected_game["selected_account"]]["friends"]["PersonaName"]);
	// Get index of current account
	account = find(Steam::steamIds.begin(), Steam::steamIds.end(), selected_game["selected_account"]) - Steam::steamIds.begin();

	// Display account selection combo box
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(currentAccountName.c_str()).x);
	if (ImGui::BeginCombo(currentAccountName.c_str(), string(Steam::steamIds[account]).c_str()))
	{
		for (int i = 0; i < Steam::steamIds.size(); i++)
		{
			bool is_selected = (account == i);
			if (ImGui::Selectable(string(Steam::steamIds[i]).c_str(), is_selected))
			{
				account = i;
				selected_game["selected_account"] = Steam::steamIds[i];
				registered_games["Steam"][selected_game["drive"]][selected_game["name"]]["selected_account"] = selected_game["selected_account"];
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
}

void GameInfoWindow::ManifestDebug()
{
	if (ImGui::TreeNode("Manifest"))
	{
		DisplayJSON(selected_game);
		ImGui::TreePop();
	}
}