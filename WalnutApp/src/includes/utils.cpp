#include <unordered_map>
#include <Windows.h>
#include <shlobj.h>
#include <iostream>
#include <sstream>
#include <vector>

#include "utils.h"

using namespace std;
using namespace ImGui;
namespace fs = filesystem;
using json = nlohmann::json;

namespace Utils
{
	string lowerString(string str)
	{
		string output = str;
		transform(output.begin(), output.end(), output.begin(), ::tolower);
		return output;
	}

	string cleansePath(const string& path)
	{
		string output = path;

		for (size_t i = 0; i < output.length(); ++i) {
			// Replace backslashes with forward slashes
			if (output[i] == '/') {
				output[i] = '\\';
			}

			// Check for sequences of consecutive slashes
			if (i > 0 && output[i] == '\\' && output[i - 1] == '\\') {
				output.erase(i, 1); // Remove the extra slash
				--i; // Adjust the index
			}
		}

		return output;
	}

	vector<string> SplitTextWithNewlines(const string& text, float maxWidth)
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
		wstring folderPath;

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

		return string(folderPath.begin(), folderPath.end());
	}
}