#ifndef UTILS_HPP
#define UTILS_HPP

#include <unordered_map>
#include <Windows.h>
#include <shlobj.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "json.hpp"
#include "thumbnail.h"

using namespace std;
using namespace Steam;
namespace fs = filesystem;
using json = nlohmann::json;

namespace utils
{
	inline shared_ptr<Walnut::Image> default_thumbnail;

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

	vector<string> SplitTextWithNewlines(const std::string& text, float maxWidth)
	{
		vector<string> result;
		istringstream text_stream(text);
		list<string> words;

		// Split text into individual words
		string word;
		while (text_stream >> word)
		{
			words.push_back(word);
		}

		// Calulate text wrapping
		string current_line;
		while (!words.empty())
		{
			word = words.front();
			words.pop_front();

			float wordWidth = ImGui::CalcTextSize((current_line + word).c_str()).x;

			if (wordWidth > maxWidth)
			{
				result.push_back(current_line);
				current_line = word;
			}
			else
			{
				current_line += word + " ";
			}
		}
		result.push_back(current_line);

		return result;
	}

	// Windows folder dialog
	string BrowseFolder()
	{
		std::wstring folderPath;

		// Initialize COM for the Windows API functions
		(void)CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		// Create an instance of the File Open Dialog
		IFileDialog* pFileDialog;
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));

		if (SUCCEEDED(hr))
		{
			// Set options for the dialog
			DWORD dwOptions;
			pFileDialog->GetOptions(&dwOptions);
			pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);

			// Show the dialog
			hr = pFileDialog->Show(NULL);

			if (SUCCEEDED(hr))
			{
				IShellItem* pItem;
				hr = pFileDialog->GetResult(&pItem);

				if (SUCCEEDED(hr))
				{
					PWSTR pszFolderPath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);

					if (SUCCEEDED(hr))
					{
						folderPath = pszFolderPath;
						CoTaskMemFree(pszFolderPath);
					}

					pItem->Release();
				}
			}

			pFileDialog->Release();
		}

		// Uninitialize COM
		CoUninitialize();

		return std::string(folderPath.begin(), folderPath.end());
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

		vector<string> textArray = SplitTextWithNewlines(label, size_arg.x);

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
}

#endif // UTILS_HPP