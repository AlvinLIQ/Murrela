#pragma once
#include <iostream>
//#include <winsock2.h>
//#include <ws2tcpip.h>
#include <windows.h>
//#include <libssh2.h>

//#pragma comment(lib,"ws2_32.lib")

const wchar_t* GetAppFolderPath()
{
#ifdef _UWP
	return Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data();
#else
	return L"\\";
#endif
}

void ReadFileFromPath(std::wstring PathToFile, void** result)
{
	FILE* fp;
	_wfopen_s(&fp, (PathToFile).c_str(), L"rb");
	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	*result = new char[fileSize];
	fread(*result, 1, fileSize, fp);
	((char*)* result)[fileSize] = '\0';
	fclose(fp);
}

void ReadFileFromAppFolderW(std::wstring PathToFile, void** result)
{
	ReadFileFromPath(GetAppFolderPath() + PathToFile, result);
}
