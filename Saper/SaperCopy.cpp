#include "framework.h"
#include "Saper.h"
#include "windowsx.h"
#include "time.h"

#define MAX_LOADSTRING 100
#define PIECE_SIZE 25
#define MAX_X_SIZE 24
#define MAX_Y_SIZE 30
#define TOP_PANEL 30
#define START_X_SIZE 10
#define START_Y_SIZE 10
#define START_MINES 10
#define MIN_MINES 10


struct TIME_ST
{
    ULONGLONG begin;
    bool started;
};

struct MINE_ST
{
    bool hasMine;
    bool isShown;
    bool isFlagged;
    int neighbors;
    int xPos;
    int yPos;
};

HINSTANCE hInst; // Program instance
WCHAR szTitle[MAX_LOADSTRING]; // Window title
WCHAR szWindowClass[MAX_LOADSTRING]; // Window Class Name
WCHAR szPieceClass[MAX_LOADSTRING]; // Piece Class Name
HWND szPieces[MAX_X_SIZE][MAX_Y_SIZE]; // Table with handles to piece windows
MINE_ST* Mines[MAX_X_SIZE][MAX_Y_SIZE]; // Table with pointers to tile structs
BOOL xNoObjects; // Current height in number of tiles
BOOL yNoObjects; // Current width in number of tiles
BOOL nxNoObjects; // Future height in number of tiles
BOOL nyNoObjects; // Future width in number of tiles
BOOL nMines; // Future number of mines
BOOL BombsLeft; // How many bombs are left
BOOL RESET_GAME = FALSE; // Whether to reset the game
BOOL STARTED = FALSE; // Whether first tile has been clicked
BOOL DEBUG = FALSE; // If debug mode is ON
BOOL GAME_STOPPED = FALSE; // If game has stopped
COLORREF COLORS[8] = { RGB(0, 0, 255), RGB(0, 200, 0), RGB(255, 0, 0) , RGB(0, 0, 100),
                        RGB(100, 0, 0) , RGB(0, 55, 55) , RGB(0, 0, 0) , RGB(0, 255, 0) }; // Table of colors for numbers
COLORREF SHOWN_TILE = RGB(230, 230, 230); // RGB value of background when a tile is shown

ATOM                RegisterMainClass(HINSTANCE hInstance); // Registers main window class 
ATOM                RegisterPieceClass(HINSTANCE hInstance); // Registers piece window class
BOOL                InitInstance(HINSTANCE, int); // Initializes basic aspects of game
BOOL                InitPieces(HWND); // Initializes children pieces
BOOL                ResetGame(HWND); // Resets game
BOOL                ShowTile(int, int, HDC); // Used to show tile
BOOL                HideTile(int, int); // 
BOOL                CheckIfEnded();
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK    CustomSizeProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndPieceProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SAPER, szWindowClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PIECE, szPieceClass, MAX_LOADSTRING);

    RegisterMainClass(hInstance);
    RegisterPieceClass(hInstance);

    srand(time(NULL));

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SAPER));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}