#pragma once

using namespace std;
using namespace ImGui;
using json = nlohmann::json;

namespace Utils
{
	string lowerString(string str);

	string upperString(string str);

	string cleansePath(const string& path);

	vector<string> SplitTextWithNewlines(const string& text, float maxWidth);

	string BrowseFolder();

	unsigned int GetProcessID(const char* m_pName);

	namespace EnumWindows
	{
		extern std::vector<HWND> m_vList;

		BOOL __stdcall GetListForWindowClass(HWND m_hWindow, LPARAM m_pParam);
	}

	namespace Registry
	{
		unsigned int GetDWORD(HKEY m_hKey, const char* m_pPath, const char* m_pKey);

		std::string GetString(HKEY m_hKey, const char* m_pPath, const char* m_pKey);

		bool WriteDWORD(HKEY m_hKey, const char* m_pPath, const char* m_pKey, unsigned int m_uValue);

		bool WriteString(HKEY m_hKey, const char* m_pPath, const char* m_pKey, std::string m_sValue);
	}
}