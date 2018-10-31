#include <Windows.h>
#include <WinInet.h>
#include <iostream>
#include <string>
#include <sstream>
#include <strsafe.h>
#include <vector>
#include <fstream>
#include <ctime>
#include "curl/curl.h"
#pragma comment(lib,"Version.lib")
#pragma comment(lib,"Wininet.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

std::string file_version(TCHAR *);
static size_t WriteCallback(void *, size_t, size_t, void *);
bool is_file_exist(const char *);
std::string unix_timestamp_a_week_later();
std::string unix_timestamp_now();
std::string exe_location();
bool is_empty(std::ifstream&);

int main()
{
	// Settings location 
	std::string exe_path = exe_location();
	std::string settings_path = exe_path + "\\settings.dat";
	std::string seven_zip_path = exe_path + "\\7zFM.exe";
	// Filestreaming
	std::ofstream exe_settings_write;
	std::ifstream exe_settings_read;
	// Settings preperation
	std::vector<std::string> settings_vector;
	std::string current_time = unix_timestamp_now();
	// Setting variables
	std::string file_path = seven_zip_path;
	std::string check_interval = "0";
	std::string current_version;
	std::string latest_fetch_version;

	if (!is_file_exist(settings_path.c_str())) {
		exe_settings_write.open(settings_path);
		exe_settings_write << seven_zip_path << "\n";
		std::string target = unix_timestamp_a_week_later();
		exe_settings_write << target << "\n";
		exe_settings_write << "0.0" << "\n";
		exe_settings_write << "0.1" << "\n";
		exe_settings_write.close();
	}
	else {
		exe_settings_read.open(settings_path);
		std::string line;
		while (std::getline(exe_settings_read, line)) {
			if (line.size() > 0)
				settings_vector.emplace_back(line);
		}
		file_path = settings_vector[0];
		check_interval = settings_vector[1];
		current_version = settings_vector[2];
		latest_fetch_version = settings_vector[3];
		exe_settings_read.close();
	}

	TCHAR* file_path_tchar = (TCHAR*)file_path.c_str();
	if (is_file_exist(file_path_tchar) && (std::stol(check_interval) > std::stol(current_time))/* && (current_version == latest_fetch_version)*/) {
		ShellExecute(NULL, "open", file_path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
		exit(0);
	}
	if (!is_file_exist(file_path_tchar)) {
		file_path.clear();
		file_path = seven_zip_path;
		file_path_tchar = (TCHAR*)file_path.c_str();
	}
	if (!is_file_exist(file_path_tchar)) {
		std::cout << "Couldn\'t find 7-Zip at usual installation locations.\n";
		std::cout << "Enter the path for the 7-Zip installation folder.\n\n";
		std::cout << "Example path: C:\\Program Files\\7-Zip\n";
		std::cout << "Your path   : ";
		//file_path.clear();
		// std::getline(std::cin, file_path); // disabled since console GUI is disabled

		if (file_path.back() != '\\') {
			file_path = file_path + "\\7zFM.exe";
			file_path_tchar = (TCHAR*)file_path.c_str();
		}
		else if (file_path.back() == '\\') {
			file_path = file_path + "7zFM.exe";
			file_path_tchar = (TCHAR*)file_path.c_str();
		}
		else {
			std::cout << "\nInvalid path, exiting.\n";
			Sleep(1000);
			exit(1);
		}
	}
	if (!is_file_exist(file_path_tchar)) {
		int msgboxID = MessageBox(
			0,
			"Couldn\'t find 7-Zip, would you like to edit settings to provide your own location?",
			"Cannot find 7-Zip!",
			MB_ICONEXCLAMATION | MB_YESNO
		);
		if (msgboxID == IDYES) {
			std::string launch_notepad_on_settings = "notepad.exe " + settings_path;
			system(launch_notepad_on_settings.c_str());
			exit(1);
		}
		else {
			exit(1);
		}
		Sleep(1000);
		exit(2);
	}
	std::string seven_zip_version = file_version(file_path_tchar);
	std::string seven_zip_major_version = seven_zip_version.substr(0, 2);
	std::string seven_zip_minor_version = seven_zip_version.substr(3, 5);

	if (!InternetCheckConnection("https://www.7-zip.org", FLAG_ICC_FORCE_CONNECTION, 0))
	{
		std::cout << "Connection to 7-Zip.org is failed.\nPlease check your internet connection.\n\n";
		ShellExecute(NULL, "open", file_path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
		exit(0);
	}

	CURL *curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.7-zip.org/download.html");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	std::istringstream f(readBuffer);
	std::string line;
	std::vector<std::string> version_lines;
	while (std::getline(f, line)) {
		if (line.find("Download 7-Zip") != std::string::npos) {
			line = line.substr(0, line.find(" ("));
			line.erase(0, 21);
			version_lines.emplace_back(line);
		}
		else
			continue;
		if (is_file_exist(settings_path.c_str())) {
			exe_settings_read.open(settings_path);
			std::string line;
			while (std::getline(exe_settings_read, line)) {
				if (line.size() > 0)
					settings_vector.emplace_back(line);
			}
			file_path = settings_vector[0];
			check_interval = settings_vector[1];
			current_version = settings_vector[2];
			latest_fetch_version = settings_vector[3];
			exe_settings_read.close();
		}
		if (is_file_exist(settings_path.c_str())) {
			exe_settings_write.open(settings_path);
			exe_settings_write << settings_vector[0] << "\n";
			exe_settings_write << settings_vector[1] << "\n";
			exe_settings_write << seven_zip_version << "\n";
			exe_settings_write << line << "\n";
			exe_settings_write.close();
		}
		std::cout << "\n--- 7-Zip found! ---\n\n";
		if (stoi(line.substr(0, 2)) > stoi(seven_zip_major_version)) {
			// char selection; // disabled console version, so no need for this anymore.
			std::cout << "There\'s a major version upgrade for 7-Zip!\n";
			std::cout << "Newest version: " + line + "\n";
			std::cout << "Your version  : " + seven_zip_version + "\n\n";
			std::cout << "Next check will happen 1 week later.\n";
			std::cout << "Would you like to update 7-Zip? (y/n): ";
			std::string message_box_text = "There\'s a major version upgrade for 7-Zip!\nWould you like to update 7-Zip?\n\nNewest version: " + line + "\nYour version     : " + seven_zip_version + "\n\nNext check will happen 1 week later.";
			int msgboxID = MessageBox(
				0,
				message_box_text.c_str(),
				"Update found for 7-Zip!",
				MB_ICONEXCLAMATION | MB_YESNO
			);
			if (msgboxID == IDYES) {
				ShellExecute(0, 0, "https://www.7-zip.org/download.html", 0, 0, SW_SHOW);
				break;
			}
			else {
				ShellExecute(NULL, "open", file_path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
				break;
			}
		}
		else if (stoi(line.substr(3, 5)) > stoi(seven_zip_minor_version)) {
			// char selection; // disabled console version, so no need for this anymore.
			std::cout << "There\'s a minor version upgrade for 7-Zip!\n";
			std::cout << "Newest version: " + line + "\n";
			std::cout << "Your version  : " + seven_zip_version + "\n\n";
			std::cout << "Would you like to update 7-Zip? (y/n): ";
			std::string message_box_text = "There\'s a minor version upgrade for 7-Zip!\nWould you like to update 7-Zip?\n\nNewest version: " + line + "\nYour version     : " + seven_zip_version + "\n\nNext check will happen 1 week later.";
			int msgboxID = MessageBox(
				0,
				message_box_text.c_str(),
				"Update found for 7-Zip!",
				MB_ICONEXCLAMATION | MB_YESNO
			);
			if (msgboxID == IDYES) {
				ShellExecute(0, 0, "https://www.7-zip.org/download.html", 0, 0, SW_SHOW);
				break;
			}
			else {
				ShellExecute(NULL, "open", file_path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
				break;
			}
		}
		else {
			ShellExecute(NULL, "open", file_path.c_str(), NULL, NULL, SW_SHOWDEFAULT);
			break;
		}
	}
}

std::string file_version(TCHAR *pszFilePath)
{
	DWORD               dwSize = 0;
	BYTE                *pbVersionInfo = NULL;
	VS_FIXEDFILEINFO    *pFileInfo = NULL;
	UINT                puLenFileInfo = 0;

	// Get the version information for the file requested
	dwSize = GetFileVersionInfoSize(pszFilePath, NULL);
	if (dwSize == 0)
	{
		printf("Error in GetFileVersionInfoSize: %d\n", GetLastError());
		return "";
	}

	pbVersionInfo = new BYTE[dwSize];

	if (!GetFileVersionInfo(pszFilePath, 0, dwSize, pbVersionInfo))
	{
		printf("Error in GetFileVersionInfo: %d\n", GetLastError());
		delete[] pbVersionInfo;
		return "";
	}

	if (!VerQueryValue(pbVersionInfo, TEXT("\\"), (LPVOID*)&pFileInfo, &puLenFileInfo))
	{
		printf("Error in VerQueryValue: %d\n", GetLastError());
		delete[] pbVersionInfo;
		return "";
	}

	// pFileInfo->dwFileVersionMS is usually zero. However, you should check
	// this if your version numbers seem to be wrong

	int major_version = (pFileInfo->dwFileVersionMS >> 16) & 0xff;
	int minor_version = (pFileInfo->dwFileVersionMS >> 0) & 0xff;

	std::string version_string = std::to_string(major_version) + '.' + std::to_string(minor_version);
	return version_string;
}

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

bool is_file_exist(const char *fileName)
{
	std::ifstream infile(fileName);
	return infile.good();
}

std::string unix_timestamp_a_week_later()
{
	time_t t = std::time(0);
	long int now = static_cast<long int> (t);
	now += 604800;
	std::string return_value = std::to_string(now);
	return return_value;
}

std::string unix_timestamp_now()
{
	time_t t = std::time(0);
	long int now = static_cast<long int> (t);
	std::string return_value = std::to_string(now);
	return return_value;
}

std::string exe_location()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

bool is_empty(std::ifstream& pFile)
{
	return pFile.peek() == std::ifstream::traits_type::eof();
}
