// Paws.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "Paws.h"
#include <string>

#include <Windows.h>
#include <conio.h>

using namespace std;

enum Flags { Quiet, Verbose };

// Forward Declarations:
void DisplayUsage();
wstring GetErrorMessage();

int wmain(int argc, wchar_t* argv[])
{
	if (argc < 2)
	{
		DisplayUsage();
	}
	else
	{
		// Extract the command-line sans the name of this executable:
		wstring strFullCommandLine{ (GetCommandLine()) };
		wstring strThisExe{ argv[0] };
		strThisExe.append(L" ");
		size_t nStart{ strThisExe.length() + 2 };
		size_t nLength{ strFullCommandLine.length() - strThisExe.length() - 1 };
		wstring strCommandLine{ strFullCommandLine.substr(nStart, nLength) };

		wcout << "Executing '" << strCommandLine << "'" << endl;

		STARTUPINFO si{};
		si.cb = sizeof(si);

		PROCESS_INFORMATION pi{};

		if (CreateProcess(
			NULL,
			const_cast<LPWSTR>(strCommandLine.c_str()),
			NULL,
			NULL,
			TRUE,
			0,
			NULL,
			NULL,
			&si,
			&pi))
		{
			WaitForSingleObject(pi.hProcess, 10*1000);

			wcout << endl;
			wcout << "Press any key to exit ...";
			while (0 == _kbhit());
		}
		else
		{
			wcout << L"Error: Failed to execute the requested executable:" << GetErrorMessage() << endl;
		}
	}
}

void DisplayUsage()
{
	wcout << "Paws - Runs specified executable, and then pauses before closing" << endl;
	wcout << "Usage: Paws[.exe] <executable>" << endl;
	wcout << endl;
	wcout << "    <executable> : Name or absolute path to the executable" << endl;
	wcout << "                   to be run. If name is used, executable must" << endl;
	wcout << "                   be reachable on the current path" << endl;
	wcout << endl;
}

wstring GetErrorMessage()
{
	// Retrieve the system error message for the last-error code

	LPWSTR lpMsgBuf{};
	DWORD dwErrorCode{ GetLastError() };

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		lpMsgBuf,
		0, NULL);

	wstring msg{ lpMsgBuf };
	
	LocalFree(lpMsgBuf);

	return msg;
}
