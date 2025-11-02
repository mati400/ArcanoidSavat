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
float BollSpeed = 20;

int PlatformWidth = 100;
int PlatformHeight = 20;
int Speed = 5;

//----------------------------------------Техчасть-----------------------------------------//

bool GameStatus = false;

int ManageWindowWidhtProcent = 2;
int ManageWindowWidhtPixel = 10;
int PlayerPosX = 300;
int PlayerPosY = 100;
int PlayerVector = 0;

int BollPosX = 300;
int BollPosY = 300;
float BollAngle = 45.0f * (3.14159f / 180.0f);
float BollVectorX = 0;
float BollVectorY = 0;

bool MouseCanTouchBoll = false;
bool BollOnMouse = false;
int MouseX = 0;
int MouseY = 0;

int ManageBigCircleRadius = 100;
int ManageSmallCircleRadius = 10;
int radius1 = 0;
int center2X = 0;
int center2Y = 0;

int brickCount = BrickLines * BrickStolbs;
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
    SolidBrush LightBlue(Color(255, 180, 220, 240));
    Pen outlinePen(Color(255, 0, 100, 200), 2.0f);
    Pen GreenLine(Color(255, 0, 255, 0), 2.0f);

    // Рисуем платформу
    int platformY = height - 100;
    int platformX = PlayerPosX;
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

    //Поле отладки
    graphics.FillRectangle(&LightBlue, (width / 10) * (10 - ManageWindowWidhtProcent), 0 , (width / 10) * ManageWindowWidhtProcent,height);

    //большйо круг
    int ManageCircle1X = (width / 10) * (10 - ManageWindowWidhtProcent) + ((width / 10 * ManageWindowWidhtProcent) / 2) - ManageBigCircleRadius / 2;
    int ManageCircle1Y = 50;
    int center1X = ManageCircle1X + ManageBigCircleRadius / 2;
    int center1Y = ManageCircle1Y + ManageBigCircleRadius / 2;
    Rect ManageCircle1(ManageCircle1X, ManageCircle1Y, ManageBigCircleRadius, ManageBigCircleRadius);
    graphics.DrawEllipse(&outlinePen, ManageCircle1);

    //малый круг
    radius1 = ManageBigCircleRadius / 2;
    center2X = center1X + (radius1 + ManageSmallCircleRadius - ManageSmallCircleRadius / 2 - 4) * cos(BollAngle) ;
    center2Y = center1Y + (radius1 + ManageSmallCircleRadius - ManageSmallCircleRadius / 2 - 4) * sin(BollAngle) ;
    Rect ManageCircle2(
        center2X - ManageSmallCircleRadius,
        center2Y - ManageSmallCircleRadius,
        ManageSmallCircleRadius * 2,
        ManageSmallCircleRadius * 2
    );
    graphics.FillEllipse(&GreenBrush, ManageCircle2);

    //Линия направления
    Point centerPoint2(center2X, center2Y);
    Point centerPoint1(center1X, center1Y);
    graphics.DrawLine(&GreenLine, centerPoint1, centerPoint2);

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
    int width = rect.right - rect.left;
    int GameWight = (width / 10) * (10 - ManageWindowWidhtProcent);

    for (int y = 0; y < BrickLines; y++)
    {
        for (int x = 0; x < BrickStolbs; x++) {
            brick[x][y].W = GameWight / BrickStolbs;
            brick[x][y].posx = x * (GameWight / BrickStolbs);
            brick[x][y].posy = 30 + y * BrickHeigth;
            brick[x][y].H = BrickHeigth;
            brick[x][y].live = true;
        }
    }
}

void StartBattleAnimation(HWND hWnd)
{
    GameStatus = true;
    SetTimer(hWnd, 1, 18, NULL);
    brickcreate(hWnd);
}

void StopBattleAnimation(HWND hWnd)
{
    GameStatus = false;
    KillTimer(hWnd, 1);
}


struct CollisionResult {
    bool collision;
    float newAngle;  
    int newPosX, newPosY;    
};

CollisionResult CheckObjectCollision(float ballCheckX, float ballCheckY, float ballRadius,
    float currentBallAngle, float objX, float objY, float objWidth, float objHeight) {

    CollisionResult result = { false, currentBallAngle, 0, 0 };
    float ballCenterX = ballCheckX + ballRadius;
    float ballCenterY = ballCheckY + ballRadius;

    float closestX = max(objX, min(ballCenterX, objX + objWidth));
    float closestY = max(objY, min(ballCenterY, objY + objHeight));

    float dx = ballCenterX - closestX;
    float dy = ballCenterY - closestY;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < ballRadius) {
        result.collision = true;
        float normalX = dx / distance;
        float normalY = dy / distance;

        float dotProduct = cos(currentBallAngle) * normalX + sin(currentBallAngle) * normalY;
        result.newAngle = atan2(sin(currentBallAngle) - 2 * dotProduct * normalY,
        cos(currentBallAngle) - 2 * dotProduct * normalX);

        result.newPosX = static_cast<int>(closestX + normalX * ballRadius - ballRadius);
        result.newPosY = static_cast<int>(closestY + normalY * ballRadius - ballRadius);
    }

    return result;
}

void GamePlay(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int MainWidht = rect.right - rect.left;
    int width = (MainWidht / 10) * (10 - ManageWindowWidhtProcent);
    /*int width = rect.right - rect.left;*/
    int height = rect.bottom - rect.top;
    
    // Движение платформы
    int platformX = PlayerPosX;
    int newPlatformX = platformX + static_cast<int>(Speed * PlayerVector);
    int platformY = height - 100;

    if (MouseCanTouchBoll == true && MouseX > BollPosX-15 && MouseX < BollPosX + BallSize + 15 && MouseY > BollPosY-15 && MouseY < BollPosY + BallSize + 15) {
        BollPosY = MouseY - BallSize/2;
        BollPosX = MouseX - BallSize/2;
        
        //BollSpeed = 0;
    }
        


    if (newPlatformX >= 0 && newPlatformX <= width - PlatformWidth) {
        PlayerPosX += static_cast<int>(Speed * PlayerVector);
    }
    
    // Движение мяча
    int NewBollPosX = BollPosX + static_cast<int>(BollSpeed * cos(BollAngle)) / 2;
    int NewBollPosY = BollPosY + static_cast<int>(BollSpeed * sin(BollAngle)) / 2;

    int DistanceX = NewBollPosX - BollPosX;
    int DistanceY = NewBollPosY - BollPosY;

    int steps = max(abs(DistanceX), abs(DistanceY));
    bool collisionOccurred = false;

    // Проверка столкновений 
    for (int i = 0; i <= steps; i++) {
        double t = (double)i / steps;
        int CheckX = BollPosX + round(DistanceX * t);
        int CheckY = BollPosY + round(DistanceY * t);

        float ballRadius = BallSize / 2.0f;
        float ballCenterX = CheckX + ballRadius;
        float ballCenterY = CheckY + ballRadius;

        // Стены
        if (ballCenterX - ballRadius <= 0) {
            BollPosX = BollPosX + 1;
            BollAngle = 3.14159f - BollAngle;
            collisionOccurred = true;
            break;
        }
        else if (ballCenterX + ballRadius >= width) {
            BollPosX = BollPosX - 1;
            BollAngle = 3.14159f - BollAngle;
            collisionOccurred = true;
            break;
        }
        else if (ballCenterY - ballRadius <= 0) {
            BollPosY = BollPosY + 1;
            BollAngle = -BollAngle;
            collisionOccurred = true;
            break;
        }
        if (CheckY > height) {
            BollPosX = width / 2;
            BollPosY = height / 2;
            BollAngle = 45.0f * (3.14159f / 180.0f);
            StopBattleAnimation(hWnd);
            collisionOccurred = true;
            break;
        }

        // Проверка столкновения с платформой
        CollisionResult platformCollision = CheckObjectCollision(
            CheckX, CheckY, ballRadius, BollAngle, platformX, platformY, PlatformWidth, PlatformHeight
        );

        if (platformCollision.collision) {
            BollAngle = platformCollision.newAngle;
            BollAngle += (rand() % 10 - 5) * 0.01f;

            if (platformCollision.newPosX != 0 || platformCollision.newPosY != 0) {
                BollPosX = platformCollision.newPosX;
                BollPosY = platformCollision.newPosY;
            }

            collisionOccurred = true;
            break;
        }

        // Проверка кирпичей
        bool brickCollisionFound = false;
        for (int y = 0; y < BrickLines; y++) {
            for (int x = 0; x < BrickStolbs; x++) {
                if (brick[x][y].live) {
                    CollisionResult brickCollision = CheckObjectCollision(
                        CheckX, CheckY, ballRadius,
                        BollAngle,
                        brick[x][y].posx,
                        brick[x][y].posy,
                        brick[x][y].W,
                        brick[x][y].H
                    );

                    if (brickCollision.collision) {
                        BollAngle = brickCollision.newAngle;
                        brick[x][y].live = false;
                        brickCollisionFound = true;

                        if (brickCollision.newPosX != 0 || brickCollision.newPosY != 0) {
                            BollPosX = brickCollision.newPosX;
                            BollPosY = brickCollision.newPosY;
                        }
                        collisionOccurred = true;
                        break;
                    }
                }
            }
            if (brickCollisionFound) {
                break;
            }  
        }

        if (brickCollisionFound) {
            break;
        }
    }

    if (!collisionOccurred) {
        BollPosX = NewBollPosX;
        BollPosY = NewBollPosY;
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

        case WM_LBUTTONDOWN:
            MouseCanTouchBoll = true;
            break;

        case WM_LBUTTONUP:
            MouseCanTouchBoll = false;
            break;

        case WM_MOUSEMOVE:
            MouseX = LOWORD(lParam);
            MouseY = HIWORD(lParam);
            break;

        case MOUSEEVENTF_RIGHTDOWN:
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