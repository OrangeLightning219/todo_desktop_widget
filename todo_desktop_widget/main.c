//#include "raylib.h"
#define RAYGUI_IMPLEMENTATION

// #include "raygui.h"
// #include "math.h"
#include "raygui.h"
#include "stdio.h"
#include "time.h"
#include "tools.h"

int AdjustWindowSize(int windowWidth)
{
    int windowHeight = GetMonitorHeight(GetCurrentMonitor()) * 0.8;
    SetWindowSize(windowWidth, windowHeight);
    return windowHeight;
}

char *getWeekday(char *weekdays[7], int day)
{
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    int month = now.tm_mon + 1;
    int year = now.tm_year + 1900;
    int weekday = (day += month < 3 ? year-- : year - 2, 23 * month / 9 + day + 4 + year / 4 - year / 100 + year / 400) % 7;
    return weekdays[weekday];
}

int main(void)
{
    const int windowWidth = 200;
    int windowHeight = 200;

    char *weekdays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    daysInMonth[1] = (now.tm_year + 1900) % 4 == 0 ? 29 : 28;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED);
    InitWindow(windowWidth, windowHeight, "test");
    SetTargetFPS(60);
    //setWindowOnBottom(GetWindowHandle());
    windowHeight = AdjustWindowSize(windowWidth);

    int deltaX = 0, deltaY = 0;
    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // setWindowOnBottom(GetWindowHandle());
            deltaX = GetMouseX();
            deltaY = GetMouseY();
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            Vector2 position = GetWindowPosition();
            SetWindowPosition((int)position.x + (GetMouseX() - deltaX), (int)position.y + (GetMouseY() - deltaY));
            windowHeight = AdjustWindowSize(windowWidth);
        }

        ClearBackground((Color){0, 0, 0, 150});

        BeginDrawing();

        int buttonSpacing = 10;
        int buttonSize = (windowHeight / 31) - buttonSpacing;
        int offsetY = (windowHeight - (((buttonSpacing + buttonSize) * daysInMonth[now.tm_mon]) - buttonSpacing)) / 2;
        for (int i = 1; i <= daysInMonth[now.tm_mon]; ++i)
        {
            char number[3];
            if (GuiButton((Rectangle){0, offsetY + (i - 1) * buttonSize + (i - 1) * buttonSpacing, buttonSize, buttonSize}, itoa(i, number, 10)))
            {
                printf("clicked day: %s", getWeekday(weekdays, i));

                DrawRectangle(0, 0, 50, 50, WHITE);
            }
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
