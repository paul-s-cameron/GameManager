#pragma once
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include <filesystem>
#include <Windows.h>
#include <imgui.h>
#include <fstream>

#include <thumbnail.h>
#include "includes/utils.h"
#include "includes/json.hpp"
#include "includes/globals.h"
#include "includes/Steam/Steam.h"
#include <GLFW/glfw3.h>
#include "../../vendor/stb_image/stb_image.h"

using namespace std;
using namespace utils;
using namespace Steam;
namespace fs = filesystem;
using json = nlohmann::json;

void RenderGameGrid(ImVec2 buttonSize, const char* filter)
{
	if (registered_games.empty() || game_images.empty())
		return;

	int buttonsPerRow = static_cast<int>(ImGui::GetContentRegionAvail().x / (buttonSize.x + 8));
	if (buttonsPerRow < 2) return;
	int remainingWidth = ImGui::GetContentRegionAvail().x - (buttonsPerRow * buttonSize.x);
	float padding = remainingWidth / (buttonsPerRow - 1);

	ImFont* font = ImGui::GetFont();
	float originalFontSize = font->Scale;
	float scale = buttonSize.x / 120;
	font->Scale = scale;
	float dim = 1;

	for (const auto& [drive, _] : registered_games.items())
	{
		string title = drive + " (" + to_string(registered_games[drive].size()) + ")";
		if (!ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			continue;

		ImGui::SetCurrentFont(font);
		int row = 0;
		for (auto it = registered_games[drive].begin(); it != registered_games[drive].end(); ) {
			const auto& entry = *(it++);
			string game = entry["name"];
			json manifest = entry;

			string name_lowered = game;
			string filter_lowered = filter;
			transform(name_lowered.begin(), name_lowered.end(), name_lowered.begin(), ::tolower);
			transform(filter_lowered.begin(), filter_lowered.end(), filter_lowered.begin(), ::tolower);

			if (strcmp(filter, "") != 0 && name_lowered.find(filter_lowered) == string::npos)
				continue;
			if (row != 0 && row % buttonsPerRow != 0)
				ImGui::SameLine(0, padding);

			ImVec2 buttonPos = ImGui::GetCursorScreenPos();

			if (game_images.count(manifest["name"]) > 0)
			{
				if (ImGui::IsMouseHoveringRect(buttonPos, ImVec2(buttonPos.x + buttonSize.x, buttonPos.y + buttonSize.y)))
					dim = 0.8;
				else
					dim = 1;
				if (ImGui::ImageButton(game_images[manifest["name"]]->GetDescriptorSet(), buttonSize, { 0, 0 }, { 1, 1 }, 0, {0, 0, 0, 0}, {1, 1, 1, dim}))
					selected_game = manifest;
			}
			else
			{
				if (ButtonCenter(game.c_str(), buttonSize))
					selected_game = manifest;
			}
			if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(1)) {
				ImGui::OpenPopup(string(manifest["appid"]).c_str());
			}
			if (ImGui::BeginPopup(string(manifest["appid"]).c_str()))
			{
				if (ImGui::MenuItem("Launch"))
					RunGame(manifest["appid"]);
				if (ImGui::MenuItem("SteamDB"))
					OpenSteamDB(manifest["appid"]);
				if (ImGui::MenuItem("Remove"))
					RemoveGame(drive, manifest["name"]);
				ImGui::EndPopup();
			}

			row++;
		}
		font->Scale = originalFontSize;
		ImGui::SetCurrentFont(font);
	}
}

void RenderGameInfo()
{
	if (ImGui::Begin("Game Info", false, ImGuiWindowFlags_NoScrollbar))
	{
		if (!selected_game.empty())
		{
			// Get game icon
			ImVec2 iconSize = { 120, 190 };
			shared_ptr<Walnut::Image> game_icon;
			if (game_images.count(selected_game["name"]) != 0)
			{
				game_icon = game_images[selected_game["name"]];
			}
			else
			{
				game_icon = default_thumbnail;
			}

			// Render game icon at center
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (iconSize.x / 2));
			ImGui::Image(game_icon.get()->GetDescriptorSet(), iconSize);
			ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x / 2) - (ImGui::CalcTextSize(string(selected_game["name"]).c_str()).x / 2));
			ImGui::Text(string(selected_game["name"]).c_str());

			ImGui::Separator();

			float buttonWidth = ImGui::GetContentRegionAvail().x / 2 - 5;
			if (ImGui::Button("Play", { buttonWidth, 50 }))
				RunGame(selected_game["appid"]);

			ImGui::SameLine(0, 5);

			if (ImGui::Button("SteamDB", { buttonWidth, 50 }))
			{
				OpenSteamDB(selected_game["appid"]);
			}

			if (ImGui::TreeNode("Manifest"))
			{
				DisplayJSON(selected_game);
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

class MainLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		// Setup
		//GLFWimage icon;
		//int channels;
		//std::string iconPathStr = "E:\\Coding\\C++\\Applications\\GameManager\\WalnutApp\\Icon.png";
		//icon.pixels = stbi_load(iconPathStr.c_str(), &icon.width, &icon.height, &channels, 4);
		//// get glfw window handle

		uint32_t width, height;
		void* data = Walnut::Image::Decode(_thumbnail, sizeof(_thumbnail), width, height);
		default_thumbnail = make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA, data);
		free(data);

		//glfwSetWindowIcon(m_WindowHandle, 1, &icon);
		//stbi_image_free(icon.pixels)
		steam_path = "C:\\Program Files (x86)\\Steam\\steam.exe";

		// Check for saved path files
		if (fs::exists("SteamPaths.json"))
		{
			ifstream file("SteamPaths.json");
			file >> registered_games;
			LoadGameIcons();
		}
		else
		{
			string default_path = "C:\\Program Files (x86)\\Steam\\steamapps";
			if (fs::exists(default_path)) {
				DetectInstalls(default_path);
				LoadGameIcons();
			}
		}

		// GUI setup
		ImGui::GetStyle().FrameRounding = 0.0f;
	}

	virtual void OnUIRender() override
	{
		if (ImGui::Begin("Game Browser", false))
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 180);
			ImGui::InputTextWithHint("##Input", "Search", filter, IM_ARRAYSIZE(filter));
			ImGui::SameLine();
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::SliderFloat("##IconSizeSlider", &iconSize, 60, 360);
			ImGui::PopItemWidth();
			// TODO: Add option to select different image types (thumbnail/banner)
			ImGui::BeginChild("##GameGrid", { 0, 0 }, true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			RenderGameGrid({ iconSize, (float)(iconSize * 1.5) }, filter);
			ImGui::EndChild();
		}
		ImGui::End();

		RenderGameInfo();

		//ImGui::ShowDemoWindow();
	}

	virtual void OnDetach() override
	{
		// Save registered games
		ofstream file("SteamPaths.json");
		file << registered_games.dump(4);

		registered_games.clear();
		game_images.clear();
		default_thumbnail.reset();
	}
private:
	float iconSize = 120;
	char path[256];
	char filter[256];
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Game Manager";
	spec.CustomTitlebar = true;
	spec.CenterWindow = true;

	Walnut::Application* app = new Walnut::Application(spec);
	std::shared_ptr<MainLayer> mainLayer = std::make_shared<MainLayer>();
	app->PushLayer(mainLayer);
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Add Game Folder"))
			{
				string path = BrowseFolder();
				if (!path.empty())
				{
					DetectInstalls(cleansePath(path));
					LoadGameIcons();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Games"))
		{
			if (ImGui::MenuItem("Reload Game Icons"))
			{
				LoadGameIcons();
			}
			if (ImGui::MenuItem("Launch Random Game"))
			{
				if (!registered_games.empty())
				{
					int randomIndex = rand() % registered_games.size();
					auto it = registered_games.begin();
					advance(it, randomIndex);
					RunGame(it.value()["appid"]);
				}
			}
			ImGui::EndMenu();
		}
	});

	return app;
}