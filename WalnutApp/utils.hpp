#ifndef UTILS_HPP
#define UTILS_HPP

#include "includes.h"

using json = nlohmann::json;

namespace utils
{
	void OpenFileWithImGuiFileDialog(std::string& selectedFilePath)
	{
		OPENFILENAME ofn;
		char fileName[MAX_PATH] = "";

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = (LPCWSTR)"All Files (*.*)\0*.*\0";
		ofn.lpstrFile = (LPWSTR)fileName;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		if (GetOpenFileName(&ofn))
		{
			selectedFilePath = fileName;
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

	std::string SplitTextWithNewlines(const std::string& text, float maxWidth)
	{
		std::string result;

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
				result += current_line + "\n";
				current_line = word;
			}
			else
			{
				current_line += word + " ";
			}
		}
		result += current_line;

		return result;
	}
}
#endif // UTILS_HPP