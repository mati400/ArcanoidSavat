#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string>
#include <gdiplus.h>
#include <cmath>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

ULONG_PTR gdiplusToken;

//---------------------------------------- Параметры ----------------------------------------//

const int BrickLines = 3;
const int BrickStolbs = 10;
int BrickHeigth = 25;

int BallSize = 50;
float BollSpeed = 20.0f;

int PlatformWidth = 100;
int PlatformHeight = 20;
int Speed = 5;

const int MaxRedLines = 4;

//----------------------------------------Техчасть-----------------------------------------//

bool GameStatus = false;
const float PI = 3.14159265358979323846f;

int PlayerPosX = 300;
int PlayerPosY = 100;
int PlayerVector = 0;

int BollPosX = 300;
int BollPosY = 300;
float BollAngle = 45.0f * (PI / 180.0f);

bool MouseWasUpped = true;
bool MouseCanTouchBoll = false;
int MouseTarget = 0;
int MouseX = 0;
int MouseY = 0;

int center1Y = 0;
int center1X = 0;
int ManageWindowWidhtProcent = 2;
int ManageBigCircleRadius = 100;
int ManageSmallCircleRadius = 10;
int center2X = 0;
int center2Y = 0;

struct RedLine {
    int FirstX = 0;
    int FirstY = 0;
    int LastX = 0;
    int LastY = 0;
    float NewAngle = 0.0f;
};

RedLine Line[MaxRedLines];

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
    Pen RedPen(Color(255, 0, 0), 2.0f);
    Pen outlinePen(Color(255, 0, 100, 200), 2.0f);
    Pen GreenLine(Color(255, 0, 255, 0), 2.0f);
    SolidBrush textBrush(Color(255, 0, 0, 0));

    Font font(L"Arial", 12);

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

    WCHAR debugText2[100];
    swprintf(debugText2, 100, L"Angle: %.2f°", BollAngle * (180.0f / PI));
    graphics.DrawString(debugText2, -1, &font, PointF(10.0f, 10.0f), &textBrush);

    WCHAR debugText[100];
    swprintf(debugText, 100, L"Angle: %.2f°", Line[0].NewAngle * (180.0f / PI));
    graphics.DrawString(debugText, -1, &font, PointF(130.0f, 10.0f), &textBrush);

    if (Line[1].NewAngle)
    {
        WCHAR debugText[100];
        swprintf(debugText, 100, L"Angle: %.2f°", Line[1].NewAngle * (180.0f / PI));
        graphics.DrawString(debugText, -1, &font, PointF(250.0f, 10.0f), &textBrush);
    }

    if (Line[2].NewAngle) {
        WCHAR debugText[100];
        swprintf(debugText, 100, L"Angle: %.2f°", Line[2].NewAngle * (180.0f / PI));
        graphics.DrawString(debugText, -1, &font, PointF(380.0f, 10.0f), &textBrush);
    }

    //Поле отладки
    graphics.FillRectangle(&LightBlue,
        static_cast<int>((width / 10.0f) * (10 - ManageWindowWidhtProcent)),
        0,
        static_cast<int>((width / 10.0f) * ManageWindowWidhtProcent),
        height);

    //большой круг
    int ManageCircle1X = static_cast<int>((width / 10.0f) * (10 - ManageWindowWidhtProcent) +
        ((width / 10.0f * ManageWindowWidhtProcent) / 2.0f) - ManageBigCircleRadius / 2.0f);
    int ManageCircle1Y = 50;
    center1X = ManageCircle1X + ManageBigCircleRadius / 2;
    center1Y = ManageCircle1Y + ManageBigCircleRadius / 2;
    Rect ManageCircle1(ManageCircle1X, ManageCircle1Y, ManageBigCircleRadius, ManageBigCircleRadius);
    graphics.DrawEllipse(&outlinePen, ManageCircle1);

    //малый круг
    int radius1 = ManageBigCircleRadius / 2;
    center2X = static_cast<int>(center1X + (radius1 + ManageSmallCircleRadius - ManageSmallCircleRadius / 2.0f - 4.0f) * cos(BollAngle));
    center2Y = static_cast<int>(center1Y + (radius1 + ManageSmallCircleRadius - ManageSmallCircleRadius / 2.0f - 4.0f) * sin(BollAngle));
    Rect ManageCircle2(
        center2X - ManageSmallCircleRadius,
        center2Y - ManageSmallCircleRadius,
        ManageSmallCircleRadius * 2,
        ManageSmallCircleRadius * 2
    );
    graphics.FillEllipse(&GreenBrush, ManageCircle2);

    //Линия направления отладки
    Point centerPoint2(center2X, center2Y);
    Point centerPoint1(center1X, center1Y);
    graphics.DrawLine(&GreenLine, centerPoint1, centerPoint2);

    //Траектория пути
    for (int i = 0; i < MaxRedLines; i++) {
        Point Line1(Line[i].FirstX, Line[i].FirstY);
        Point Line2(Line[i].LastX, Line[i].LastY);
        graphics.DrawLine(&RedPen, Line1, Line2);
    }

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
    int GameWight = static_cast<int>((width / 10.0f) * (10 - ManageWindowWidhtProcent));

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

    // Расширяем блок на радиус
    float expandedLeft = objX - ballRadius;
    float expandedRight = objX + objWidth + ballRadius;
    float expandedTop = objY - ballRadius;
    float expandedBottom = objY + objHeight + ballRadius;

    if (ballCenterX >= expandedLeft && ballCenterX <= expandedRight &&
        ballCenterY >= expandedTop && ballCenterY <= expandedBottom) {

        result.collision = true;

        //расстояние до стороны
        float leftDist = ballCenterX - objX;
        float rightDist = objX + objWidth - ballCenterX;
        float topDist = ballCenterY - objY;
        float bottomDist = objY + objHeight - ballCenterY;

        float minDist = min(min(leftDist, rightDist), min(topDist, bottomDist));
        float normalX = 0.0f, normalY = 0.0f;

        if (minDist == leftDist) {
            normalX = -1.0f; // Левая сторона
        }
        else if (minDist == rightDist) {
            normalX = 1.0f;  // Правая сторона
        }
        else if (minDist == topDist) {
            normalY = -1.0f; // Верхняя сторона
        }
        else {
            normalY = 1.0f;  // Нижняя сторона
        }

        // углы
        float corners[4][2] = {
            {objX, objY}, {objX + objWidth, objY},
            {objX, objY + objHeight}, {objX + objWidth, objY + objHeight}
        };

        for (int i = 0; i < 4; i++) {
            float cornerX = corners[i][0];
            float cornerY = corners[i][1];
            float cornerDx = ballCenterX - cornerX;
            float cornerDy = ballCenterY - cornerY;
            float cornerDist = sqrtf(cornerDx * cornerDx + cornerDy * cornerDy);

            if (cornerDist < minDist + 5.0f && cornerDist < ballRadius) {
                if (cornerX == objX && cornerY == objY) {
                    normalX = -0.707f; normalY = -0.707f; // Левый верхний
                }
                else if (cornerX == objX + objWidth && cornerY == objY) {
                    normalX = 0.707f; normalY = -0.707f;  // Правый верхний
                }
                else if (cornerX == objX && cornerY == objY + objHeight) {
                    normalX = -0.707f; normalY = 0.707f;  // Левый нижний
                }
                else {
                    normalX = 0.707f; normalY = 0.707f;   // Правый нижний
                }
                minDist = cornerDist;
                break;
            }
        }
        float VectorX = cosf(currentBallAngle);
        float VectorY = sinf(currentBallAngle);
        float dot = VectorX * normalX + VectorY * normalY;

        if (dot < 0) {
            float reflectX = VectorX - 2.0f * dot * normalX;
            float reflectY = VectorY - 2.0f * dot * normalY;
            result.newAngle = atan2f(reflectY, reflectX);
        }
        else {
            result.newAngle = currentBallAngle;
        }

        float overlap = ballRadius - minDist;
        result.newPosX = static_cast<int>(ballCheckX + normalX * overlap);
        result.newPosY = static_cast<int>(ballCheckY + normalY * overlap);
    }

    return result;
}

void GamePlay(HWND hWnd) {
    RECT rect;
    GetClientRect(hWnd, &rect);
    int MainWidht = rect.right - rect.left;
    int width = static_cast<int>((MainWidht / 10.0f) * (10 - ManageWindowWidhtProcent));
    int height = rect.bottom - rect.top;

    // Движение платформы
    int platformX = PlayerPosX;
    int newPlatformX = platformX + static_cast<int>(static_cast<float>(Speed) * static_cast<float>(PlayerVector));
    int platformY = height - 100;

    if (MouseCanTouchBoll == true) {
        if (MouseX > BollPosX - 15 && MouseX < BollPosX + ManageSmallCircleRadius + 15 && MouseY > BollPosY - 15 && MouseY < BollPosY + ManageSmallCircleRadius + 15) {
            MouseTarget = 1;
        }

        if (MouseX > center2X - 15 && MouseX < center2X + ManageSmallCircleRadius + 15 && MouseY > center2Y - 15 && MouseY < center2Y + ManageSmallCircleRadius + 15) {
            MouseTarget = 2;
        }
    }

    if (MouseTarget == 1) {
        BollPosY = MouseY - BallSize / 2;
        BollPosX = MouseX - BallSize / 2;
    }

    if (MouseTarget == 2) {
        BollAngle = atan2f(static_cast<float>(MouseY - center1Y), static_cast<float>(MouseX - center1X));
    }

    if (newPlatformX >= 0 && newPlatformX <= width - PlatformWidth) {
        PlayerPosX += static_cast<int>(static_cast<float>(Speed) * static_cast<float>(PlayerVector));
    }

    // Движение мяча
    int NewBollPosX = BollPosX + static_cast<int>((BollSpeed * cosf(BollAngle)) / 2.0f);
    int NewBollPosY = BollPosY + static_cast<int>((BollSpeed * sinf(BollAngle)) / 2.0f);

    int DistanceX = NewBollPosX - BollPosX;
    int DistanceY = NewBollPosY - BollPosY;

    int steps = max(abs(DistanceX), abs(DistanceY));
    bool collisionOccurred = false;

    // Проверка столкновений 
    for (int i = 0; i <= steps; i++) {

        double t = static_cast<double>(i) / static_cast<double>(steps);
        int CheckX = BollPosX + static_cast<int>(round(static_cast<double>(DistanceX) * t));
        int CheckY = BollPosY + static_cast<int>(round(static_cast<double>(DistanceY) * t));

        float ballRadius = static_cast<float>(BallSize) / 2.0f;
        float ballCenterX = static_cast<float>(CheckX) + ballRadius;
        float ballCenterY = static_cast<float>(CheckY) + ballRadius;

        if (ballCenterX - ballRadius <= 0.0f) {
            BollPosX = static_cast<int>(ballRadius);
            BollAngle = PI - BollAngle;
            collisionOccurred = true;
            MouseTarget = 0;
            MouseCanTouchBoll = false;
            break;
        }
        else if (ballCenterX + ballRadius >= static_cast<float>(width)) {
            BollPosX = width - BallSize - 1;
            BollAngle = PI - BollAngle;
            collisionOccurred = true;
            MouseTarget = 0;
            MouseCanTouchBoll = false;
            break;
        }
        else if (ballCenterY - ballRadius <= 0.0f) {
            BollPosY = static_cast<int>(ballRadius);
            BollAngle = -BollAngle;
            collisionOccurred = true;
            MouseTarget = 0;
            MouseCanTouchBoll = false;
            break;
        }
        if (CheckY > height) {
            BollPosX = width / 2;
            BollPosY = height / 2;
            BollAngle = 45.0f * (PI / 180.0f);
            StopBattleAnimation(hWnd);
            collisionOccurred = true;
            break;
        }

        // Проверка столкновения с платформой
        CollisionResult platformCollision = CheckObjectCollision(
            static_cast<float>(CheckX), static_cast<float>(CheckY), ballRadius, BollAngle,
            static_cast<float>(platformX), static_cast<float>(platformY),
            static_cast<float>(PlatformWidth), static_cast<float>(PlatformHeight)
        );

        if (platformCollision.collision) {
            BollAngle = platformCollision.newAngle;

            if (platformCollision.newPosX != 0 || platformCollision.newPosY != 0) {
                BollPosX = platformCollision.newPosX;
                BollPosY = platformCollision.newPosY;
            }

            MouseTarget = 0;
            MouseCanTouchBoll = false;
            collisionOccurred = true;
            break;
        }

        // Проверка кирпичей
        bool brickCollisionFound = false;
        for (int y = 0; y < BrickLines; y++) {
            for (int x = 0; x < BrickStolbs; x++) {
                if (brick[x][y].live) {
                    CollisionResult brickCollision = CheckObjectCollision(
                        static_cast<float>(CheckX), static_cast<float>(CheckY), ballRadius,
                        BollAngle,
                        static_cast<float>(brick[x][y].posx),
                        static_cast<float>(brick[x][y].posy),
                        static_cast<float>(brick[x][y].W),
                        static_cast<float>(brick[x][y].H)
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
                MouseTarget = 0;
                MouseCanTouchBoll = false;
                break;
            }
        }

        if (brickCollisionFound) {
            MouseTarget = 0;
            MouseCanTouchBoll = false;
            break;
        }
    }

    if (!collisionOccurred) {
        BollPosX = NewBollPosX;
        BollPosY = NewBollPosY;
    }

    //проверка колизии пути
    for (int i = 0; i < MaxRedLines; i++) {
        Line[i].FirstX = 0;
        Line[i].FirstY = 0;
        Line[i].LastX = 0;
        Line[i].LastY = 0;
        Line[i].NewAngle = 0.0f;
    }

    // Инициализация
    float predictAngle = BollAngle;
    int predictPosX = BollPosX;
    int predictPosY = BollPosY;
    int lineIndex = 0;
    int maxPredictions = MaxRedLines;

    if (lineIndex < MaxRedLines) {
        Line[lineIndex].FirstX = predictPosX + BallSize / 2;
        Line[lineIndex].FirstY = predictPosY + BallSize / 2;
    }

    for (int prediction = 0; prediction < maxPredictions && lineIndex < MaxRedLines; prediction++) {
        int nextPredictPosX = predictPosX + static_cast<int>((BollSpeed * 39.0f * cosf(predictAngle)) / 2.0f);
        int nextPredictPosY = predictPosY + static_cast<int>((BollSpeed * 39.0f * sinf(predictAngle)) / 2.0f);

        bool collisionFound = false;
        int collisionX = 0, collisionY = 0;
        float newPredictAngle = predictAngle;

        int predictDistanceX = nextPredictPosX - predictPosX;
        int predictDistanceY = nextPredictPosY - predictPosY;
        int predictSteps = max(abs(predictDistanceX), abs(predictDistanceY));

        if (predictSteps == 0) {
            Line[lineIndex].LastX = Line[lineIndex].FirstX;
            Line[lineIndex].LastY = Line[lineIndex].FirstY;
            lineIndex++;
            continue;
        }

        for (int i = 0; i <= predictSteps && !collisionFound; i++) {
            double t = static_cast<double>(i) / static_cast<double>(predictSteps);
            int checkX = predictPosX + static_cast<int>(round(static_cast<double>(predictDistanceX) * t));
            int checkY = predictPosY + static_cast<int>(round(static_cast<double>(predictDistanceY) * t));

            float ballRadius = static_cast<float>(BallSize) / 2.0f;

            float ballCenterX = static_cast<float>(checkX) + ballRadius;
            float ballCenterY = static_cast<float>(checkY) + ballRadius;

            if (ballCenterX - ballRadius <= 0.0f) {
                // Левая стена
                collisionFound = true;
                newPredictAngle = static_cast<float>(M_PI) - predictAngle;
                Line[lineIndex].NewAngle = newPredictAngle;
                collisionX = static_cast<int>(ballRadius);
                collisionY = static_cast<int>(ballCenterY);
                predictPosX = 1;
                predictPosY = checkY;
                break;
            }
            else if (ballCenterX + ballRadius >= static_cast<float>(width)) {
                // Правая стена
                collisionFound = true;
                newPredictAngle = static_cast<float>(M_PI) - predictAngle;
                Line[lineIndex].NewAngle = newPredictAngle;
                collisionX = width - static_cast<int>(ballRadius);
                collisionY = static_cast<int>(ballCenterY);
                predictPosX = width - BallSize - 1;
                predictPosY = checkY;
                break;
            }
            else if (ballCenterY - ballRadius <= 0.0f) {
                // Верхняя стена
                collisionFound = true;
                newPredictAngle = -predictAngle;
                Line[lineIndex].NewAngle = newPredictAngle;
                collisionX = static_cast<int>(ballCenterX);
                collisionY = static_cast<int>(ballRadius);
                predictPosX = checkX;
                predictPosY = 1;
                break;
            }
            else if (ballCenterY + ballRadius >= static_cast<float>(height)) {
                // Нижняя стена
                collisionFound = true;
                collisionX = static_cast<int>(ballCenterX);
                collisionY = height - static_cast<int>(ballRadius);
                break;
            }

            // Проверка столкновения с платформой
            CollisionResult platformCollision = CheckObjectCollision(
                static_cast<float>(checkX), static_cast<float>(checkY), ballRadius, predictAngle,
                static_cast<float>(PlayerPosX), static_cast<float>(platformY),
                static_cast<float>(PlatformWidth), static_cast<float>(PlatformHeight)
            );

            if (platformCollision.collision) {
                collisionFound = true;
                collisionX = checkX + static_cast<int>(ballRadius);
                collisionY = checkY + static_cast<int>(ballRadius);
                Line[lineIndex].NewAngle = platformCollision.newAngle;
                newPredictAngle = platformCollision.newAngle;
                predictPosX = platformCollision.newPosX;
                predictPosY = platformCollision.newPosY;
                break;
            }

            // Проверка столкновения с кирпичами
            for (int y = 0; y < BrickLines && !collisionFound; y++) {
                for (int x = 0; x < BrickStolbs && !collisionFound; x++) {
                    if (brick[x][y].live) {
                        CollisionResult brickCollision = CheckObjectCollision(
                            static_cast<float>(checkX), static_cast<float>(checkY), ballRadius, predictAngle,
                            static_cast<float>(brick[x][y].posx), static_cast<float>(brick[x][y].posy),
                            static_cast<float>(brick[x][y].W), static_cast<float>(brick[x][y].H)
                        );

                        if (brickCollision.collision) {
                            collisionFound = true;
                            collisionX = checkX + static_cast<int>(ballRadius);
                            collisionY = checkY + static_cast<int>(ballRadius);
                            Line[lineIndex].NewAngle = brickCollision.newAngle;
                            newPredictAngle = brickCollision.newAngle;
                            predictPosX = brickCollision.newPosX;
                            predictPosY = brickCollision.newPosY;
                            break;
                        }
                    }
                }
                if (collisionFound) break;
            }
        }

        if (collisionFound) {
            Line[lineIndex].LastX = collisionX;
            Line[lineIndex].LastY = collisionY;
            predictAngle = newPredictAngle;
        }
        else {
            Line[lineIndex].LastX = nextPredictPosX + BallSize / 2;
            Line[lineIndex].LastY = nextPredictPosY + BallSize / 2;
            predictPosX = nextPredictPosX;
            predictPosY = nextPredictPosY;
        }

        if (lineIndex + 1 < MaxRedLines) {
            Line[lineIndex + 1].FirstX = Line[lineIndex].LastX;
            Line[lineIndex + 1].FirstY = Line[lineIndex].LastY;
        }

        lineIndex++;

        if (predictPosY > height) {
            break;
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

    case WM_LBUTTONDOWN:
        if (MouseWasUpped == true)
        {
            MouseWasUpped = false;
            MouseCanTouchBoll = true;
        }
        break;

    case WM_LBUTTONUP:
        MouseWasUpped = true;
        MouseCanTouchBoll = false;
        MouseTarget = 0;
        break;

    case WM_MOUSEMOVE:
        MouseX = LOWORD(lParam);
        MouseY = HIWORD(lParam);
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

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
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