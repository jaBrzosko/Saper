// Saper.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Saper.h"
#include "windowsx.h"
#include "time.h"

#define MAX_LOADSTRING 100
#define PIECE_SIZE 25
#define MIN_X_SIZE 10
#define MIN_Y_SIZE 10
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

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
WCHAR szPieceClass[MAX_LOADSTRING];
HWND szPieces[MAX_X_SIZE][MAX_Y_SIZE];
MINE_ST* Mines[MAX_X_SIZE][MAX_Y_SIZE];
BOOL xNoObjects;
BOOL yNoObjects;
BOOL nxNoObjects;
BOOL nyNoObjects;
BOOL nMines;
BOOL BombsLeft;
BOOL RESET_GAME = FALSE;
BOOL STARTED = FALSE;
BOOL DEBUG = FALSE;
BOOL GAME_STOPPED = FALSE;
COLORREF COLORS[8] = { RGB(0, 0, 255), RGB(0, 200, 0), RGB(255, 0, 0) , RGB(0, 0, 100),
                        RGB(100, 0, 0) , RGB(0, 55, 55) , RGB(0, 0, 0) , RGB(200, 200, 0) };
COLORREF SHOWN_TILE = RGB(0xC8, 0xC8, 0xC8);

ATOM                RegisterMainClass(HINSTANCE hInstance);
ATOM                RegisterPieceClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
BOOL                InitPieces(HWND);
BOOL                ResetGame(HWND);
BOOL                ShowTile(int, int, HDC);
BOOL                HideTile(int, int);
BOOL                UncoverTile(HWND, MINE_ST*);
BOOL                InitSingleTile(int, int, HWND);
BOOL                SetDefaultMine(int, int);
BOOL                InitBombs();
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

    // Main message loop:
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

ATOM RegisterMainClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SAPER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MINE));

    return RegisterClassExW(&wcex);
}

ATOM RegisterPieceClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndPieceProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szPieceClass;
    wcex.hIconSm = NULL;

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    xNoObjects = START_X_SIZE;
    yNoObjects = START_Y_SIZE;

    nxNoObjects = START_X_SIZE;
    nyNoObjects = START_Y_SIZE;
    nMines = START_MINES;

    RECT rt = { 0, 0, xNoObjects * (PIECE_SIZE + 1) + 1, yNoObjects * (PIECE_SIZE + 1) + 1 + TOP_PANEL };

    AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX, TRUE);

    TIME_ST* st = new TIME_ST;
    st->begin = GetTickCount64();
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
        (x - rt.right) / 2, (y - rt.bottom) / 2, rt.right - rt.left, rt.bottom - rt.top, nullptr, nullptr, hInstance, st);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    InitPieces(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
        case ID_GAME_NEW:
        {
            ResetGame(hWnd);
            SetTimer(hWnd, 1, 50, NULL);
            break;
        }
        case ID_HELP_DEBUG:
            if (!DEBUG)
            {
                CheckMenuItem(GetMenu(hWnd), ID_HELP_DEBUG, MF_CHECKED);
                DEBUG = TRUE;
                for (int x = 0; x < xNoObjects; x++)
                {
                    for (int y = 0; y < yNoObjects; y++)
                    {
                        if (Mines[x][y]->isShown || Mines[x][y]->isFlagged) continue;
                        HDC hdc = GetDC(szPieces[x][y]);
                        ShowTile(x, y, hdc);
                        ReleaseDC(szPieces[x][y], hdc);
                    }
                }
            }
            else
            {
                CheckMenuItem(GetMenu(hWnd), ID_HELP_DEBUG, MF_UNCHECKED);
                DEBUG = FALSE;
                for (int x = 0; x < xNoObjects; x++)
                {
                    for (int y = 0; y < yNoObjects; y++)
                    {
                        HideTile(x, y);
                    }
                }
            }
            break;
        case ID_GAME_CUSTOMSIZE:
        {
            DialogBox(hInst, MAKEINTRESOURCE(IDD_CUSTOMSIZE), hWnd, (DLGPROC)CustomSizeProc);
            if (RESET_GAME)
            {
                RESET_GAME = FALSE;
                ResetGame(hWnd);
            }
            SetTimer(hWnd, 1, 50, NULL);
            break;
        }
        case ID_GAME_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_CREATE:
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        TIME_ST* pState = reinterpret_cast<TIME_ST*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pState));
        SetTimer(hWnd, 1, 50, NULL);
        break;
    }
    case WM_TIMER:
    {
        if (wParam == 1 && !GAME_STOPPED)
        {
            TIME_ST* pS = reinterpret_cast<TIME_ST*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
            ULONGLONG ms;
            if (!STARTED)
            {
                pS->begin = GetTickCount64();
                ms = 0;
            }
            else
            {
                ms = GetTickCount64();
                ms = (ms - pS->begin) / 100;
            }

            HDC hdc = GetDC(hWnd);

            HFONT hFont = CreateFont(25, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial Bold"));
            SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(255, 0, 0));

            int msec = ms % 10;
            int sec = ms / 10;

            TCHAR s[7], pts[5];
            RECT rect, ptsRect;
            
            GetClientRect(hWnd, &rect);
            
            rect.bottom = rect.top + TOP_PANEL;
            ptsRect.right = rect.right;
            rect.right = rect.right - (rect.right - rect.left) / 2;
            ptsRect.top = rect.top;
            ptsRect.bottom = rect.bottom;
            ptsRect.left = rect.right;
            
            _stprintf_s(s, 7, _T("%04d.%d"), sec, msec);
            _stprintf_s(pts, 5, _T("%04d"), BombsLeft);
            //SetWindowText(hWnd, s);

            DrawText(hdc, s, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            DrawText(hdc, pts, -1, &ptsRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            DeleteObject(hFont);
            ReleaseDC(hWnd, hdc);
            SetTimer(hWnd, 1, 50, NULL);
        }
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WndPieceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }
    case WM_CREATE:
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        MINE_ST* pState = reinterpret_cast<MINE_ST*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pState));
        break;
    }
    case WM_RBUTTONDOWN:
    {
        if (GAME_STOPPED) break;
        MINE_ST* tile = reinterpret_cast<MINE_ST*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (tile->isFlagged)
        {
            BombsLeft++;
            tile->isFlagged = false;
            RECT rect;
            GetClientRect(hWnd, &rect);
            //InvalidateRect(hWnd, &rect, TRUE);
            HDC hdc = GetDC(hWnd);
            FillRect(hdc, &rect, (HBRUSH)(COLOR_GRAYTEXT + 1));
            if (DEBUG)
            {
                ShowTile(tile->xPos, tile->yPos, hdc);
            }
            ReleaseDC(hWnd, hdc);
        }
        else if (!tile->isShown && BombsLeft > 0)
        {
            BombsLeft--;
            tile->isFlagged = true;
            HDC hdc = GetDC(hWnd);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_FLAG));
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
            //BitBlt(hdc, 0, 0, PIECE_SIZE, PIECE_SIZE, memDC, 0, 0, SRCCOPY);
            StretchBlt(hdc, 0, 0, PIECE_SIZE, PIECE_SIZE, memDC, 0, 0, 48, 48, SRCCOPY);
            DeleteObject(oldBitmap);
            ReleaseDC(hWnd, hdc);
            if (CheckIfEnded())
            {
                SendMessage(GetAncestor(hWnd, GA_PARENT), WM_TIMER, 1, 0);
                GAME_STOPPED = TRUE;
                MessageBox(hWnd, L"Win!", L"Minesweeper", MB_OK);
            }
        }
        break;
    }
    case WM_LBUTTONDOWN:
    {
        if (GAME_STOPPED) break;
        if (!STARTED)
        {
            STARTED = TRUE;
        }
        MINE_ST* tile = reinterpret_cast<MINE_ST*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        UncoverTile(hWnd, tile);
        if (!GAME_STOPPED && CheckIfEnded())
        {
            SendMessage(GetAncestor(hWnd, GA_PARENT), WM_TIMER, 1, 0);
            GAME_STOPPED = TRUE;
            MessageBox(hWnd, L"Win!", L"Minesweeper", MB_OK);
        }
        break;
    }
    case WM_DESTROY:
    {
        MINE_ST* tile = reinterpret_cast<MINE_ST*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        delete tile;
        break;
    }
    case WM_PAINT:
    {
        HDC hdc = BeginPaint(hWnd, &ps);
        
        MINE_ST* tile = reinterpret_cast<MINE_ST*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (tile->isShown)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            HPEN pen = CreatePen(PS_SOLID, 1, SHOWN_TILE);
            HPEN oldPen = (HPEN)SelectObject(hdc, pen);
            HBRUSH brush = CreateSolidBrush(SHOWN_TILE);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
            Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
            SelectObject(hdc, oldPen);
            DeleteObject(pen);
            SelectObject(hdc, oldBrush);
            DeleteObject(brush);

            ShowTile(tile->xPos, tile->yPos, hdc);
        }
        EndPaint(hWnd, &ps);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL UncoverTile(HWND hWnd, MINE_ST* tile)
{
    if (tile->isShown || tile->isFlagged)
    {
        return 0;
    }
    HDC hdc = GetDC(hWnd);
    RECT rc;
    GetClientRect(hWnd, &rc);
    int x = tile->xPos;
    int y = tile->yPos;
    tile->isShown = true;

    HPEN pen = CreatePen(PS_SOLID, 1, SHOWN_TILE);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);
    HBRUSH brush = CreateSolidBrush(SHOWN_TILE);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);

    ShowTile(x, y, hdc);


    if (!tile->hasMine)
    {
        if (tile->neighbors <= 0)
        {
            if (x - 1 >= 0)
            {
                UncoverTile(szPieces[x - 1][y], Mines[x - 1][y]);
                if (y - 1 >= 0)
                {
                    UncoverTile(szPieces[x - 1][y - 1], Mines[x - 1][y - 1]);
                }
                if (y + 1 < yNoObjects)
                {
                    UncoverTile(szPieces[x - 1][y + 1], Mines[x - 1][y + 1]);
                }
            }
            if (x + 1 < xNoObjects)
            {
                UncoverTile(szPieces[x + 1][y], Mines[x + 1][y]);
                if (y - 1 >= 0)
                {
                    UncoverTile(szPieces[x + 1][y - 1], Mines[x + 1][y - 1]);
                }
                if (y + 1 < yNoObjects)
                {
                    UncoverTile(szPieces[x + 1][y + 1], Mines[x + 1][y + 1]);
                }
            }
            if (y - 1 >= 0)
            {
                UncoverTile(szPieces[x][y - 1], Mines[x][y - 1]);
            }
            if (y + 1 < yNoObjects)
            {
                UncoverTile(szPieces[x][y + 1], Mines[x][y + 1]);
            }
        }
    }
    else
    {
        GAME_STOPPED = TRUE;
        MessageBox(hWnd, L"Mine", L"Boom!", MB_OK | MB_ICONERROR);
    }
    ReleaseDC(hWnd, hdc);
    return 0;
}

BOOL InitPieces(HWND parent)
{
    for (int x = 0; x < MAX_X_SIZE; x++)
    {
        for (int y = 0; y < MAX_Y_SIZE; y++)
        {
            InitSingleTile(x, y, parent);
        }
    }
    InitBombs();

    return 0;
}


BOOL InitSingleTile(int x, int y, HWND parent)
{
    Mines[x][y] = new MINE_ST;
    SetDefaultMine(x, y);
    szPieces[x][y] = CreateWindowW(szPieceClass, szTitle, WS_VISIBLE | WS_CHILD,
        1 + x * (1 + PIECE_SIZE), TOP_PANEL + 1 + y * (1 + PIECE_SIZE), PIECE_SIZE, PIECE_SIZE,
        parent, nullptr, hInst, Mines[x][y]);

    return 0;
}

BOOL SetDefaultMine(int x, int y)
{
    Mines[x][y]->hasMine = false;
    Mines[x][y]->isShown = false;
    Mines[x][y]->isFlagged = false;
    Mines[x][y]->neighbors = 0;
    Mines[x][y]->xPos = x;
    Mines[x][y]->yPos = y;
    return 0;
}

BOOL InitBombs()
{
    int count = nMines;
    while (count != 0)
    {
        int x = rand() % xNoObjects;
        int y = rand() % yNoObjects;
        if (!(Mines[x][y]->hasMine))
        {
            Mines[x][y]->hasMine = true;
            if (x - 1 >= 0)
            {
                Mines[x - 1][y]->neighbors++;
                if (y - 1 >= 0)
                {
                    Mines[x - 1][y - 1]->neighbors++;
                }
                if (y + 1 < yNoObjects)
                {
                    Mines[x - 1][y + 1]->neighbors++;
                }
            }
            if (x + 1 < xNoObjects)
            {
                Mines[x + 1][y]->neighbors++;
                if (y - 1 >= 0)
                {
                    Mines[x + 1][y - 1]->neighbors++;
                }
                if (y + 1 < yNoObjects)
                {
                    Mines[x + 1][y + 1]->neighbors++;
                }
            }
            if (y - 1 >= 0)
            {
                Mines[x][y - 1]->neighbors++;
            }
            if (y + 1 < yNoObjects)
            {
                Mines[x][y + 1]->neighbors++;
            }
            count--;
        }
    }
    BombsLeft = nMines;
    return 0;
}


BOOL ResetGame(HWND hWnd)
{
    int x = GetSystemMetrics(SM_CXSCREEN);
    int y = GetSystemMetrics(SM_CYSCREEN);

    for (int i = 0; i < xNoObjects; i++)
    {
        for (int j = 0; j < yNoObjects; j++)
        {
            SetDefaultMine(i, j);
            HideTile(i, j);
        }
    }

    xNoObjects = nxNoObjects;
    yNoObjects = nyNoObjects;
    
    InitBombs();

    RECT rt = { 0, 0, xNoObjects * (PIECE_SIZE + 1) + 1, yNoObjects * (PIECE_SIZE + 1) + 1 + TOP_PANEL };
    AdjustWindowRect(&rt, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MINIMIZEBOX ^ WS_MAXIMIZEBOX, TRUE);

    MoveWindow(hWnd, (x - rt.right) / 2, (y - rt.bottom) / 2, rt.right - rt.left, rt.bottom - rt.top, TRUE);
    
    GAME_STOPPED = FALSE;
    STARTED = FALSE;
    DEBUG = FALSE;
    return 0;
}

BOOL ShowTile(int x, int y, HDC hdc)
{
    RECT rect;

    //HFONT hFont = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
    //    CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial Bold"));
    //HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
    GetClientRect(szPieces[x][y], &rect);
    if (Mines[x][y]->isShown)
        SetBkColor(hdc, SHOWN_TILE);
    else
        SetBkColor(hdc, RGB(0x6D, 0x6D, 0x6D));
    if (Mines[x][y]->hasMine)
    {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);

    }
    else if (Mines[x][y]->neighbors > 0)
    {
        SetTextColor(hdc, COLORS[Mines[x][y]->neighbors - 1]);
        TCHAR s[2];
        _stprintf_s(s, 2, _T("%d"), Mines[x][y]->neighbors);
        DrawText(hdc, s, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    //SelectObject(hdc, oldFont);
    //DeleteObject(hFont);
    return 0;
}

BOOL HideTile(int x, int y)
{
    if (!Mines[x][y]->isShown && !Mines[x][y]->isFlagged)
    {
        RECT rect;
        GetClientRect(szPieces[x][y], &rect);
        InvalidateRect(szPieces[x][y], &rect, TRUE);
    }
    return 0;
}


BOOL CheckIfEnded()
{
    int unshownTiles = 0;
    int realBombsLeft = nMines;
    for (int x = 0; x < xNoObjects; x++)
    {
        for (int y = 0; y < yNoObjects; y++)
        {
            //if ((Mines[x][y]->hasMine && !Mines[x][y]->isFlagged) || (!Mines[x][y]->isShown && !Mines[x][y]->isFlagged)) return FALSE;
            if (!Mines[x][y]->isShown && !Mines[x][y]->isFlagged) unshownTiles++;
            if (Mines[x][y]->hasMine && Mines[x][y]->isFlagged) realBombsLeft--;
        }
    }

    return BombsLeft == unshownTiles || realBombsLeft == 0;
}

BOOL CALLBACK CustomSizeProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    int ctrl;
    int xs, ys, ms;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        SetDlgItemInt(hwndDlg, IDC_XSIZE, nxNoObjects, TRUE);
        SetDlgItemInt(hwndDlg, IDC_YSIZE, nyNoObjects, TRUE);
        SetDlgItemInt(hwndDlg, IDC_MINES, nMines, TRUE);
        break;
    }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (xs = GetDlgItemInt(hwndDlg, IDC_XSIZE, &ctrl, FALSE))
                if (xs < MIN_X_SIZE)
                    nxNoObjects = MIN_Y_SIZE;
                else if (xs > MAX_X_SIZE)
                    nxNoObjects = MAX_X_SIZE;
                else
                    nxNoObjects = xs;
            if (ys = GetDlgItemInt(hwndDlg, IDC_YSIZE, &ctrl, FALSE))
                if (ys < MIN_Y_SIZE)
                    nyNoObjects = MIN_Y_SIZE;
                else if (ys > MAX_Y_SIZE)
                    nyNoObjects = MAX_Y_SIZE;
                else
                    nyNoObjects = ys;
            if (ms = GetDlgItemInt(hwndDlg, IDC_MINES, &ctrl, FALSE))
                if (ms < MIN_MINES)
                    nMines = MIN_MINES;
                else if (ms > nyNoObjects * nxNoObjects)
                    nMines = nyNoObjects * nxNoObjects;
                else
                    nMines = ms;
            // Fall through.
            RESET_GAME = TRUE;

        case IDCANCEL:
            EndDialog(hwndDlg, wParam);
            return TRUE;
        }
    }
    return FALSE;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}