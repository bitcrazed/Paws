// Paws.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>
#include <Windows.h>
#include <conio.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING ((DWORD)0x4)
#endif

struct VTColors
{
	std::wstring strVTReset{ L"\x1b[m" };
	std::wstring strVTYellow{ L"\x1b[33m" };
	std::wstring strVTGreen{ L"\x1b[32m" };
	std::wstring strVTRed{ L"\x1b[31m" };

	void clear()
	{
		strVTReset.clear();
		strVTYellow.clear();
		strVTGreen.clear();
		strVTRed.clear();
	}
};

// Forward Declarations:
void DisplayUsage(const VTColors& colors);
void PauseBeforeClosing(const VTColors& colors);
bool EnableVTProcessing();
std::wstring GetErrorMessage(DWORD dwErrorCode);
LPWSTR GetCmdLnSkipFirstToken();

int main()
{
	DWORD dwExitCode{};
	VTColors colors{};

	// Enable Console VT Processing
	bool fVTEnabled = EnableVTProcessing();

	// Configure color strings if VT is disabled
	if (fVTEnabled == false)
		colors.clear();

	// Extract the command-line without the name of this executable:
	LPWSTR szCommandLine{ GetCmdLnSkipFirstToken() };

	if (szCommandLine == nullptr)
		DisplayUsage(colors);
	else
	{
		// Launch the requested command
		std::wcout << colors.strVTYellow << L"Paws is executing: '" << colors.strVTGreen << szCommandLine << colors.strVTYellow << L'\'' << colors.strVTReset << std::endl;
		STARTUPINFOW si{ sizeof(STARTUPINFOW) };
		PROCESS_INFORMATION pi{};
		if (
			CreateProcessW(
				nullptr,
				szCommandLine,
				nullptr,
				nullptr,
				TRUE,
				0,
				nullptr,
				nullptr,
				&si,
				&pi
			) != FALSE)
		{
			// Wait for the executable to complete
			WaitForSingleObject(pi.hProcess, INFINITE);
			GetExitCodeProcess(pi.hProcess, &dwExitCode);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		else
		{
			dwExitCode = GetLastError();
			std::wcerr << colors.strVTRed << L"Error: Paws failed to execute: " << GetErrorMessage(dwExitCode) << colors.strVTReset << std::endl;
		}
	}

	//	Wait until the user hits a key before exiting
	PauseBeforeClosing(colors);
	return static_cast<int>(dwExitCode);
}

void PauseBeforeClosing(const VTColors& colors)
{
	std::wcout << colors.strVTYellow << L"\nPaws-ed: Press any key to exit . . . " << colors.strVTReset << std::flush;
	static_cast<void>(_getwch());
	std::wcout << std::endl;
}

void DisplayUsage(const VTColors& colors)
{
	std::wcout << colors.strVTYellow << L"Paws" << colors.strVTReset << L": Runs the specified command, and then pauses before closing\n"
		L"Usage: " << colors.strVTYellow << L"Paws" << colors.strVTReset << L"[.exe] <" << colors.strVTGreen << L"command" << colors.strVTReset << L"> <" << colors.strVTGreen << L"args" << colors.strVTReset << L">\n\n"
		L"    command : Command to be run. If an executable is specified, it must be reachable on the current path\n"
		L"    args    : Arguments to be passed to the executable" << std::endl;
}

std::wstring GetErrorMessage(DWORD dwErrorCode)
{
	LPWSTR lpMsgBuf{};
	std::wstring msg{};
	if (
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr,
			dwErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&lpMsgBuf),
			0,
			nullptr
		) != 0)
	{
		msg.assign(lpMsgBuf);
	}

	LocalFree(lpMsgBuf);
	return msg;
}

bool EnableVTProcessing()
{
	DWORD consoleMode{};
	HANDLE hConsole = { GetStdHandle(STD_OUTPUT_HANDLE) };
	return (GetConsoleMode(hConsole, &consoleMode) != FALSE &&
		SetConsoleMode(hConsole, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != FALSE);
}

LPWSTR GetCmdLnSkipFirstToken()
{
	static LPWSTR cmdLn{};
	bool isInQuotes{};
	bool isInToken{};
	DWORD tokensCnt{};

	if (cmdLn == nullptr)
		cmdLn = GetCommandLineW();

	for (LPWCH cursor{ cmdLn }; *cursor != L'\0'; ++cursor)
	{
		switch (*cursor)
		{
		case L' ': case L'\t': // Token separators, unless in a quoted substring
			if (isInQuotes == false)
				isInToken = false;
			break;

		case L'\"':
			isInQuotes = !isInQuotes;
		default: // This includes quotation marks
			if (isInToken == false)
			{
				++tokensCnt;
				if (tokensCnt > 1) // That is, the cursor points to the beginning of the second token
					return cursor;

				isInToken = true;
			}
			break;
		}
	}

	return nullptr;
}
