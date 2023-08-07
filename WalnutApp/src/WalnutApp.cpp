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

std::string SplitTextWithNewlines(const std::string& text, float maxWidth)
{
	std::string result;
	std::string currentLine;
	float currentWidth = 0.0f;
	size_t lastSpaceIdx = 0;

	for (size_t i = 0; i < text.size(); ++i)
	{
		char c = text[i];
		float charWidth = ImGui::CalcTextSize(&c, &c + 1).x;

		// If adding the current character exceeds the maximum width, handle newline insertion
		if (currentWidth + charWidth > maxWidth)
		{
			if (isspace(c) && i > lastSpaceIdx)
			{
				// Insert newline at the last space character
				result += currentLine.substr(0, lastSpaceIdx) + "\n";
				currentLine = currentLine.substr(lastSpaceIdx + 1);
				currentWidth = ImGui::CalcTextSize(currentLine.c_str()).x;
				lastSpaceIdx = 0;
			}
			else
			{
				// If there's no space character nearby, insert a newline
				result += currentLine + "\n";
				currentLine.clear();
				currentWidth = 0.0f;
			}
		}

		currentLine += c;
		currentWidth += charWidth;

		if (isspace(c))
			lastSpaceIdx = i;
	}

	if (!currentLine.empty())
		result += currentLine;

	return result;
}

void RenderGameGrid(ImVec2 buttonSize)
{
	// Calculate the number of buttons per row and column based on paretn frames available width and height
	int buttonsPerRow = static_cast<int>(ImGui::GetContentRegionAvail().x / (buttonSize.x + 8));
	int remainingWidth = ImGui::GetContentRegionAvail().x - buttonsPerRow * (buttonSize.x + 8);
	float padding = remainingWidth / (buttonsPerRow - 1);

	int row = 0;
	for (const auto& [game, manifest] : registered_games.items()) {
		if (row != 0 && row % buttonsPerRow != 0)
			ImGui::SameLine(0, padding);

		if (game_images.count((string)manifest["name"]) > 0)
		{
			ImGui::ImageButton(game_images[(string)manifest["name"]]->GetDescriptorSet(), buttonSize, { 0, 0 }, { 1, 1 }, 0);
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
	virtual void OnUIRender() override
	{
		ImGui::Begin("Add game path");
		ImGui::InputText("Path", path, IM_ARRAYSIZE(path));
		if (ImGui::Button("Add")) {
			DetectInstalls(cleansePath(path));
			memset(path, 0, IM_ARRAYSIZE(path));
		}
		ImGui::End();

		ImGui::Begin("Detected Games");
		RenderGameGrid({120, 180});
		ImGui::End();

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