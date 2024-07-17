#include "UMapp.hpp"

#include <conio.h>
#include <iomanip>
#include <vector>
#include <algorithm>

void clearScreen() 
{
    HANDLE hStdOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hStdOut, (TCHAR)' ', cellCount, homeCoords, &count);

    FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count);

    SetConsoleCursorPosition(hStdOut, homeCoords);
}

void printHeader() 
{
    std::cout << "PID" << std::setw(16);
    std::cout << "Time (ms)";

    std::cout << std::endl;
}

void printRow(ULONG_PTR id, ULONG64 value) 
{
    std::cout << id << std::setw(16);
    std::cout << (double)value / 10000.;
    std::cout << std::endl;
}

void printData(std::unordered_map<ULONG_PTR, ULONG64>& map) 
{
    clearScreen();
    printHeader();

    std::vector<std::pair<ULONG_PTR, ULONG64>> v(map.begin(), map.end());
    sort(v.begin(), v.end(), [](const std::pair<ULONG_PTR, ULONG64> &a, const std::pair<ULONG_PTR, ULONG64> &b) {
        return a.second > b.second;
        });

    for (const auto &pair : v) 
    {
		printRow(pair.first, pair.second);
    }
}