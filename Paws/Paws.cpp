// Paws.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>
#include <Windows.h>
#include <conio.h>
#include <wchar.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING ((DWORD)0x4)
#endif

struct VTColors
{
	std::wstring strVTReset{ L"\x1b[m" };
	std::wstring strVTYellow{ L"\x1b[33m" };
	std::wstring strVTGreen{ L"\x1b[32m" };
	std::wstring strVTRed{ L"\x1b[31m" };

	// Clears the VT strings
	void clear()
	{
		bool bResult;

		bResult = strVTReset.empty();
		bResult = strVTYellow.empty();
		bResult = strVTGreen.empty();
		bResult = strVTRed.empty();
	}
};

// Function Declarations:
bool EnableVTProcessing();
void DisplayUsage(const VTColors& colors);
void PauseBeforeClosing(const VTColors& colors);
std::wstring GetErrorMessage(const DWORD dwErrorCode);
std::wstring GetCommandlineToExecute();
DWORD LaunchProcess(const std::wstring& cmdline);

int wmain(int argc, wchar_t* argv[])
{
	DWORD result{};

	// Clear color strings if VT is disabled
	VTColors colors{};
	if (false == EnableVTProcessing())
	{
		colors.clear();
	}

	if (argc < 2)
	{
		DisplayUsage(colors);
		result = EXIT_FAILURE;
	}
	else
	{
		std::wstring cmdline{ GetCommandlineToExecute() };

		// Launch the requested command
		std::wcout
			<< colors.strVTYellow
			<< L"Paws is executing '" << cmdline << "'"
			<< L'\''
			<< colors.strVTReset
			<< std::endl;

		result = LaunchProcess(cmdline);
		if(0 != result)
		{
			std::wcerr 
				<< colors.strVTRed 
				<< L"Error: Paws failed to execute: " 
				<< GetErrorMessage(result) 
				<< colors.strVTReset 
				<< std::endl;
		}
	}

	//	Wait until the user hits a key before exiting
	PauseBeforeClosing(colors);

	return result;
}

void PauseBeforeClosing(const VTColors& colors)
{
	std::wcout << colors.strVTYellow << std::endl << L"Paws-ed: Press any key to exit . . . " << colors.strVTReset << std::flush;
	static_cast<void>(_getch());
	std::wcout << std::endl;
}

void DisplayUsage(const VTColors& colors)
{
	std::wcout
		<< colors.strVTYellow << L"Paws" << colors.strVTReset << L": Runs the specified command, and then pauses before closing" << std::endl
		<< L"Usage: " << colors.strVTYellow << L"Paws" << colors.strVTReset << L"[.exe] <" 
		<< colors.strVTGreen << L"target" << colors.strVTReset << L"> <" 
		<< colors.strVTGreen << L"args" << colors.strVTReset << L">" << std::endl << std::endl
		<< L"    target : Command/executable to be run" << std::endl
		<< L"    args    : Arguments to be passed to the command/executable" << std::endl;
}

std::wstring GetErrorMessage(DWORD dwErrorCode)
{
	LPWSTR lpMsgBuf{};
	std::wstring msg{};
	if (
		FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER 
				| FORMAT_MESSAGE_FROM_SYSTEM 
				| FORMAT_MESSAGE_IGNORE_INSERTS,
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
	return (FALSE != GetConsoleMode(hConsole, &consoleMode)
		&& FALSE != SetConsoleMode(hConsole, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING));
}

std::wstring GetCommandlineToExecute()
{
	std::wstring result{};
	LPWSTR cmdline{ GetCommandLineW() };
	int argc{};
	LPWSTR* argv{ CommandLineToArgvW(cmdline, &argc) };
	
	// Check if we have any args to play with
	if (NULL != argv && argc > 1)
	{
		// Append all the args with spaces between them
		for (int i = 1; i < argc; i++)
		{
			result.append(argv[i]);
			result.append(L" ");
		}

		// Cleanup before we return
		LocalFree(argv);
	}

	return result;
}

DWORD LaunchProcess(const std::wstring& cmdline)
{
	DWORD result{};
	PROCESS_INFORMATION pi{};
	STARTUPINFOW si{ sizeof(STARTUPINFOW) };
	if (
		CreateProcessW(
			nullptr,
			const_cast<LPWSTR>(cmdline.c_str()),
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

		// Clean-up on the way out
		GetExitCodeProcess(pi.hProcess, &result);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}

	return result;
}
