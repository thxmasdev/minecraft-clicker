#include <windows.h>
#include <commctrl.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <vector>
#include <atomic>
#include <fstream>
#include <tlhelp32.h>
#include <psapi.h>
#include <winternl.h>
#include <ntstatus.h>
#include <shlobj.h>
#include <shellapi.h>
#include <algorithm>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")

// Window controls IDs
#define ID_BUTTON_TOGGLE 1001
#define ID_BUTTON_CLICK_TYPE 1002
#define ID_BUTTON_MODE 1003
#define ID_BUTTON_SETTINGS 1004
#define ID_BUTTON_DESTRUCT 1005
#define ID_SLIDER_MIN_CPS 1006
#define ID_SLIDER_MAX_CPS 1007
#define ID_STATIC_STATUS 1008
#define ID_STATIC_CPS 1009
#define ID_STATIC_MIN_CPS 1010
#define ID_STATIC_MAX_CPS 1011
#define ID_STATIC_CURRENT_CPS 1012
#define ID_STATIC_CLICK_TYPE 1013
#define ID_STATIC_MODE 1014
#define ID_STATIC_LOGO 1015

// Global variables (matching original)
static bool g_active = false;
static bool g_running = true;
static bool g_leftClick = true;
static bool g_holdMode = false;
static int g_minCps = 8;
static int g_maxCps = 12;
static double g_currentCps = 0.0;
static std::mt19937 g_rng(std::chrono::steady_clock::now().time_since_epoch().count());
static char* g_exePath = nullptr;

// GUI specific
HWND g_hWnd = nullptr;
HWND g_hStatusText = nullptr;
HWND g_hCurrentCpsText = nullptr;
HWND g_hClickTypeText = nullptr;
HWND g_hModeText = nullptr;
HWND g_hMinCpsText = nullptr;
HWND g_hMaxCpsText = nullptr;
HWND g_hMinSlider = nullptr;
HWND g_hMaxSlider = nullptr;
std::thread g_clickThread;
std::thread g_hotkeyThread;

// Colors
#define BG_COLOR RGB(25, 25, 25)
#define TEXT_COLOR RGB(255, 255, 255)
#define ACCENT_COLOR RGB(0, 150, 255)
#define SUCCESS_COLOR RGB(0, 255, 100)
#define WARNING_COLOR RGB(255, 150, 0)
#define DANGER_COLOR RGB(255, 50, 50)

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateControls(HWND hwnd);
void UpdateInterface();
void ToggleActive();
void ToggleClickType();
void ToggleMode();
void OpenSettings();
void SelfDestruct();
void ClickingThread();
void HotkeyThread();

// Original functions from console version
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
    
    ExitProcess(0);
}

// Original clicking thread logic
void clickingThread() {
    auto lastClickTime = std::chrono::steady_clock::now();
    int clickCount = 0;
    auto cpsStartTime = std::chrono::steady_clock::now();
    static bool wasPressed = false;
    static auto pressStartTime = std::chrono::steady_clock::now();
    static bool lastCpsUpdateState = false;
    
    while (g_running) {
        bool shouldClick = false;
        
        if (g_holdMode) {
            shouldClick = g_active && isWindowActive();
        } else {
            bool leftPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
            bool rightPressed = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            bool targetButtonPressed = g_leftClick ? leftPressed : rightPressed;
            
            // Detectar cuando se presiona el botón por primera vez
            if (targetButtonPressed && !wasPressed) {
                pressStartTime = std::chrono::steady_clock::now();
            }
            
            // Actualizar el estado de presionado
            wasPressed = targetButtonPressed;
            
            // Solo hacer clic si está activo, Minecraft está en primer plano, y el botón está presionado
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
                PostMessage(g_hWnd, WM_USER + 1, 0, 0);
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
            
            lastCpsUpdateState = true;
        } else {
            // Solo actualizar CPS a 0 cuando cambie el estado de clicking
            if (lastCpsUpdateState) {
                g_currentCps = 0.0;
                PostMessage(g_hWnd, WM_USER + 1, 0, 0);
                lastCpsUpdateState = false;
            }
        }
        
        Sleep(1);
    }
}

// Original hotkey thread
void hotkeyThread() {
    static bool yPressed = false, lPressed = false, oPressed = false, pPressed = false;
    
    while (g_running) {
        bool yCurrent = (GetAsyncKeyState('Y') & 0x8000) != 0;
        if (yCurrent && !yPressed) {
            PostMessage(g_hWnd, WM_USER + 2, 0, 0); // Toggle
        }
        yPressed = yCurrent;
        
        bool pCurrent = (GetAsyncKeyState('P') & 0x8000) != 0;
        if (pCurrent && !pPressed) {
            PostMessage(g_hWnd, WM_USER + 3, 0, 0); // Click type
        }
        pPressed = pCurrent;
        
        bool lCurrent = (GetAsyncKeyState('L') & 0x8000) != 0;
        if (lCurrent && !lPressed) {
            PostMessage(g_hWnd, WM_USER + 4, 0, 0); // Mode
        }
        lPressed = lCurrent;
        
        bool oCurrent = (GetAsyncKeyState('O') & 0x8000) != 0;
        if (oCurrent && !oPressed) {
            PostMessage(g_hWnd, WM_USER + 5, 0, 0); // Settings
        }
        oPressed = oCurrent;
        
        if (GetAsyncKeyState('N') & 0x8000) {
            PostMessage(g_hWnd, WM_USER + 6, 0, 0); // Self destruct
        }
        
        Sleep(10);
    }
}

void UpdateInterface() {
    if (!g_hWnd) return;
    
    // Update status
    std::string status = g_active ? "ACTIVO" : "INACTIVO";
    SetWindowTextA(g_hStatusText, status.c_str());
    
    // Update current CPS
    std::ostringstream cpsStream;
    cpsStream << std::fixed << std::setprecision(1) << g_currentCps << " CPS";
    SetWindowTextA(g_hCurrentCpsText, cpsStream.str().c_str());
    
    // Update click type
    std::string clickType = g_leftClick ? "IZQUIERDO" : "DERECHO";
    SetWindowTextA(g_hClickTypeText, clickType.c_str());
    
    // Update mode
    std::string mode = g_holdMode ? "HOLD" : "CLICK";
    SetWindowTextA(g_hModeText, mode.c_str());
    
    // Update CPS labels
    std::string minCpsText = "Min: " + std::to_string(g_minCps) + " CPS";
    std::string maxCpsText = "Max: " + std::to_string(g_maxCps) + " CPS";
    SetWindowTextA(g_hMinCpsText, minCpsText.c_str());
    SetWindowTextA(g_hMaxCpsText, maxCpsText.c_str());
    
    // Update sliders
    SendMessage(g_hMinSlider, TBM_SETPOS, TRUE, g_minCps);
    SendMessage(g_hMaxSlider, TBM_SETPOS, TRUE, g_maxCps);
    
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void CreateControls(HWND hwnd) {
    HFONT hFont = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    HFONT hTitleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                                  DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    
    // Logo/Title
    HWND hLogo = CreateWindowA("STATIC", "⚡ MINECRAFT AUTOCLICKER ⚡", 
                              WS_VISIBLE | WS_CHILD | SS_CENTER,
                              20, 20, 460, 40, hwnd, (HMENU)ID_STATIC_LOGO, GetModuleHandle(NULL), NULL);
    SendMessage(hLogo, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
    
    // Status section
    CreateWindowA("STATIC", "Estado:", WS_VISIBLE | WS_CHILD,
                 30, 80, 80, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_hStatusText = CreateWindowA("STATIC", "INACTIVO", WS_VISIBLE | WS_CHILD,
                                 120, 80, 100, 25, hwnd, (HMENU)ID_STATIC_STATUS, GetModuleHandle(NULL), NULL);
    
    // Current CPS
    CreateWindowA("STATIC", "CPS Actual:", WS_VISIBLE | WS_CHILD,
                 250, 80, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_hCurrentCpsText = CreateWindowA("STATIC", "0.0 CPS", WS_VISIBLE | WS_CHILD,
                                     360, 80, 100, 25, hwnd, (HMENU)ID_STATIC_CURRENT_CPS, GetModuleHandle(NULL), NULL);
    
    // Click type and mode
    CreateWindowA("STATIC", "Tipo Click:", WS_VISIBLE | WS_CHILD,
                 30, 110, 100, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_hClickTypeText = CreateWindowA("STATIC", "IZQUIERDO", WS_VISIBLE | WS_CHILD,
                                    140, 110, 100, 25, hwnd, (HMENU)ID_STATIC_CLICK_TYPE, GetModuleHandle(NULL), NULL);
    
    CreateWindowA("STATIC", "Modo:", WS_VISIBLE | WS_CHILD,
                 280, 110, 60, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    g_hModeText = CreateWindowA("STATIC", "CLICK", WS_VISIBLE | WS_CHILD,
                               340, 110, 100, 25, hwnd, (HMENU)ID_STATIC_MODE, GetModuleHandle(NULL), NULL);
    
    // CPS Sliders
    CreateWindowA("STATIC", "Configuración CPS:", WS_VISIBLE | WS_CHILD,
                 30, 150, 200, 25, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Min CPS
    g_hMinCpsText = CreateWindowA("STATIC", "Min: 8 CPS", WS_VISIBLE | WS_CHILD,
                                 30, 180, 100, 25, hwnd, (HMENU)ID_STATIC_MIN_CPS, GetModuleHandle(NULL), NULL);
    g_hMinSlider = CreateWindowA(TRACKBAR_CLASSA, "", 
                                WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
                                30, 205, 200, 30, hwnd, (HMENU)ID_SLIDER_MIN_CPS, GetModuleHandle(NULL), NULL);
    SendMessage(g_hMinSlider, TBM_SETRANGE, TRUE, MAKELONG(1, 25));
    SendMessage(g_hMinSlider, TBM_SETPOS, TRUE, g_minCps);
    SendMessage(g_hMinSlider, TBM_SETTICFREQ, 5, 0);
    
    // Max CPS
    g_hMaxCpsText = CreateWindowA("STATIC", "Max: 12 CPS", WS_VISIBLE | WS_CHILD,
                                 270, 180, 100, 25, hwnd, (HMENU)ID_STATIC_MAX_CPS, GetModuleHandle(NULL), NULL);
    g_hMaxSlider = CreateWindowA(TRACKBAR_CLASSA, "", 
                                WS_VISIBLE | WS_CHILD | TBS_HORZ | TBS_AUTOTICKS,
                                270, 205, 200, 30, hwnd, (HMENU)ID_SLIDER_MAX_CPS, GetModuleHandle(NULL), NULL);
    SendMessage(g_hMaxSlider, TBM_SETRANGE, TRUE, MAKELONG(1, 50));
    SendMessage(g_hMaxSlider, TBM_SETPOS, TRUE, g_maxCps);
    SendMessage(g_hMaxSlider, TBM_SETTICFREQ, 10, 0);
    
    // Control buttons
    CreateWindowA("BUTTON", "🎯 TOGGLE (Y)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 30, 260, 120, 35, hwnd, (HMENU)ID_BUTTON_TOGGLE, GetModuleHandle(NULL), NULL);
    
    CreateWindowA("BUTTON", "🖱️ CLICK TYPE (P)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 170, 260, 140, 35, hwnd, (HMENU)ID_BUTTON_CLICK_TYPE, GetModuleHandle(NULL), NULL);
    
    CreateWindowA("BUTTON", "⚙️ MODE (L)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 330, 260, 120, 35, hwnd, (HMENU)ID_BUTTON_MODE, GetModuleHandle(NULL), NULL);
    
    CreateWindowA("BUTTON", "🔧 SETTINGS (O)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 30, 310, 140, 35, hwnd, (HMENU)ID_BUTTON_SETTINGS, GetModuleHandle(NULL), NULL);
    
    CreateWindowA("BUTTON", "💥 SELF-DESTRUCT (N)", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 190, 310, 180, 35, hwnd, (HMENU)ID_BUTTON_DESTRUCT, GetModuleHandle(NULL), NULL);
    
    // Instructions
    CreateWindowA("STATIC", "Hotkeys: Y=Toggle | P=Click Type | L=Mode | O=Settings | N=Destruct", 
                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                 20, 360, 460, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    CreateWindowA("STATIC", "Discord: thxmasdev | GitHub: thxmasdev/minecraft-clicker", 
                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                 20, 385, 460, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
    
    // Set fonts for all controls
    EnumChildWindows(hwnd, [](HWND hwndChild, LPARAM lParam) -> BOOL {
        HFONT hFont = (HFONT)lParam;
        SendMessage(hwndChild, WM_SETFONT, (WPARAM)hFont, TRUE);
        return TRUE;
    }, (LPARAM)hFont);
    
    // Set title font for logo
    SendMessage(hLogo, WM_SETFONT, (WPARAM)hTitleFont, TRUE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateControls(hwnd);
            UpdateInterface();
            return 0;
            
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, TEXT_COLOR);
            SetBkColor(hdc, BG_COLOR);
            return (LRESULT)CreateSolidBrush(BG_COLOR);
        }
        
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            HBRUSH hBrush = CreateSolidBrush(BG_COLOR);
            FillRect(hdc, &rect, hBrush);
            DeleteObject(hBrush);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BUTTON_TOGGLE:
                    g_active = !g_active;
                    UpdateInterface();
                    break;
                    
                case ID_BUTTON_CLICK_TYPE:
                    g_leftClick = !g_leftClick;
                    UpdateInterface();
                    break;
                    
                case ID_BUTTON_MODE:
                    g_holdMode = !g_holdMode;
                    UpdateInterface();
                    break;
                    
                case ID_BUTTON_SETTINGS:
                    MessageBoxA(hwnd, "Usa las barras deslizantes para ajustar CPS Min/Max", "Configuración", MB_OK | MB_ICONINFORMATION);
                    break;
                    
                case ID_BUTTON_DESTRUCT:
                    if (MessageBoxA(hwnd, "¿Estás seguro de que quieres auto-destruir?", 
                                   "Confirmación", MB_YESNO | MB_ICONWARNING) == IDYES) {
                        selfDestruct();
                    }
                    break;
            }
            return 0;
            
        case WM_HSCROLL:
            if ((HWND)lParam == g_hMinSlider) {
                g_minCps = SendMessage(g_hMinSlider, TBM_GETPOS, 0, 0);
                if (g_minCps > g_maxCps) {
                    g_maxCps = g_minCps;
                }
                UpdateInterface();
            } else if ((HWND)lParam == g_hMaxSlider) {
                g_maxCps = SendMessage(g_hMaxSlider, TBM_GETPOS, 0, 0);
                if (g_maxCps < g_minCps) {
                    g_minCps = g_maxCps;
                }
                UpdateInterface();
            }
            return 0;
            
        case WM_USER + 1: // Update CPS display
            UpdateInterface();
            return 0;
            
        case WM_USER + 2: // Toggle (Y)
            g_active = !g_active;
            UpdateInterface();
            return 0;
            
        case WM_USER + 3: // Click type (P)
            g_leftClick = !g_leftClick;
            UpdateInterface();
            return 0;
            
        case WM_USER + 4: // Mode (L)
            g_holdMode = !g_holdMode;
            UpdateInterface();
            return 0;
            
        case WM_USER + 5: // Settings (O)
            MessageBoxA(hwnd, "Usa las barras deslizantes para ajustar CPS Min/Max", "Configuración", MB_OK | MB_ICONINFORMATION);
            return 0;
            
        case WM_USER + 6: // Self destruct (N)
            if (MessageBoxA(hwnd, "¿Estás seguro de que quieres auto-destruir?", 
                           "Confirmación", MB_YESNO | MB_ICONWARNING) == IDYES) {
                selfDestruct();
            }
            return 0;
            
        case WM_CLOSE:
            g_running = false;
            if (g_clickThread.joinable()) g_clickThread.join();
            if (g_hotkeyThread.joinable()) g_hotkeyThread.join();
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Get executable path for self-destruct
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    g_exePath = exePath;
    
    // Initialize common controls
    InitCommonControls();
    
    // Register window class
    const char* CLASS_NAME = "MinecraftClickerWindow";
    
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = CreateSolidBrush(BG_COLOR);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    RegisterClassA(&wc);
    
    // Create window
    g_hWnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "⚡ Minecraft Autoclicker v2.0 ⚡",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 520, 460,
        NULL, NULL, hInstance, NULL
    );
    
    if (g_hWnd == NULL) {
        return 0;
    }
    
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    // Start threads
    g_running = true;
    g_clickThread = std::thread(clickingThread);
    g_hotkeyThread = std::thread(hotkeyThread);
    
    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // Cleanup
    g_running = false;
    if (g_clickThread.joinable()) g_clickThread.join();
    if (g_hotkeyThread.joinable()) g_hotkeyThread.join();
    
    return 0;
}