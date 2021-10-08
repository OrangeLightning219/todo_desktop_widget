#define RAYGUI_IMPLEMENTATION

#include "raygui.h"
#include "stdio.h"
#include "time.h"
#include "tools.h"

#define X_WINDOW_POSITION 31
#define Y_WINDOW_POSITION 32

typedef struct Panel
{
    int x;
    int y;
} Panel;

int AdjustWindowSize()
{
    int windowHeight = GetMonitorHeight(GetCurrentMonitor()) * 0.8;
    return windowHeight;
}

char *GetWeekday(char *weekdays[7], int day)
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
    int windowWidth = 200;
    int extendedWindowWidth = 500;
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
    windowHeight = AdjustWindowSize();

    int buttonSpacing = 10;
    int buttonSize = (windowHeight / 31) - buttonSpacing;
    int panelOffset = 10;
    int panelWidth = extendedWindowWidth - buttonSize - panelOffset;
    int panelHeight = 24 * buttonSize;
    Panel currentPanel;

    windowWidth = buttonSize;
    SetWindowSize(windowWidth, windowHeight);

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

    Shader panelShader = LoadShader(0, "resources/panel_shader.fs");

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
    int dayButtonClicked = -1;

    int panelSizeLoc = GetShaderLocation(panelShader, "uPanelSize");
    int xPositionLoc = GetShaderLocation(panelShader, "xPosition");
    int yPositionLoc = GetShaderLocation(panelShader, "yPosition");

    SetShaderValue(panelShader, panelSizeLoc, &(Vector2){panelWidth, panelHeight}, SHADER_UNIFORM_VEC2);

    while (!WindowShouldClose())
    {
        buttonSize = (windowHeight / 31) - buttonSpacing;
        int offsetY = (windowHeight - (((buttonSpacing + buttonSize) * daysInMonth[now.tm_mon]) - buttonSpacing)) / 2;

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // setWindowOnBottom(GetWindowHandle());
            int mouseX = GetMouseX();
            int mouseY = GetMouseY();
            int buttonCenter = offsetY + dayButtonClicked * buttonSize + dayButtonClicked * buttonSpacing + buttonSize / 2;
            if (dayButtonClicked != -1 && mouseX > buttonSize && (mouseY < currentPanel.y || mouseY > currentPanel.y + panelHeight))
            {
                dayButtonClicked = -1;
                SetWindowSize(windowWidth, windowHeight);
                // currentPanel.active = false;
            }
            windowPosition = GetWindowPosition();
            deltaX = GetMouseX();
            deltaY = GetMouseY();
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            Vector2 position = GetWindowPosition();
            SetWindowPosition((int)position.x + (GetMouseX() - deltaX), (int)position.y + (GetMouseY() - deltaY));
            int oldHeight = windowHeight;
            windowHeight = AdjustWindowSize();
            if (windowHeight != oldHeight)
            {
                UnloadTexture(baseButtons);
                UnloadTexture(completedButtons);
                UnloadTexture(currentButtons);

                buttonSize = (windowHeight / 31) - buttonSpacing;
                windowWidth = buttonSize;
                SetWindowSize(windowWidth, windowHeight);
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
                if (dayButtonClicked == i)
                {
                    dayButtonClicked = -1;
                    SetWindowSize(windowWidth, windowHeight);
                }
                else
                {
                    dayButtonClicked = i;
                    int buttonCenter = offsetY + dayButtonClicked * buttonSize + dayButtonClicked * buttonSpacing + buttonSize / 2;
                    int panelY = buttonCenter - panelHeight / 2;
                    if (buttonCenter + panelHeight / 2 > GetScreenHeight())
                    {
                        panelY -= (buttonCenter + panelHeight / 2) - GetScreenHeight();
                    }
                    else if (buttonCenter - (panelHeight / 2 + buttonSize) < 0)
                    {
                        panelY = buttonSize;
                    }
                    currentPanel = (Panel){buttonSize + panelOffset, panelY};
                    float xPosition = (float)currentPanel.x;
                    float yPosition = (float)(GetScreenHeight() - currentPanel.y - panelHeight);
                    SetShaderValue(panelShader, xPositionLoc, &xPosition, SHADER_UNIFORM_FLOAT);
                    SetShaderValue(panelShader, yPositionLoc, &yPosition, SHADER_UNIFORM_FLOAT);
                }
                // daysCompleted[i] = true;
                // SaveStorageValue(i, 1);
            }
        }

        if (dayButtonClicked != -1)
        {
            if (GetScreenWidth() != extendedWindowWidth)
            {
                SetWindowSize(extendedWindowWidth, windowHeight);
            }

            BeginShaderMode(panelShader);
            DrawRectangle(currentPanel.x, currentPanel.y, panelWidth, panelHeight, BLANK);
            EndShaderMode();

            char *weekday = GetWeekday(weekdays, dayButtonClicked + 1);
            char buffer[14];
            snprintf(buffer, 14, "%s, %d", weekday, dayButtonClicked + 1);
            DrawText(buffer, currentPanel.x + (panelWidth / 2 - MeasureText(buffer, 30) / 2), currentPanel.y - buttonSize / 2 - 15, 30, WHITE);

            for (int i = panelHeight / 24; i < panelHeight; i += panelHeight / 24)
            {
                DrawLine(currentPanel.x, currentPanel.y + i, currentPanel.x + panelWidth, currentPanel.y + i, DARKGRAY);
            }

            for (int i = 1; i < 24; ++i)
            {
                char number[6];
                snprintf(number, 6, i < 10 ? "0%d:00" : "%d:00", i);
                DrawText(number, currentPanel.x + 5, currentPanel.y + i * buttonSize - 20, 20, DARKGRAY);
            }
        }

        EndDrawing();
    }

    UnloadTexture(baseButtons);
    UnloadTexture(completedButtons);
    UnloadTexture(currentButtons);
    UnloadShader(panelShader);
    CloseWindow();

    return 0;
}
