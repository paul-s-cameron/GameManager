#include "Includes.hpp"

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

	string upperString(string str)
	{
		string output = str;
		transform(output.begin(), output.end(), output.begin(), ::toupper);
		return output;
	}

	string cleansePath(const string& path)
	{
		string output = path;

		// Check if path contains a drive letter
		if (output.length() >= 2 && output[1] == ':') {
			// Convert drive letter to uppercase
			output[0] = toupper(output[0]);
		}

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

	unsigned int GetProcessID(const char* m_pName)
	{
		unsigned int m_uProcessID = 0U;

		HANDLE m_hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (m_hSnapshot)
		{
			PROCESSENTRY32 m_pEntry32;
			m_pEntry32.dwSize = sizeof(PROCESSENTRY32);
			if (Process32First(m_hSnapshot, &m_pEntry32))
			{
				while (Process32Next(m_hSnapshot, &m_pEntry32))
				{
					wchar_t wName[MAX_PATH];
					MultiByteToWideChar(CP_ACP, 0, m_pName, -1, wName, MAX_PATH);

					if (wcsstr(m_pEntry32.szExeFile, wName))
					{
						m_uProcessID = m_pEntry32.th32ProcessID;
						break;
					}
				}
			}

			CloseHandle(m_hSnapshot);
		}

		return m_uProcessID;
	}

	namespace EnumWindows
	{
		std::vector<HWND> m_vList;

		BOOL __stdcall GetListForWindowClass(HWND m_hWindow, LPARAM m_pParam)
		{
			char m_cClassName[128];
			GetClassNameA(m_hWindow, m_cClassName, sizeof(m_cClassName));

			if (strcmp(m_cClassName, reinterpret_cast<const char*>(m_pParam)) == 0)
				m_vList.emplace_back(m_hWindow);

			return TRUE;
		}
	}

	namespace Registry
	{
		unsigned int GetDWORD(HKEY m_hKey, const char* m_pPath, const char* m_pKey)
		{
			unsigned int m_uReturn = 0U;

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				DWORD m_dDataSize = sizeof(m_uReturn);
				RegQueryValueExA(m_hRegistryKey, m_pKey, 0, 0, LPBYTE(&m_uReturn), &m_dDataSize);

				RegCloseKey(m_hRegistryKey);
			}

			return m_uReturn;
		}

		std::string GetString(HKEY m_hKey, const char* m_pPath, const char* m_pKey)
		{
			std::string m_sReturn(128, '\0');

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				DWORD m_dDataSize = m_sReturn.size();
				if (RegQueryValueExA(m_hRegistryKey, m_pKey, 0, 0, LPBYTE(&m_sReturn[0]), &m_dDataSize) == ERROR_SUCCESS)
					m_sReturn.resize(m_dDataSize - 1);
				else
					m_sReturn.clear();

				RegCloseKey(m_hRegistryKey);
			}

			return m_sReturn;
		}

		bool WriteDWORD(HKEY m_hKey, const char* m_pPath, const char* m_pKey, unsigned int m_uValue)
		{
			bool m_bSuccess = false;

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				if (RegSetValueExA(m_hRegistryKey, m_pKey, 0, REG_DWORD, LPBYTE(&m_uValue), sizeof(m_uValue)) == ERROR_SUCCESS)
					m_bSuccess = true;

				RegCloseKey(m_hRegistryKey);
			}

			return m_bSuccess;
		}

		bool WriteString(HKEY m_hKey, const char* m_pPath, const char* m_pKey, std::string m_sValue)
		{
			bool m_bSuccess = false;

			HKEY m_hRegistryKey;
			if (RegOpenKeyA(m_hKey, m_pPath, &m_hRegistryKey) == ERROR_SUCCESS)
			{
				if (RegSetValueExA(m_hRegistryKey, m_pKey, 0, REG_SZ, LPBYTE(&m_sValue[0]), m_sValue.size() + 1) == ERROR_SUCCESS)
					m_bSuccess = true;

				RegCloseKey(m_hRegistryKey);
			}

			return m_bSuccess;
		}
	}
}