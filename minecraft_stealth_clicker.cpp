#pragma comment(linker, "/SUBSYSTEM:console")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")

#include <windows.h>
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <string>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include <iomanip>
#include <shellapi.h>
#include <cmath>
#include <algorithm>


#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"
#define UNDERLINE "\033[4m"

static bool g_active = false;
static bool g_running = true;
static bool g_leftClick = true;
static bool g_holdMode = false;
static int g_minCps = 8;
static int g_maxCps = 12;
static double g_currentCps = 0.0;
static std::mt19937 g_rng(std::chrono::steady_clock::now().time_since_epoch().count());
static char* g_exePath = nullptr;


double getGaussianCps(int minCps, int maxCps) {
    double mean = (minCps + maxCps) / 2.0;
    double stddev = (maxCps - minCps) / 6.0;
    std::normal_distribution<double> dist(mean, stddev);
    double result = dist(g_rng);
    return std::max((double)minCps, std::min((double)maxCps, result));
}


void addHumanJitter() {
    std::uniform_int_distribution<int> jitter(-2, 2);
    POINT cursor;
    GetCursorPos(&cursor);
    SetCursorPos(cursor.x + jitter(g_rng), cursor.y + jitter(g_rng));
}


bool isWindowActive() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd == nullptr) return false;
    
    char windowTitle[256];
    GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));
    
    std::string title(windowTitle);
    

    std::transform(title.begin(), title.end(), title.begin(), ::tolower);
    

    return title.find("minecraft") != std::string::npos ||
           title.find("lunar") != std::string::npos ||
           title.find("badlion") != std::string::npos ||
           title.find("forge") != std::string::npos ||
           title.find("fabric") != std::string::npos ||
           title.find("optifine") != std::string::npos;
}


void performClick(bool leftClick) {
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    

    if (leftClick) {

        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));

        Sleep(1);

        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
    } else {

        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));

        Sleep(1);

        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));
    }
}


void selfDestruct() {
    g_running = false;
    g_active = false;
    
    HANDLE hProcess = GetCurrentProcess();
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    
    if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, 0);
        CloseHandle(hToken);
    }
    
    MEMORY_BASIC_INFORMATION mbi;
    LPVOID addr = 0;
    while (VirtualQuery(addr, &mbi, sizeof(mbi))) {
        if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE)) {
            SecureZeroMemory(mbi.BaseAddress, mbi.RegionSize);
        }
        addr = (LPBYTE)mbi.BaseAddress + mbi.RegionSize;
    }
    
    std::uniform_int_distribution<int> sleepTime(50, 200);
    Sleep(sleepTime(g_rng));
    
    if (g_exePath) {
        char tempPath[MAX_PATH];
        char tempFile[MAX_PATH];
        GetTempPathA(MAX_PATH, tempPath);
        GetTempFileNameA(tempPath, "tmp", 0, tempFile);
        
        HANDLE hFile = CreateFileA(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
                                  FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
        
        if (hFile != INVALID_HANDLE_VALUE) {
            const char* script = 
                "@echo off\n"
                "ping 127.0.0.1 -n 3 > nul\n"
                "taskkill /f /im \"minecraft_clicker.exe\" 2>nul\n"
                "ping 127.0.0.1 -n 2 > nul\n"
                "del /f /q \"%s\" 2>nul\n"
                "sdelete -p 3 -s -z \"%s\" 2>nul\n"
                "del /f /q \"%%~f0\" & exit\n";
            
            char buffer[1024];
            sprintf_s(buffer, script, g_exePath, g_exePath);
            
            DWORD written;
            WriteFile(hFile, buffer, strlen(buffer), &written, NULL);
            CloseHandle(hFile);
            
            STARTUPINFOA si = {sizeof(si)};
            PROCESS_INFORMATION pi;
            si.dwFlags = STARTF_USESHOWWINDOW;
            si.wShowWindow = SW_HIDE;
            
            CreateProcessA(NULL, tempFile, NULL, NULL, FALSE, 
                          CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &pi);
            
            if (pi.hProcess) {
                CloseHandle(pi.hProcess);
                CloseHandle(pi.hThread);
            }
        }
    }
    
    FreeConsole();
    TerminateProcess(GetCurrentProcess(), 0);
}

static bool g_needsUpdate = true;
static bool g_lastActive = false;
static bool g_lastLeftClick = true;
static bool g_lastHoldMode = false;
static double g_lastCps = 0.0;


void displayInterface() {

    if (!g_needsUpdate && 
        g_lastActive == g_active && 
        g_lastLeftClick == g_leftClick && 
        g_lastHoldMode == g_holdMode &&
        abs(g_lastCps - g_currentCps) < 0.1) {
        return;
    }
    
    system("cls");

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    
    std::cout << CYAN << BOLD << "\n";
    std::cout << "  +==================================================+\n";
    std::cout << "  |" << MAGENTA << "            >> MINECRAFT AUTOCLICKER <<           " << CYAN << "|\n";
    std::cout << "  +==================================================+\n";
    std::cout << "  |" << YELLOW << "  Created by Thomàs                       " << CYAN << "|\n";
    std::cout << "  |" << YELLOW << "  Discord: thxmasdev                            " << CYAN << "|\n";
    std::cout << "  +==================================================+\n";
    std::cout << "  |" << WHITE << "  [*] CPS Settings:                               " << CYAN << "|\n";
    std::cout << "  |" << GREEN << "     Min: " << g_minCps << " CPS  |  Max: " << g_maxCps << " CPS" << std::string(20 - std::to_string(g_minCps).length() - std::to_string(g_maxCps).length(), ' ') << CYAN << "|\n";
    std::cout << "  |" << GREEN << "     Current: " << std::fixed << std::setprecision(1) << g_currentCps << " CPS" << std::string(28 - std::to_string((int)g_currentCps).length(), ' ') << CYAN << "|\n";
    std::cout << "  +==================================================+\n";
    std::cout << "  |" << WHITE << "  [!] Status:                                     " << CYAN << "|\n";
    std::cout << "  |" << (g_active ? GREEN "     [+] ACTIVE" : RED "     [-] INACTIVE") << std::string(35, ' ') << CYAN << "|\n";
    std::cout << "  |" << WHITE << "     Click Type: " << (g_leftClick ? BLUE "LEFT" : YELLOW "RIGHT") << std::string(25, ' ') << CYAN << "|\n";
    std::cout << "  |" << WHITE << "     Mode: " << (g_holdMode ? MAGENTA "HOLD" : GREEN "CLICK") << std::string(31, ' ') << CYAN << "|\n";
    std::cout << "  +==================================================+\n";
    std::cout << "  |" << WHITE << "  [>] Controls:                                   " << CYAN << "|\n";
    std::cout << "  |" << YELLOW << "     Y" << WHITE << " - Toggle ON/OFF                         " << CYAN << "|\n";
    std::cout << "  |" << YELLOW << "     P" << WHITE << " - Switch Click Type                     " << CYAN << "|\n";
    std::cout << "  |" << YELLOW << "     L" << WHITE << " - Toggle Hold Mode                      " << CYAN << "|\n";
    std::cout << "  |" << YELLOW << "     O" << WHITE << " - Open Settings                         " << CYAN << "|\n";
    std::cout << "  |" << RED << "     N" << WHITE << " - Self Destruct                         " << CYAN << "|\n";
    std::cout << "  +==================================================+\n";
    std::cout << "  |" << MAGENTA << "  [i] Hold configured mouse button to activate!   " << CYAN << "|\n";
    std::cout << "  +==================================================+\n";
    std::cout << RESET << "\n";

    g_lastActive = g_active;
    g_lastLeftClick = g_leftClick;
    g_lastHoldMode = g_holdMode;

    g_lastCps = g_currentCps;
    g_needsUpdate = false;
}


void configureSettings() {
    system("cls");
    std::cout << "=== CONFIGURACION ===\n";
    std::cout << "Min CPS (actual: " << g_minCps << "): ";
    std::cin >> g_minCps;
    std::cout << "Max CPS (actual: " << g_maxCps << "): ";
    std::cin >> g_maxCps;
    
    if (g_minCps > g_maxCps) {
        std::swap(g_minCps, g_maxCps);
    }
    
    std::cout << "Configuracion guardada. Presiona cualquier tecla...";
    _getch();
}


void clickingThread() {
    auto lastClickTime = std::chrono::steady_clock::now();
    int clickCount = 0;
    auto cpsStartTime = std::chrono::steady_clock::now();

    static bool wasPressed = false;
    static auto pressStartTime = std::chrono::steady_clock::now();
    
    while (g_running) {
        bool shouldClick = false;
        
        if (g_holdMode) {
            shouldClick = g_active && isWindowActive();
        } else {
            bool leftPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            bool rightPressed = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

            bool targetButtonPressed = false;
            if (g_leftClick) {
                targetButtonPressed = leftPressed;
            } else {
                targetButtonPressed = rightPressed;
            }

            if (targetButtonPressed && !wasPressed) {
                pressStartTime = std::chrono::steady_clock::now();
            }
            
            wasPressed = targetButtonPressed;

            if (g_active && isWindowActive() && targetButtonPressed) {
                auto timeSincePress = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - pressStartTime).count();
                shouldClick = timeSincePress >= 50;
            }
        }
        
        if (shouldClick) {
            auto now = std::chrono::steady_clock::now();

            auto timeSinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(now - cpsStartTime).count();
            if (timeSinceStart >= 1000) {
                g_currentCps = (clickCount * 1000.0) / timeSinceStart;
                clickCount = 0;
                cpsStartTime = now;
            }

            double targetCps = getGaussianCps(g_minCps, g_maxCps);
            double interval = 1000.0 / targetCps;

            std::uniform_real_distribution<double> variation(0.95, 1.05);
            interval *= variation(g_rng);
            
            auto timeSinceLastClick = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastClickTime).count();
            
            if (timeSinceLastClick >= interval) {
                addHumanJitter();
                performClick(g_leftClick);
                lastClickTime = now;
                clickCount++;

                std::uniform_int_distribution<int> randomSleep(1, 5);
                Sleep(randomSleep(g_rng));
            }
        } else {
            if (!shouldClick) {
                g_currentCps = 0.0;
            }
        }
        
        Sleep(1);
    }
}


void hotkeyThread() {
    static bool yPressed = false, lPressed = false, oPressed = false, pPressed = false;
    
    while (g_running) {

        bool yCurrent = (GetAsyncKeyState('Y') & 0x8000) != 0;
        if (yCurrent && !yPressed) {
            g_active = !g_active;
            g_needsUpdate = true;
        }
        yPressed = yCurrent;

        bool pCurrent = (GetAsyncKeyState('P') & 0x8000) != 0;
        if (pCurrent && !pPressed) {
            g_leftClick = !g_leftClick;
            g_needsUpdate = true;
        }
        pPressed = pCurrent;

        bool lCurrent = (GetAsyncKeyState('L') & 0x8000) != 0;
        if (lCurrent && !lPressed) {
            g_holdMode = !g_holdMode;
            g_needsUpdate = true;
        }
        lPressed = lCurrent;

        bool oCurrent = (GetAsyncKeyState('O') & 0x8000) != 0;
        if (oCurrent && !oPressed) {
            configureSettings();
            g_needsUpdate = true;
        }
        oPressed = oCurrent;

        if (GetAsyncKeyState('N') & 0x8000) {
            selfDestruct();
        }
        
        Sleep(10);
    }
}

int main(int argc, char* argv[]) {

    g_exePath = argv[0];

    ShowCursor(FALSE);

    SetConsoleTitleA("System Process");

    srand(static_cast<unsigned int>(time(nullptr)));

    std::thread clickThread(clickingThread);
    std::thread hotkeyThreadObj(hotkeyThread);

    displayInterface();

    while (g_running) {
        displayInterface();
        Sleep(500);
    }

    clickThread.join();
    hotkeyThreadObj.join();
    
    return 0;
}