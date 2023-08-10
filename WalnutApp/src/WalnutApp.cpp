#pragma once
#include "includes/Includes.hpp"
#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "../images/thumbnail.h"

using namespace std;
namespace fs = filesystem;
using json = nlohmann::json;

class MainLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		// Setup
		uint32_t width, height;
		void* data = Walnut::Image::Decode(_thumbnail, sizeof(_thumbnail), width, height);
		default_thumbnail = make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA, data);
		free(data);

		if (Steam::Init())
			Steam::LoadUserData();

		// Check for saved path files
		if (fs::exists("SavedGames.json"))
		{
			ifstream file("SavedGames.json");
			registered_games = json::parse(file);
			Steam::m_steamPath = registered_games["steam_path"];
			registered_games.erase("steam_path");
			Steam::LoadGameIcons();
		}
		else
		{
			string default_path = Steam::m_steamPath + "\\steamapps";
			if (fs::exists(default_path)) {
				Steam::DetectInstalls(default_path);
				Steam::LoadGameIcons();
			}
		}

		// GUI setup
		ImGui::GetStyle().FrameRounding = 0.0f;
		if (registered_games.count("icon_size") != 0)
			gameGrid.SetIconSize((int)registered_games["icon_size"]);
	}

	virtual void OnUIRender() override
	{
		// Main window render
		gameGrid.Render();
		gameInfoWindow.Render();

		//ImGui::ShowDemoWindow();
	}

	virtual void OnDetach() override
	{
		// Add steam path to registered games
		registered_games["steam_path"] = Steam::m_steamPath;
		registered_games["icon_size"] = gameGrid.GetIconSize();

		// Save registered games
		ofstream file("SavedGames.json");
		file << registered_games.dump(4);

		registered_games.clear();
		game_images.clear();
		default_thumbnail.reset();
	}
private:
	char path[256];

	GameGrid gameGrid;
	GameInfoWindow gameInfoWindow;
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
				string path = Utils::BrowseFolder();
				if (!path.empty())
				{
					Steam::DetectInstalls(Utils::cleansePath(path));
					Steam::LoadGameIcons();
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Games"))
		{
			if (ImGui::MenuItem("Reload Game Icons"))
			{
				Steam::LoadGameIcons();
			}
			if (ImGui::MenuItem("Launch Random Game"))
			{
				if (!registered_games.empty())
				{
					int randomIndex = rand() % registered_games.size();
					auto it = registered_games.begin();
					advance(it, randomIndex);
					Steam::RunGame(it.value()["appid"]);
				}
			}
			ImGui::EndMenu();
		}
	});

	return app;
}