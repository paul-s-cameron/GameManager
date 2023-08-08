#pragma once
#include <imgui_internal.h>

#include "imgui.h"
#include "json.hpp"

using namespace std;
using namespace ImGui;
using json = nlohmann::json;

namespace utils
{
	void DisplayJSON(const json& jsonData);

	string cleansePath(const string& path);

	vector<string> SplitTextWithNewlines(const string& text, float maxWidth);

	string BrowseFolder();

	bool ButtonCenter(const char* label, const ImVec2& size_arg);
}