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

int windowWidth = 200;
int extendedWindowWidth = 500;
int windowHeight = 200;
int buttonSpacing = 10;
int buttonSize;
int panelOffsetX = 10;
int panelWidth;
int panelHeight;
Panel currentPanel;
int dayButtonClicked = -1;
int panelOffsetY;

Texture2D baseButtons;
Texture2D completedButtons;
Texture2D currentButtons;

char *weekdays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int GetNewWindowSize()
{
    return GetMonitorHeight(GetCurrentMonitor()) * 0.8;
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

void CreatePanel(Shader panelShader, int xPositionLoc, int yPositionLoc, int panelSizeLoc)
{
    int buttonCenter = panelOffsetY + dayButtonClicked * buttonSize + dayButtonClicked * buttonSpacing + buttonSize / 2;
    int panelY = buttonCenter - panelHeight / 2;
    if (buttonCenter + panelHeight / 2 > GetScreenHeight())
    {
        panelY -= (buttonCenter + panelHeight / 2) - GetScreenHeight();
    }
    else if (buttonCenter - (panelHeight / 2 + buttonSize) < 0)
    {
        panelY = buttonSize;
    }
    currentPanel = (Panel){buttonSize + panelOffsetX, panelY};
    float xPosition = (float)currentPanel.x;
    float yPosition = (float)(GetScreenHeight() - currentPanel.y - panelHeight);
    SetShaderValue(panelShader, xPositionLoc, &xPosition, SHADER_UNIFORM_FLOAT);
    SetShaderValue(panelShader, yPositionLoc, &yPosition, SHADER_UNIFORM_FLOAT);
    SetShaderValue(panelShader, panelSizeLoc, &(Vector2){panelWidth, panelHeight}, SHADER_UNIFORM_VEC2);
}

void RefreshWindow()
{
    time_t t = time(NULL);
    struct tm now = *localtime(&t);

    buttonSize = (windowHeight / 31) - buttonSpacing;
    windowWidth = buttonSize;
    panelWidth = extendedWindowWidth - buttonSize - panelOffsetX;
    panelHeight = 24 * buttonSize;
    panelOffsetY = (windowHeight - (((buttonSpacing + buttonSize) * daysInMonth[now.tm_mon]) - buttonSpacing)) / 2;

    SetWindowSize(windowWidth, windowHeight);

    Image baseButtonsImage = LoadImage("resources/buttons_texture.png");
    ImageResize(&baseButtonsImage, buttonSize * 7, buttonSize * 5);
    baseButtons = LoadTextureFromImage(baseButtonsImage);

    Image completedButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
    ImageColorTint(&completedButtonsImage, (Color){255, 92, 64, 255});
    completedButtons = LoadTextureFromImage(completedButtonsImage);

    Image currentButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
    ImageColorTint(&currentButtonsImage, YELLOW);
    currentButtons = LoadTextureFromImage(currentButtonsImage);

    UnloadImage(baseButtonsImage);
    UnloadImage(completedButtonsImage);
    UnloadImage(currentButtonsImage);
}

int main(void)
{
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    daysInMonth[1] = (now.tm_year + 1900) % 4 == 0 ? 29 : 28;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED);
    InitWindow(windowWidth, windowHeight, "test");
    SetTargetFPS(60);
    //setWindowOnBottom(GetWindowHandle());
    SetWindowPosition(LoadStorageValue(X_WINDOW_POSITION), LoadStorageValue(Y_WINDOW_POSITION));

    currentPanel = (Panel){0, 0};
    windowHeight = GetNewWindowSize();
    RefreshWindow();

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

    int mouseOffsetX = 0;
    int mouseOffsetY = 0;
    Vector2 previousWindowPosition = GetWindowPosition();

    int panelSizeLoc = GetShaderLocation(panelShader, "uPanelSize");
    int xPositionLoc = GetShaderLocation(panelShader, "xPosition");
    int yPositionLoc = GetShaderLocation(panelShader, "yPosition");

    SetShaderValue(panelShader, panelSizeLoc, &(Vector2){panelWidth, panelHeight}, SHADER_UNIFORM_VEC2);

    Font headerFont = LoadFont("resources/Glacial Indifference.otf");
    Font hourFont = LoadFont("resources/CascadiaCode.ttf");
    SetTextureFilter(headerFont.texture, TEXTURE_FILTER_BILINEAR);
    SetTextureFilter(hourFont.texture, TEXTURE_FILTER_BILINEAR);

    while (!WindowShouldClose())
    {

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            // setWindowOnBottom(GetWindowHandle());
            int mouseX = GetMouseX();
            int mouseY = GetMouseY();
            int buttonCenter = panelOffsetY + dayButtonClicked * buttonSize + dayButtonClicked * buttonSpacing + buttonSize / 2;
            if (dayButtonClicked != -1 && mouseX > buttonSize && (mouseY < currentPanel.y || mouseY > currentPanel.y + panelHeight))
            {
                dayButtonClicked = -1;
                SetWindowSize(windowWidth, windowHeight);
            }
            previousWindowPosition = GetWindowPosition();
            mouseOffsetX = GetMouseX();
            mouseOffsetY = GetMouseY();
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
        {
            Vector2 position = GetWindowPosition();
            SetWindowPosition((int)position.x + (GetMouseX() - mouseOffsetX), (int)position.y + (GetMouseY() - mouseOffsetY));
            int oldHeight = windowHeight;
            windowHeight = GetNewWindowSize();
            if (windowHeight != oldHeight)
            {
                RefreshWindow();
                CreatePanel(panelShader, xPositionLoc, yPositionLoc, panelSizeLoc);
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            Vector2 currentWindowPosition = GetWindowPosition();
            if (currentWindowPosition.x != previousWindowPosition.x || currentWindowPosition.y != previousWindowPosition.y)
            {
                previousWindowPosition = currentWindowPosition;
                SaveStorageValue(X_WINDOW_POSITION, previousWindowPosition.x);
                SaveStorageValue(Y_WINDOW_POSITION, previousWindowPosition.y);
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

            if (GuiTextureButtonEx((Rectangle){0, panelOffsetY + i * buttonSize + i * buttonSpacing, buttonSize, buttonSize}, "", *buttonTexture, (Rectangle){(i % 7) * buttonSize, (i / 7) * buttonSize, buttonSize, buttonSize}))
            {
                if (dayButtonClicked == i)
                {
                    dayButtonClicked = -1;
                    SetWindowSize(windowWidth, windowHeight);
                }
                else
                {
                    dayButtonClicked = i;
                    CreatePanel(panelShader, xPositionLoc, yPositionLoc, panelSizeLoc);
                }
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
            DrawTextEx(headerFont, buffer, (Vector2){currentPanel.x + (panelWidth / 2 - MeasureTextEx(headerFont, buffer, 40, 1).x / 2), currentPanel.y - buttonSize / 2 - 20}, 40, 1, WHITE);

            for (int i = panelHeight / 24; i < panelHeight; i += panelHeight / 24)
            {
                DrawLine(currentPanel.x, currentPanel.y + i, currentPanel.x + panelWidth, currentPanel.y + i, DARKGRAY);
            }
            for (int i = 1; i < 24; ++i)
            {
                char number[6];
                snprintf(number, 6, i < 10 ? "0%d:00" : "%d:00", i);
                DrawTextEx(hourFont, number, (Vector2){currentPanel.x + 5, currentPanel.y + i * buttonSize - 18}, 18, 1, DARKGRAY);
            }
        }

        EndDrawing();
    }

    UnloadTexture(baseButtons);
    UnloadTexture(completedButtons);
    UnloadTexture(currentButtons);
    UnloadShader(panelShader);
    UnloadFont(headerFont);
    UnloadFont(hourFont);
    CloseWindow();

    return 0;
}
