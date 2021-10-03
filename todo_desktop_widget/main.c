#define RAYGUI_IMPLEMENTATION

#include "raygui.h"
#include "stdio.h"
#include "time.h"
#include "tools.h"

#define X_WINDOW_POSITION 31
#define Y_WINDOW_POSITION 32

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
    SetWindowPosition(LoadStorageValue(X_WINDOW_POSITION), LoadStorageValue(Y_WINDOW_POSITION));
    windowHeight = AdjustWindowSize(windowWidth);

    int buttonSpacing = 10;
    int buttonSize = (windowHeight / 31) - buttonSpacing;

    Image baseButtonsImage = LoadImage("resources/buttons_texture.png");
    ImageResize(&baseButtonsImage, buttonSize * 7, buttonSize * 5);
    Texture2D baseButtons = LoadTextureFromImage(baseButtonsImage);

    Image completedButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
    ImageColorTint(&completedButtonsImage, (Color){255, 92, 64, 255});
    Texture2D completedButtons = LoadTextureFromImage(completedButtonsImage);

    Image currentButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
    ImageColorTint(&currentButtonsImage, YELLOW);
    Texture2D currentButtons = LoadTextureFromImage(currentButtonsImage);

    UnloadImage(baseButtonsImage);
    UnloadImage(completedButtonsImage);
    UnloadImage(currentButtonsImage);

    bool daysCompleted[31];
    if(now.tm_mday == 1)
    {
        for (int i = 0; i < 31; ++i)
        {
            daysCompleted[i] = false;
            SaveStorageValue(i, 0);
        }
    }
    else
    {
        for (int i = 0; i < 31; ++i)
        {
            daysCompleted[i] = LoadStorageValue(i) == 1 ? true : false;
        }
    }
    
    int deltaX = 0, deltaY = 0;
    Vector2 windowPosition = GetWindowPosition();
    while (!WindowShouldClose())
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // setWindowOnBottom(GetWindowHandle());
            windowPosition = GetWindowPosition();
            deltaX = GetMouseX();
            deltaY = GetMouseY();
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            Vector2 position = GetWindowPosition();
            SetWindowPosition((int)position.x + (GetMouseX() - deltaX), (int)position.y + (GetMouseY() - deltaY));
            int oldHeight = windowHeight;
            windowHeight = AdjustWindowSize(windowWidth);
            if (windowHeight != oldHeight)
            {
                // ReloadTextures(&buttonsImage, &baseButtons, &completedButtons, &currentButtons, buttonSize);
                UnloadTexture(baseButtons);
                UnloadTexture(completedButtons);
                UnloadTexture(currentButtons);

                buttonSize = (windowHeight / 31) - buttonSpacing;
                baseButtonsImage = LoadImage("resources/buttons_texture.png");
                ImageResize(&baseButtonsImage, buttonSize * 7, buttonSize * 5);
                baseButtons = LoadTextureFromImage(baseButtonsImage);

                completedButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
                ImageColorTint(&completedButtonsImage, (Color){255, 92, 64, 255});
                completedButtons = LoadTextureFromImage(completedButtonsImage);

                currentButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
                ImageColorTint(&currentButtonsImage, YELLOW);
                currentButtons = LoadTextureFromImage(currentButtonsImage);

                UnloadImage(baseButtonsImage);
                UnloadImage(completedButtonsImage);
                UnloadImage(currentButtonsImage);
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            Vector2 currentWindowPosition = GetWindowPosition();
            if(currentWindowPosition.x != windowPosition.x || currentWindowPosition.y != windowPosition.y)
            {
                windowPosition = currentWindowPosition;
                SaveStorageValue(X_WINDOW_POSITION, windowPosition.x);
                SaveStorageValue(Y_WINDOW_POSITION, windowPosition.y);
            }
        }
        ClearBackground(BLANK);
        // ClearBackground((Color){0, 0, 0, 150});

        BeginDrawing();

        buttonSize = (windowHeight / 31) - buttonSpacing;
        int offsetY = (windowHeight - (((buttonSpacing + buttonSize) * daysInMonth[now.tm_mon]) - buttonSpacing)) / 2;
        for (int i = 0; i < daysInMonth[now.tm_mon]; ++i)
        {
            Texture2D *buttonTexture;
            
            if(daysCompleted[i])
            {
                buttonTexture = &completedButtons;
            }
            else if (i + 1 == now.tm_mday)
            {
                buttonTexture = &currentButtons;
            }
            else
            {
                buttonTexture = &baseButtons;
            }
            if (GuiTextureButtonEx((Rectangle){0, offsetY + i * buttonSize + i * buttonSpacing, buttonSize, buttonSize}, "", *buttonTexture, (Rectangle){(i % 7) * buttonSize, (i / 7) * buttonSize, buttonSize, buttonSize}))
            {
                // printf("clicked day: %s", getWeekday(weekdays, i + 1));
                daysCompleted[i] = true;
                SaveStorageValue(i, 1);
            }
        }

        EndDrawing();
    }

    UnloadTexture(baseButtons);
    UnloadTexture(completedButtons);
    UnloadTexture(currentButtons);
    CloseWindow();

    return 0;
}
