#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <random>
#include <locale>

void SetCurrentPathToBaseDirectory()
{
	TCHAR buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::filesystem::current_path(std::filesystem::path(buffer).parent_path());
}

void ReadFirstLine(const std::wstring& file, std::wstring& line)
{
	auto fstream{ std::wifstream(file) };
	if (!fstream.good())
		return;

	std::getline(fstream, line);
	fstream.close();
}

void ReadAllLines(const std::wstring& file, std::vector<std::wstring>& lines)
{
	auto fstream{ std::wifstream(file) };

	if (fstream.good())
	{
		std::wstring line;
		while (std::getline(fstream, line))
			lines.push_back(line);
	}

	fstream.close();
}

void AppendLine(const std::wstring& file, const std::wstring& line)
{
	auto fstream{ std::wofstream(file, std::ios::out | std::ios::app) };
	fstream << line << std::endl;
	fstream.close();
}

void ClearFile(const std::wstring& file)
{
	std::wofstream(file, std::ios::out | std::ios::trunc).close();
}

bool LastExec(const std::wstring& file)
{
	auto now{ std::chrono::system_clock::now() };
	time_t nowc{ std::chrono::system_clock::to_time_t(now) };
	tm nowtm;
	localtime_s(&nowtm, &nowc);
	TCHAR nowBuffer[16];
	std::wcsftime(nowBuffer, 16, L"%Y%m%d", &nowtm);

	auto fstream{ std::wfstream(file, std::ios::in) };
	bool retVal;
	if (!fstream.good())
	{
		fstream.close();
		fstream = std::wfstream(file, std::ios::out);
		fstream << nowBuffer;
		retVal = true;
	}
	else
	{
		TCHAR lastExecBuffer[16];
		fstream >> lastExecBuffer;
		
		if (std::wcscmp(nowBuffer, lastExecBuffer) > 0)
		{
			fstream.close();
			fstream = std::wfstream(file, std::ios::out | std::ios::trunc);
			fstream << nowBuffer;
			retVal = true;
		}
		else
			retVal = false;
	}

	fstream.close();
	return retVal;
}

void GetFilesInDirectory(const std::wstring& directory, std::vector<std::wstring>& files)
{
	for (const auto& entry : std::filesystem::directory_iterator(directory))
		files.push_back(entry.path().wstring());
}

template<typename Iter>
Iter GetRandomItem(Iter start, Iter end)
{
	std::random_device randomDevice;
	auto randomGenerator{ std::mt19937(randomDevice()) };
	auto distribution{ std::uniform_int_distribution<>(0, std::distance(start, end) - 1) };
	std::advance(start, distribution(randomGenerator));
	return start;
}

constexpr auto LASTEXEC_FILE{ L"LastExec.txt" };
constexpr auto FOLDERPATH_FILE{ L"FolderPath.txt" };
constexpr auto LASTWALLPAPERS_FILE{ L"LastWallpapers.txt" };

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR pCmdLine, _In_ int)
{
	std::locale::global(std::locale(".UTF-8"));

	SetCurrentPathToBaseDirectory();

	if (std::wstring(pCmdLine).find(L"force") == std::wstring::npos)
	{
		if (!LastExec(LASTEXEC_FILE))
			return 0;
	}

	std::wstring folderPath;
	ReadFirstLine(FOLDERPATH_FILE, folderPath);

	if (folderPath.empty())
		return 1;

	std::vector<std::wstring> lastWallpapers;
	ReadAllLines(LASTWALLPAPERS_FILE, lastWallpapers);
	std::sort(lastWallpapers.begin(), lastWallpapers.end());

	std::vector<std::wstring> wallpapers;
	GetFilesInDirectory(folderPath, wallpapers);
	std::sort(wallpapers.begin(), wallpapers.end());

	if (lastWallpapers.size() >= wallpapers.size())
	{
		ClearFile(LASTWALLPAPERS_FILE);
		lastWallpapers.clear();
	}

	std::vector<std::wstring> validWallpapers;
	std::set_difference(wallpapers.begin(), wallpapers.end(), lastWallpapers.begin(), lastWallpapers.end(), std::back_inserter(validWallpapers));

	const std::wstring& nextWallpaper{ *GetRandomItem(validWallpapers.begin(), validWallpapers.end()) };

	AppendLine(LASTWALLPAPERS_FILE, nextWallpaper);

	SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, const_cast<LPTSTR>(nextWallpaper.c_str()), SPIF_UPDATEINIFILE);

	return 0;
}