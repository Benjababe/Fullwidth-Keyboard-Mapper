#include <iostream>
#include <map>
#include <Windows.h>

std::map<std::pair<DWORD, bool>, WORD> symbolMap;

const WORD FW_ZERO = 0xFF10;
const WORD FW_CAPS_A = 0xFF21;
const WORD FW_A = 0xFF41;

/**
 * @brief Writes unicode character onto keyboard cursor
 *
 * @param unicodeChar Character to write
 * @param keyup Flag whether to send key up or down input
 */
void WriteUnicode(WORD unicodeChar, bool keyup)
{
    INPUT input = {0};
    input.type = INPUT_KEYBOARD;
    input.ki.wScan = unicodeChar;
    input.ki.dwFlags = (keyup) ? (KEYEVENTF_UNICODE | KEYEVENTF_KEYUP) : KEYEVENTF_UNICODE;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;

    UINT res = SendInput(1, &input, sizeof(input));
    if (res == 0)
        std::cerr << "Error sending input: " << GetLastError() << std::endl;
}

/**
 * @brief Retrieves corresponding fullwidth unicode character if available
 *
 * @param vkCode Virtual-Key Code entered
 * @param useCaps Flag whether CAPS lock is enabled
 * @param shiftState Flag whether Shift key is currently pressed
 * @return std::pair<bool, WORD> Bool flag whether to use unicode and the unicode value
 */
WORD getUnicodeChar(DWORD vkCode, bool useCaps, bool shiftState)
{
    WORD unicodeChar = UNICODE_NOCHAR;

    if ((vkCode >= 0x41 && vkCode <= 0x5A))
        unicodeChar = ((useCaps) ? FW_CAPS_A : FW_A) + vkCode - 0x41;
    else if (symbolMap[std::make_pair(vkCode, shiftState)] != 0)
        unicodeChar = symbolMap[std::make_pair(vkCode, shiftState)];
    else if ((vkCode >= 0x30 && vkCode <= 0x39) || (vkCode >= 0x60 && vkCode <= 0x69))
    {
        DWORD baseCode = (vkCode >= 0x60) ? vkCode - 0x30 : vkCode;
        unicodeChar = FW_ZERO + baseCode - 0x30;
    }

    return unicodeChar;
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    BOOL fEatKeystroke = FALSE;

    if (nCode == HC_ACTION)
    {
        // Listens for key up/down actions
        switch (wParam)
        {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

            // Do not interfere if ctrl is pressed
            bool ctrlState = ((GetKeyState(VK_CONTROL) & 0x1000) != 0);
            if (ctrlState)
                break;

            // Retrieve relevant modifier states
            bool capsLockState = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0);
            bool shiftState = ((GetKeyState(VK_SHIFT) & 0x1000) != 0);
            bool useCaps = capsLockState ^ shiftState;

            // Process keyboard input for valid unicode character if available
            WORD unicodeChar = getUnicodeChar(p->vkCode, useCaps, shiftState);
            if (fEatKeystroke = (unicodeChar != UNICODE_NOCHAR))
            {
                if ((wParam == WM_KEYDOWN) || (wParam == WM_SYSKEYDOWN))
                    WriteUnicode(unicodeChar, false);
                else if ((wParam == WM_KEYUP) || (wParam == WM_SYSKEYUP))
                    WriteUnicode(unicodeChar, true);
            }

            break;
        }
    }

    // If key was eaten, return true to stop any further processing
    // Else the key will be handled as per normal
    return (fEatKeystroke ? 1 : CallNextHookEx(NULL, nCode, wParam, lParam));
}

// Setup other keys that aren't contiguous
void populateSymbolMap()
{
    symbolMap[std::make_pair(0x31, true)] = 0xFF01;      // '!'
    symbolMap[std::make_pair(0xDE, true)] = 0xFF02;      // '"'
    symbolMap[std::make_pair(0x33, true)] = 0xFF03;      // '#'
    symbolMap[std::make_pair(0x34, true)] = 0xFF04;      // '$'
    symbolMap[std::make_pair(0x35, true)] = 0xFF05;      // '%'
    symbolMap[std::make_pair(0x37, true)] = 0xFF06;      // '&'
    symbolMap[std::make_pair(0xDE, false)] = 0xFF07;     // '''
    symbolMap[std::make_pair(0x39, true)] = 0xFF08;      // '('
    symbolMap[std::make_pair(0x30, true)] = 0xFF09;      // ')'
    symbolMap[std::make_pair(0x38, true)] = 0xFF0A;      // '*'
    symbolMap[std::make_pair(0xBB, true)] = 0xFF0B;      // '+'
    symbolMap[std::make_pair(0xBC, false)] = 0xFF0C;     // ','
    symbolMap[std::make_pair(0xBD, false)] = 0xFF0D;     // '-'
    symbolMap[std::make_pair(0xBE, false)] = 0xFF0E;     // '.'
    symbolMap[std::make_pair(0xBF, false)] = 0xFF0F;     // '/'
    symbolMap[std::make_pair(0xBA, true)] = 0xFF1A;      // ':'
    symbolMap[std::make_pair(0xBA, false)] = 0xFF1B;     // ';'
    symbolMap[std::make_pair(0xBC, true)] = 0xFF1C;      // '<'
    symbolMap[std::make_pair(0xBB, false)] = 0xFF1D;     // '='
    symbolMap[std::make_pair(0xBE, true)] = 0xFF1E;      // '>'
    symbolMap[std::make_pair(0xBF, true)] = 0xFF1F;      // '?'
    symbolMap[std::make_pair(0x32, true)] = 0xFF20;      // '@'
    symbolMap[std::make_pair(0xDB, false)] = 0xFF3B;     // '['
    symbolMap[std::make_pair(0xDC, false)] = 0xFF3C;     // '\'
    symbolMap[std::make_pair(0xDD, false)] = 0xFF3D;     // ']'
    symbolMap[std::make_pair(0x36, true)] = 0xFF3E;      // '^'
    symbolMap[std::make_pair(0xBD, true)] = 0xFF3F;      // '_'
    symbolMap[std::make_pair(0xC0, false)] = 0xFF40;     // '`'
    symbolMap[std::make_pair(0xDB, true)] = 0xFF5B;      // '{'
    symbolMap[std::make_pair(0xDC, true)] = 0xFF5C;      // '|'
    symbolMap[std::make_pair(0xDD, true)] = 0xFF5D;      // '}'
    symbolMap[std::make_pair(0xC0, true)] = 0xFF5E;      // '~'
    symbolMap[std::make_pair(VK_SPACE, false)] = 0xFFA0; // ' '
}

int main()
{
    // Setup non alphabet keys
    populateSymbolMap();

    // Set up the keyboard hook
    HHOOK g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, 0, 0);

    // Message loop, keep it running until it's told to stop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Unhook and clean up when the program ends
    UnhookWindowsHookEx(g_keyboardHook);

    return 0;
}