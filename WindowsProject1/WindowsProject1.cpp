#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <gdiplus.h>
#include <cstdlib>
#include <cmath>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

ULONG_PTR gdiplusToken;

//---------------------------------------- Параметры ----------------------------------------//

const int BrickLines = 3;
const int BrickStolbs = 10;
int BrickHeigth = 25;

int BallSize = 25;
float BollSpeed = 10;

int PlatformWidth = 100;
int PlatformHeight = 20;
int Speed = 5;

//----------------------------------------Техчасть-----------------------------------------//

bool GameStatus = false;
int PlayerPosX = 0;
int PlayerPosY = 100;
int PlayerVector = 0;
int BollPosX = 300;
int BollPosY = 300;
float BollAngle = 45.0f * (3.14159f / 180.0f);
float BollVectorX = 0;
float BollVectorY = 0;

struct brickst {
    int posx = 0;
    int posy = 0;
    int H = 0;
    int W = 0;
    bool live = true;
};

brickst brick[BrickStolbs][BrickLines];

void InitGDIPlus()
{
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

void ShutdownGDIPlus()
{
    GdiplusShutdown(gdiplusToken);
}

void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

    // Очистка фона
    HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdcMem, &rect, hBrush);
    DeleteObject(hBrush);

    Graphics graphics(hdcMem);
    graphics.SetSmoothingMode(SmoothingModeAntiAlias);

    SolidBrush BlackBrush(Color(255, 0, 0, 0));
    SolidBrush BlueBrush(Color(255, 0, 0, 255));
    SolidBrush GreenBrush(Color(255, 0, 255, 0));

    // Рисуем платформу
    int platformY = height - 100;
    int platformX = (width / 2) - (PlatformWidth / 2) + PlayerPosX;
    graphics.FillRectangle(&BlueBrush, platformX, platformY, PlatformWidth, PlatformHeight);

    // Рисуем мяч
    graphics.FillEllipse(&GreenBrush, BollPosX, BollPosY, BallSize, BallSize);

    for (int y = 0; y < BrickLines; y++) {
        for (int x = 0; x < BrickStolbs; x++) {
            if (brick[x][y].live) {
                graphics.FillRectangle(&BlackBrush, brick[x][y].posx, brick[x][y].posy, brick[x][y].W, brick[x][y].H);
            }
        }
    }

    // Отладка
    WCHAR debugText[100];
    swprintf(debugText, 100, L"Angle: %.2f°", BollAngle * (180.0f / 3.14159f));
    Font font(L"Arial", 12);
    SolidBrush textBrush(Color(255, 0, 0, 0));
    graphics.DrawString(debugText, -1, &font, PointF(10, 10), &textBrush);

    BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);

    EndPaint(hWnd, &ps);
}

//----------------------------------------Основа-----------------------------------------//

void brickcreate(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;;

    for (int y = 0; y < BrickLines; y++)
    {
        for (int x = 0; x < BrickStolbs; x++) {
            brick[x][y].W = width / BrickStolbs;
            brick[x][y].posx = x * (width / BrickStolbs);
            brick[x][y].posy = y * BrickHeigth;
            brick[x][y].H = BrickHeigth;
            brick[x][y].live = true;
        }
    }
}

void StartBattleAnimation(HWND hWnd)
{
    GameStatus = true;
    SetTimer(hWnd, 1, 8, NULL);
    brickcreate(hWnd);
}

void StopBattleAnimation(HWND hWnd)
{
    GameStatus = false;
    KillTimer(hWnd, 1);
}

void GamePlay(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Движение платформы
    int platformX = (width / 2) - (PlatformWidth / 2) + PlayerPosX;
    int newPlatformX = platformX + static_cast<int>(Speed * PlayerVector);

    int platformY = height - 100;
    platformX = (width / 2) - (PlatformWidth / 2) + PlayerPosX;

    if (newPlatformX >= 0 && newPlatformX <= width - PlatformWidth) {
        PlayerPosX += static_cast<int>(Speed * PlayerVector);
        if (BollPosY > height - PlayerPosY && BollPosY < height - PlayerPosY + PlatformHeight) {
            if (BollPosX > PlayerPosX + 5 && BollPosX < PlayerPosX - 5) {
                BollPosX += static_cast<int>(Speed * PlayerVector);
            }
            else if (BollPosX > PlayerPosX + PlatformWidth - 5 && BollPosX < PlayerPosX + PlatformWidth + 5) {
                BollPosX += static_cast<int>(Speed * PlayerVector);
            }
        }
    }

    // Движение мяча
    int NewBollPosX = BollPosX + static_cast<int>(BollSpeed * cos(BollAngle))/2;
    int NewBollPosY = BollPosY + static_cast<int>(BollSpeed * sin(BollAngle))/2;

    int DistanceX = NewBollPosX - BollPosX;
    int DistanceY = NewBollPosY - BollPosY;

    int steps = max(abs(DistanceX), abs(DistanceY));

    // Проверка столкновений 
    for (int i = 0; i <= steps; i++) {
        double t = (double)i / steps;
        int CheckX = BollPosX + round(DistanceX * t);
        int CheckY = BollPosY + round(DistanceY * t);


        //Стены
        if (CheckX <= 0) {
            BollPosX = CheckX+BallSize/2;
            BollAngle = 3.14159f - BollAngle;
            break;
        }
        else if (CheckX >= width - BallSize) {
            BollPosX = width - BallSize - 1;
            BollAngle = 3.14159f - BollAngle;
            break;
        }
        if (CheckY <= 0) {
            BollPosY = BallSize / 2;             
            BollAngle = -BollAngle;
            break;
        }
        if (BollPosY > height) { 
            BollPosX = width / 2;
            BollPosY = height / 2;
            BollAngle = 45.0f * (3.14159f / 180.0f);
            StopBattleAnimation(hWnd);
            break;
        }

        //Платформа углы
        
        struct Corner {
            float x, y;
        };

        Corner corners[4] = {
       {platformX, platformY}, // левый верхний
       {platformX + PlatformWidth, platformY}, // правый верхний
       {platformX, platformY + PlatformHeight}, // левый нижний
       {platformX + PlatformWidth, platformY + PlatformHeight} // правый нижний
        };

        float ballRadius = BallSize / 2.0f;

        for (int i = 0; i < 4; i++) {
            float dx = BollPosX - corners[i].x;
            float dy = BollPosY + BallSize / 2.0f; - corners[i].y;
            float distance = sqrt(dx * dx + dy * dy);

            // Если расстояние меньше радиуса мяча + небольшой зазор
            if (distance < ballRadius + 2.0f) {
                // Вычисляем нормаль от угла к центру мяча
                float normalX = dx / distance;
                float normalY = dy / distance;

                // Отражаем угол мяча относительно нормали
                float dotProduct = cos(BollAngle) * normalX + sin(BollAngle) * normalY;
                BollAngle = atan2(sin(BollAngle) - 2 * dotProduct * normalY,
                    cos(BollAngle) - 2 * dotProduct * normalX);

                BollAngle += (rand() % 10 - 5) * 0.01f;
                break;
            }
        }

        //Платформа края
        if (CheckY + BallSize >= platformY &&
            CheckY <= platformY + PlatformHeight &&
            CheckX + BallSize >= platformX &&
            CheckX <= platformX + PlatformWidth) {

            BollPosX = CheckX;
            BollPosY = CheckY;


            if (BollPosX + BallSize / 2.0f < platformX && cos(BollAngle) > 0) {
                BollPosX = platformX - BallSize - 5;
                BollAngle = 3.14159f - BollAngle;

            }
            else if (BollPosX + BallSize / 2.0f > platformX + PlatformWidth && cos(BollAngle) < 0) {
                BollPosX = platformX + PlatformWidth + 5;
                BollAngle = 3.14159f - BollAngle;
            }
            else {
                BollPosY = platformY - BallSize;
                BollAngle = -BollAngle;
            }

            BollAngle += (rand() % 10 - 5) * 0.01f;
        }

        //кирпичи
        for (int y = 0; y < BrickLines; y++) {
            for (int x = 0; x < BrickStolbs; x++) {
                if (brick[x][y].live == true) {
                    if (CheckY + BallSize >= brick[x][y].posy &&
                        CheckY <= brick[x][y].posy + brick[x][y].H &&
                        CheckX + BallSize >= brick[x][y].posx &&
                        CheckX <= brick[x][y].posx + brick[x][y].W) {

                        float ballCenterX = CheckX + BallSize / 2.0f;
                        float ballCenterY = CheckY + BallSize / 2.0f;
                        float platformCenterX = brick[x][y].posx + brick[x][y].W / 2.0f;

                        BollPosX = CheckX;
                        BollPosY = CheckY;

                        if (ballCenterX < brick[x][y].posx && cos(BollAngle) > 0) {
                            BollAngle = 3.14159f - BollAngle;
                        }
                        else if (ballCenterX > brick[x][y].posx + (width / BrickStolbs) && cos(BollAngle) < 0) {
                            BollAngle = 3.14159f - BollAngle;
                        }
                        else if (ballCenterY > brick[x][y].posy) {
                            BollPosY = BollPosY + 10;
                            BollAngle = -BollAngle;
                        }
                        else if (ballCenterY < brick[x][y].posy) {
                            BollPosY = BollPosY - 10;
                            BollAngle = -BollAngle;
                        }

                        BollAngle += (rand() % 10 - 5) * 0.01f;

                        brick[x][y].live = false;
                    }
                    else {
                        BollPosX = NewBollPosX;
                        BollPosY = NewBollPosY;
                    }
                }
            }
        }

    }



    InvalidateRect(hWnd, NULL, TRUE);
}

//----------------------------------------Окно-----------------------------------------//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {

    case WM_ERASEBKGND:
        return 1;

    case WM_CREATE:
        break;

    case WM_PAINT:
        OnPaint(hWnd);
        break;

    case WM_TIMER:
        if (wParam == 1) {
            GamePlay(hWnd);
        }
        break;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_SPACE:
            if (GameStatus == false) {
                StartBattleAnimation(hWnd);
            }
            break;

        case VK_LEFT:
            PlayerVector = -1;
            break;

        case VK_RIGHT:
            PlayerVector = 1;
            break;

        case VK_ESCAPE:
            PostQuitMessage(0);
            break;
        }
        break;

    case WM_KEYUP:
        switch (wParam) {
        case VK_LEFT:
            if (PlayerVector == -1) {
                PlayerVector = 0;
            }
            break;
      
        case VK_RIGHT:
            if (PlayerVector == 1) {
                PlayerVector = 0;
            }
            break;
        }
        break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* mmi = (MINMAXINFO*)lParam;
        mmi->ptMinTrackSize.x = 800;
        mmi->ptMinTrackSize.y = 600;
        mmi->ptMaxTrackSize.x = 800;
        mmi->ptMaxTrackSize.y = 600;
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    InitGDIPlus();

    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"GDIPlusWindowClass";

    if (!RegisterClassEx(&wcex))
        return 0;

    RECT rect = { 0, 0, 800, 600 };
    AdjustWindowRect(&rect, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HWND hWnd = CreateWindow(
        L"GDIPlusWindowClass", L"Pong Game",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, hInstance, NULL);

    if (!hWnd)
        return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ShutdownGDIPlus();
    return (int)msg.wParam;
}