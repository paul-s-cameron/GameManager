#pragma once
#include <imgui_internal.h>

#include "imgui.h"
#include "json.hpp"

using namespace std;
using namespace ImGui;
using json = nlohmann::json;

namespace Utils
{
	string lowerString(string str);

	string cleansePath(const string& path);

	vector<string> SplitTextWithNewlines(const string& text, float maxWidth);

	string BrowseFolder();
}