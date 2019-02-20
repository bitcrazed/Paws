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
void DisplayUsage(bool fVTEnabled);
void PauseBeforeClosing();
bool EnableVTProcessing();
wstring GetErrorMessage();

// Globals:
wstring szVTReset{};
wstring szVTYellow{};
wstring szVTGreen{};
wstring szVTRed{};

int wmain(int argc, wchar_t* argv[])
{
	// Enable Console VT Processing
	bool fVTEnabled = EnableVTProcessing();

	// Configure color strings if VT is enabled
	if (fVTEnabled)
	{
		szVTReset.assign(L"\x1b[m");
		szVTYellow.assign(L"\x1b[33m");
		szVTGreen.assign(L"\x1b[32m");
		szVTRed.assign(L"\x1b[31m");
	}

	if (argc < 2)
	{
		DisplayUsage(fVTEnabled);
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

		// Launch the requested command
		wcout << szVTYellow << "Paws is executing: '" << szVTGreen << strCommandLine << szVTYellow << "'" << szVTReset << endl;
		STARTUPINFO si{si.cb = sizeof(STARTUPINFO)};
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
			// Wait for the executable to complete
			WaitForSingleObject(pi.hProcess, INFINITE);
		}
		else
		{
			wcout << szVTRed << L"Error: Paws failed to execute:" << GetErrorMessage() << szVTReset << endl;
		}
	}

	//	Wait until the user hits a key before exiting
	PauseBeforeClosing();
}

void PauseBeforeClosing()
{
	EnableVTProcessing();
	wcout << endl;
	wcout << szVTYellow << "Paws-ed: Press any key to exit ..." << szVTReset << endl;
	while (0 == _kbhit());
}

void DisplayUsage(bool fVTEnabled)
{
	wcout << szVTYellow << "Paws" << szVTReset << ": Runs the specified command, and then pauses before closing" << endl;
	wcout << "Usage: " << szVTYellow << "Paws" << szVTReset << "[.exe] <" << szVTGreen << "command"<< szVTReset << "> <" << szVTGreen << "args" << szVTReset << ">" << endl;
	wcout << endl;
	wcout << "    command : Command to be run. If an executable is specified, it must be reachable on the current path" << endl;
	wcout << "    args    : Arguments to be passed to the executable" << endl;
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

bool EnableVTProcessing()
{
	DWORD consoleMode{};
	HRESULT hr{ E_UNEXPECTED };
	HANDLE hConsole = { GetStdHandle(STD_OUTPUT_HANDLE) };
	GetConsoleMode(hConsole, &consoleMode);
	hr = SetConsoleMode(hConsole, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)
		? S_OK
		: GetLastError();
	return S_OK == hr;
}