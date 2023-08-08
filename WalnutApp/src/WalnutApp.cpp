#include "../includes.h"

using namespace std;
using namespace Steam;
using namespace utils;
using namespace m_parser;
using json = nlohmann::json;

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

class MainLayer : public Walnut::Layer
{
public:
	virtual void OnAttach() override
	{
		ImGui::GetStyle().FrameRounding = 0.0f;
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Add game path");
		ImGui::InputText("Path", path, IM_ARRAYSIZE(path));
		if (ImGui::Button("Add")) {
			DetectInstalls(cleansePath(path));
			memset(path, 0, IM_ARRAYSIZE(path));
		}
		ImGui::End();

		ImGui::Begin("Game Browser");
		RenderGameGrid({ 120, 180 });
		ImGui::End();
	}

	virtual void OnDetach() override
	{
		registered_games.clear();
		game_images.clear();
	}
private:
	char path[256];
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Game Manager";
	spec.CustomTitlebar = true;

	Walnut::Application* app = new Walnut::Application(spec);
	std::shared_ptr<MainLayer> mainLayer = std::make_shared<MainLayer>();
	app->PushLayer(mainLayer);
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});

	return app;
}