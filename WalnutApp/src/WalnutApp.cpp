#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"

#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

#include "../json.hpp"
#include "../Steam.h"
#include "../ManifestParser.hpp"

using namespace std;
using namespace Steam;
using namespace m_parser;
using json = nlohmann::json;

string cleansePath(const string& path)
{
	string output = path;
	for (size_t i = 0; i < output.length(); ++i) {
		if (output[i] == '\\') {
			output[i] = '/';
		}
	}
	return output;
}

class MainLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Add game path");
		ImGui::InputText("Path", path, IM_ARRAYSIZE(path));
		if (ImGui::Button("Add")) {
			DetectInstalls(cleansePath(path));
		}
		ImGui::End();

		ImGui::Begin("Detected Games");
		for (const auto& [game, manifest] : registered_games.items()) {
			ImGui::Text("%s", game.c_str());
		}
		ImGui::End();

		//ImGui::ShowDemoWindow();
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
	app->SetMenubarCallback([app, mainLayer](){});

	return app;
}