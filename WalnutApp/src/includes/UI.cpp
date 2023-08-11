#include "Includes.hpp"

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

bool FilledCheckbox(const char* label, bool* value, const float size = 28) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImVec2 pos = window->DC.CursorPos;
	ImGui::InvisibleButton(label, ImVec2(label_size.x + size, label_size.y));

	bool clicked = false;

	if (ImGui::IsItemClicked()) {
		*value = !(*value);
		clicked = true;
		ImGui::MarkItemEdited(id);
	}

	ImU32 bg_col = ImGui::GetColorU32(ImGuiCol_FrameBg);
	ImU32 bg_col_active = ImGui::GetColorU32(ImGuiCol_FrameBgActive);
	ImU32 check_col = ImGui::GetColorU32(ImGuiCol_CheckMark);

	if (ImGui::IsItemHovered()) {
		bg_col = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
		bg_col_active = ImGui::GetColorU32(ImGuiCol_FrameBgActive);
		check_col = ImGui::GetColorU32(ImGuiCol_Text);
	}

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), bg_col, style.FrameRounding);
	if (*value) {
		const float pad = ImMax(1.0f, (float)(int)(size / 6.0f));
		draw_list->AddRectFilled(ImVec2(pos.x + pad, pos.y + pad), ImVec2(pos.x + size - pad, pos.y + size - pad), check_col);
	}

	if (label_size.x > 0)
		ImGui::RenderText(ImVec2(pos.x + size, pos.y), label);

	return clicked;
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
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 48);
		ImGui::SliderInt("##IconSizeSlider", &iconSize, 60, 360);
		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 10);
		if (FilledCheckbox("##Landscape", &header, 32))
		{
			if (!header)
				Steam::m_imageSuffix= "_library_600x900.jpg";
			else
				Steam::m_imageSuffix = "_header.jpg";
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

	RenderFavoriteGames(filter, buttonsPerRow, padding, buttonSize);
	RenderAllGames(filter, buttonsPerRow, padding, buttonSize);
}

void GameGrid::RenderFavoriteGames(const char* filter, int buttonsPerRow, float padding, ImVec2 buttonSize)
{
	///////////////////////////////////////////////
	// This loop does not seperate games by drive//

	if (registered_games.count("favorite") == 0) return;
	if (registered_games["favorite"].empty()) return;
	if (!ImGui::CollapsingHeader("Favorites", ImGuiTreeNodeFlags_DefaultOpen))
		return;

	int row = 0;
	for (const auto& [drive, platforms] : registered_games["favorite"].items())
	{
		for (auto app = platforms.begin(); app != platforms.end();)
		{
			string platform = app.key();
			json games = app.value();
			app++;

			// Load Steam games
			if (string(platform) == "Steam")
			{
				for (auto it = games.begin(); it != games.end();)
				{
					string game = it.key();
					string appid = it.value();
					it++;

					// Skip if filter is set and game name doesn't match
					if (strcmp(filter, "") != 0 && Utils::lowerString(game).find(Utils::lowerString(filter)) == string::npos)
						continue;

					// Add padding between buttons
					if (row != 0 && row % buttonsPerRow != 0)
						ImGui::SameLine(0, padding);

					if (game_images.count(game) > 0)
					{
						// Detect if mouse is hovering over button, and dim if so
						ImVec2 buttonPos = ImGui::GetCursorScreenPos();
						float dim = 1;
						if (ImGui::IsMouseHoveringRect(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y))) dim = 0.8;

						// Draw Image button
						PushID(("favorite" + game).c_str());
						if (ImGui::ImageButton(game_images[game]->GetDescriptorSet(), buttonSize, { 0, 0 }, { 1, 1 }, 0, { 0, 0, 0, 0 }, { 1, 1, 1, dim }))
							Steam::SelectGame(drive, game);
						PopID();
					}
					else
					{
						// Draw Text button
						if (ButtonCenter(game.c_str(), buttonSize))
							Steam::SelectGame(drive, game);
					}

					if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1)) {
						ImGui::OpenPopup(string(appid).c_str());
					}
					if (registered_games["favorite"][drive].count("Steam") == 0)
						break;
					row++;
				}
			}
		}

		if (registered_games["favorite"][drive].empty())
		{
			registered_games["favorite"].erase(drive);
			break;
		}
	}
}

void GameGrid::RenderAllGames(const char* filter, int buttonsPerRow, float padding, ImVec2 buttonSize)
{
	for (const auto& [drive, platforms] : registered_games["all_games"].items())
	{
		// Render drive divider
		int drive_games = 0;
		for (const auto& [platform, games] : platforms.items())
			drive_games += games.size();
		string title = drive + " (" + to_string(drive_games) + " games)";
		if (!ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			continue;

		for (const auto& [platform, games] : platforms.items())
		{
			if (games.empty()) continue;

			// Load Steam games
			if (string(platform) == "Steam")
			{
				int row = 0;
				for (auto it = games.begin(); it != games.end();)
				{
					string game = it.key();
					json manifest = it.value();

					// Skip if filter is set and game name doesn't match
					if (strcmp(filter, "") != 0 && Utils::lowerString(game).find(Utils::lowerString(filter)) == string::npos)
						continue;

					// Add padding between buttons
					if (row != 0 && row % buttonsPerRow != 0)
						ImGui::SameLine(0, padding);

					if (game_images.count(game) > 0)
					{
						// Detect if mouse is hovering over button, and dim if so
						ImVec2 buttonPos = ImGui::GetCursorScreenPos();
						float dim = 1;
						if (ImGui::IsMouseHoveringRect(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y))) dim = 0.8;

						// Draw Image button
						if (ImGui::ImageButton(game_images[game]->GetDescriptorSet(), buttonSize, { 0, 0 }, { 1, 1 }, 0, { 0, 0, 0, 0 }, { 1, 1, 1, dim }))
							Steam::SelectGame(drive, game);
					}
					else
					{
						// Draw Text button
						if (ButtonCenter(game.c_str(), buttonSize))
							Steam::SelectGame(drive, game);
					}

					// Draw popup menu
					RenderPopupMenu(drive, game, manifest["appid"]);

					it++;
					row++;
				}
			}
		}
	}
}

void GameGrid::RenderPopupMenu(string drive, string game, string appid)
{
	if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1)) {
		ImGui::OpenPopup(string(appid).c_str());
	}
	if (ImGui::BeginPopup(string(appid).c_str()))
	{
		// Logic for favorites
		bool is_favorite = false;
		if (registered_games["favorite"].count(drive) > 0)
		{
			if (registered_games["favorite"][drive]["Steam"].count(game) > 0)
				is_favorite = true;
		}
		if (is_favorite)
		{
			if (ImGui::MenuItem("Unfavorite"))
			{
				registered_games["favorite"][drive]["Steam"].erase(game);
				if (registered_games["favorite"][drive]["Steam"].empty())
					registered_games["favorite"][drive].erase("Steam");
			}
		}
		else
			if (ImGui::MenuItem("Favorite"))
				registered_games["favorite"][drive]["Steam"][game] = appid;
		//end

		if (ImGui::MenuItem("Launch"))
		{
			Steam::SelectGame(drive, game);
			Steam::RunGame(appid);
		}
		if (ImGui::MenuItem("SteamDB"))
			Steam::OpenSteamDB(appid);
		if (ImGui::MenuItem("Hide"))
			Steam::RemoveGame(drive, game);
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
			//cout << selected_game.dump(4) << endl;

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

			ManifestDebug();
		}
	}
	ImGui::End();
}

void GameInfoWindow::DisplayAccount()
{
	if (selected_game["platform"] != "Steam") return;
	if (selected_game.find("selected_account") == selected_game.end())
	{
		if (!Steam::m_steamGameAccounts.empty())
			selected_game["selected_account"] = Steam::m_steamGameAccounts[0];
		else return;
	}

	// Get index of current account
	account = find(Steam::m_steamGameAccounts.begin(), Steam::m_steamGameAccounts.end(), selected_game["selected_account"]) - Steam::m_steamGameAccounts.begin();

	// Display account selection combo box
	ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
	if (ImGui::BeginCombo("##GameAccountSelect", string(Steam::m_steamGameAccounts[account]).c_str()))
	{
		for (int i = 0; i < Steam::m_steamGameAccounts.size(); i++)
		{
			bool is_selected = (account == i);
			if (ImGui::Selectable(string(Steam::m_steamGameAccounts[i]).c_str(), is_selected))
			{
				account = i;
				selected_game["selected_account"] = Steam::m_steamGameAccounts[i];
				registered_games["all_games"][selected_game["drive"]]["Steam"][selected_game["name"]]["selected_account"] = selected_game["selected_account"];
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

void AccountInfo::Render()
{
	if (ImGui::Begin("Account Info", false, ImGuiWindowFlags_NoScrollbar))
	{
		if (!Steam::m_steamAccounts.empty())
		{
			// Get index of current account
			account = find(Steam::m_steamAccounts.begin(), Steam::m_steamAccounts.end(), current_user) - Steam::m_steamAccounts.begin();

			// Display account selection combo box
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			if (ImGui::BeginCombo("##AccountSelect", string(Steam::m_steamAccounts[account]).c_str()))
			{
				for (int i = 0; i < Steam::m_steamAccounts.size(); i++)
				{
					bool is_selected = (account == i);
					if (ImGui::Selectable(string(Steam::m_steamAccounts[i]).c_str(), is_selected))
					{
						account = i;
						current_user = Steam::m_steamAccounts[i];
					}
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			if (ImGui::Button("Login", { ImGui::GetContentRegionAvail().x, 32 }))
			{
				if (current_user != Steam::GetLoginUser())
				{
					// Shutdown steam
					Steam::Exit();

					bool m_bSomeRegisterFailed = false;
					if (!Utils::Registry::WriteString(HKEY_CURRENT_USER, Steam::m_pSteamRegistry, "AutoLoginUser", current_user))
						m_bSomeRegisterFailed = true;

					if (!Utils::Registry::WriteDWORD(HKEY_CURRENT_USER, Steam::m_pSteamRegistry, "RememberPassword", 1U))
						m_bSomeRegisterFailed = true;

					if (m_bSomeRegisterFailed)
						cout << "Failed to write to registry" << endl;
					else
						Steam::Start();
				}
			}
		}
	}
	ImGui::End();
}