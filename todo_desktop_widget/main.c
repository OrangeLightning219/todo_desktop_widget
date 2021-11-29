#define RAYGUI_IMPLEMENTATION

#include "raygui.h"
// #include "raylib.h"
#include "stdio.h"
// #include "stdlib.h"
#include "time.h"
#include "tools.h"

#define X_WINDOW_POSITION 0
#define Y_WINDOW_POSITION 1
#define MONTH_CLEARED 2

typedef struct Task
{
    char *name;
    int hourFrom, hourTo;
    int minuteFrom, minuteTo;
    bool completed;
    Color color;
    Color checkColor;
} Task;

typedef struct Panel
{
    int x;
    int y;
    Task tasks[];
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
int taskOffsetX;

Texture2D baseButtons;
Texture2D completedButtons;
Texture2D currentButtons;
Texture2D checkTexture;

char *weekdays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int weekdaysInMonth[7][5] = {0};
bool daysCompleted[31] = {0};

Task **tasksMap;
int tasksCounts[31] = {0};

int GetNewWindowSize()
{
    return GetMonitorHeight(GetCurrentMonitor()) * 0.8;
}

char *GetWeekday(int day, struct tm now)
{
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
    float radius = (float)buttonSize;
    SetShaderValue(panelShader, xPositionLoc, &xPosition, SHADER_UNIFORM_FLOAT);
    SetShaderValue(panelShader, yPositionLoc, &yPosition, SHADER_UNIFORM_FLOAT);
    SetShaderValue(panelShader, panelSizeLoc, &(Vector2){panelWidth, panelHeight}, SHADER_UNIFORM_VEC2);
    SetShaderValue(panelShader, GetShaderLocation(panelShader, "cornerRadius"), &radius, SHADER_UNIFORM_FLOAT);
}

void RefreshWindow()
{
    time_t timestamp = time(NULL);
    struct tm now = *localtime(&timestamp);

    buttonSpacing = windowHeight * 0.005;
    buttonSize = (windowHeight / 31) - buttonSpacing;
    windowWidth = buttonSize;
    extendedWindowWidth = 9 * buttonSize;
    panelWidth = extendedWindowWidth - buttonSize - panelOffsetX;
    panelHeight = 24 * buttonSize;
    panelOffsetY = (windowHeight - (((buttonSpacing + buttonSize) * daysInMonth[now.tm_mon]) - buttonSpacing)) / 2;
    SetWindowSize(windowWidth, windowHeight);

    Image baseButtonsImage = LoadImage("resources/buttons_texture.png");
    ImageResize(&baseButtonsImage, buttonSize * 7, buttonSize * 5);
    baseButtons = LoadTextureFromImage(baseButtonsImage);

    Image completedButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
    ImageColorTint(&completedButtonsImage, (Color){116, 34, 245, 255});
    completedButtons = LoadTextureFromImage(completedButtonsImage);

    Image currentButtonsImage = ImageFromImage(baseButtonsImage, (Rectangle){0, 0, baseButtonsImage.width, baseButtonsImage.height});
    ImageColorTint(&currentButtonsImage, YELLOW);
    currentButtons = LoadTextureFromImage(currentButtonsImage);

    checkTexture = LoadTexture("resources/check.png");
    SetTextureFilter(checkTexture, TEXTURE_FILTER_BILINEAR);

    UnloadImage(baseButtonsImage);
    UnloadImage(completedButtonsImage);
    UnloadImage(currentButtonsImage);
}

unsigned int hex2int(char *hex)
{
    unsigned int val = 0;
    while (*hex)
    {
        // get current character then increment
        char byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9')
            byte = byte - '0';
        else if (byte >= 'a' && byte <= 'f')
            byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <= 'F')
            byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

int ReadLine(char *fileBuffer, char *lineBuffer, int start)
{
    int length = 0;
    while (fileBuffer[start + length] != '\n')
    {
        ++length;
    }

    for (int i = start; i < start + length; ++i)
    {
        lineBuffer[i - start] = fileBuffer[i];
    }

    return length;
}

void LoadTasks(bool loadTemplate)
{
    time_t timestamp = time(NULL);
    struct tm now = *localtime(&timestamp);
    if (loadTemplate)
    {
        tasksMap = (Task **)malloc(daysInMonth[now.tm_mon] * sizeof(Task *));
    }

    char *fileBuffer;
    char *lineBuffer = (char *)calloc(64, sizeof(char));
    lineBuffer[63] = '\0';
    if (loadTemplate)
    {
        fileBuffer = LoadFileText("resources/tasks.todo");
    }
    else
    {
        fileBuffer = LoadFileText("resources/completed.todo");
    }

    if (fileBuffer == NULL && !loadTemplate)
    {
        return;
    }

    int fileLength = 0;
    while (fileBuffer[fileLength] != '\0')
    {
        ++fileLength;
    }

    int cursor = 0;
    int lineLength = -1;
    int taskDays[5] = {0};
    while ((lineLength = ReadLine(fileBuffer, lineBuffer, cursor)) != 0)
    {
        cursor += lineLength + 1;
        if (lineBuffer[0] == '<')
        {
            for (int i = 0; i < 5; ++i)
            {
                taskDays[i] = 0;
            }
            char dayName[10];
            int dayStart = 1;
            while (lineBuffer[dayStart] != '>')
            {
                dayName[dayStart - 1] = lineBuffer[dayStart];
                ++dayStart;
            }
            dayName[dayStart - 1] = '\0';

            int count = 0;
            int index = 0;
            while (fileBuffer[cursor + index] != '<' && cursor + index < fileLength)
            {
                if (fileBuffer[cursor + index] == '~')
                {
                    ++count;
                }
                ++index;
            }

            if (loadTemplate)
            {
                for (int day = 1; day <= daysInMonth[now.tm_mon]; ++day)
                {
                    if (TextIsEqual(dayName, GetWeekday(day, now)))
                    {
                        if (count > 0)
                        {
                            tasksCounts[day - 1] = count;
                            tasksMap[day - 1] = (Task *)calloc(count, sizeof(Task));
                            for (int i = 0; i < 5; ++i)
                            {
                                if (taskDays[i] == 0)
                                {
                                    taskDays[i] = day;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            daysCompleted[day - 1] = true;
                        }
                        // printf("Found %s on %d\n", dayName, day);
                    }
                }
            }
            // printf("Found %d tasks for day %s\n", count, dayName);
        }
        else if (lineBuffer[0] == '~')
        {
            if (loadTemplate)
            {
                int taskStart = 2;
                int taskLength = 0;
                while (lineBuffer[taskStart + taskLength] != '|')
                {
                    ++taskLength;
                }
                taskLength -= 1;

                int fromStart = taskStart + taskLength + 3;
                char hourFrom[2] = {lineBuffer[fromStart], lineBuffer[fromStart + 1]};
                char minuteFrom[2] = {lineBuffer[fromStart + 3], lineBuffer[fromStart + 4]};
                char hourTo[2] = {lineBuffer[fromStart + 6], lineBuffer[fromStart + 7]};
                char minuteTo[2] = {lineBuffer[fromStart + 9], lineBuffer[fromStart + 10]};

                int colorStart = fromStart + 15;
                char colorHex[9];

                for (int i = 0; i < 8; ++i)
                {
                    colorHex[i] = lineBuffer[colorStart + i];
                }
                colorHex[8] = '\0';

                for (int i = 0; i < 5; ++i)
                {
                    if (taskDays[i] == 0)
                    {
                        break;
                    }
                    int day = taskDays[i] - 1;

                    for (int j = 0; j < tasksCounts[day]; ++j)
                    {
                        if (tasksMap[day][j].name == NULL)
                        {
                            Task *task = &tasksMap[day][j];
                            task->name = (char *)malloc((taskLength + 1) * sizeof(char));
                            for (int k = 0; k < taskLength; ++k)
                            {
                                task->name[k] = lineBuffer[taskStart + k];
                            }
                            task->name[taskLength] = '\0';
                            task->hourFrom = atoi(hourFrom);
                            task->minuteFrom = atoi(minuteFrom);
                            task->hourTo = atoi(hourTo);
                            task->minuteTo = atoi(minuteTo);
                            task->color = GetColor(hex2int(colorHex));
                            Vector3 hsv = ColorToHSV(task->color);
                            task->checkColor = ColorFromHSV(hsv.x, hsv.y, hsv.z + 0.5 > 1.0 ? 1.0 : hsv.z + 0.5);
                            task->completed = false;
                            // printf("Set task name %s at day %d - from: %d:%d to: %d:%d\n", task->name, day + 1, task->hourFrom, task->minuteFrom, task->hourTo, task->minuteTo);
                            break;
                        }
                    }
                }
            }
            else
            {
                char dayNumber[3];
                int tasksStart;
                if (lineBuffer[3] != ' ')
                {
                    dayNumber[0] = lineBuffer[2];
                    dayNumber[1] = lineBuffer[3];
                    dayNumber[2] = '\0';
                    tasksStart = 7;
                }
                else
                {
                    dayNumber[0] = lineBuffer[2];
                    dayNumber[1] = '\0';
                    tasksStart = 6;
                }

                int day = atoi(dayNumber);
                Task *tasks = tasksMap[day - 1];
                int taskStart = tasksStart;
                char *taskName = (char *)malloc(128 * sizeof(char));
                int tasksCompleted = 0;
                for (int i = tasksStart; i < lineLength; ++i)
                {
                    if (lineBuffer[i] == ',')
                    {
                        for (int j = 0; j < i - taskStart; ++j)
                        {
                            taskName[j] = lineBuffer[taskStart + j];
                        }
                        taskName[i - taskStart] = '\0';
                        taskStart = i + 2;
                        for (int k = 0; k < tasksCounts[day - 1]; ++k)
                        {
                            Task *task = &tasks[k];
                            if (TextIsEqual(taskName, task->name))
                            {
                                task->completed = true;
                                ++tasksCompleted;
                            }
                        }
                    }
                }

                if (tasksCompleted == tasksCounts[day - 1])
                {
                    daysCompleted[day - 1] = true;
                }

                free(taskName);
            }
        }
    }

    free(lineBuffer);
    UnloadFileText(fileBuffer);
}

void SaveCompletedTasks()
{
    FILE *file = fopen("resources/completed.todo", "w");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }
    for (int weekday = 0; weekday < 7; ++weekday)
    {
        char *dayName = weekdays[weekday];
        fprintf(file, "<%s>\n", dayName);
        for (int i = 0; i < 5; ++i)
        {
            int day = weekdaysInMonth[weekday][i];
            if (day != 0 && tasksCounts[day - 1] != 0)
            {
                fprintf(file, "~ %d | ", day);
                Task *tasks = tasksMap[day - 1];
                for (int j = 0; j < tasksCounts[day - 1]; ++j)
                {
                    Task *task = &tasks[j];
                    if (task->completed)
                    {
                        fprintf(file, "%s, ", task->name);
                    }
                }
                fprintf(file, "\n");
            }
        }
    }
    fprintf(file, "\n");
    fclose(file);
}

void FreeTasks()
{
    time_t timestamp = time(NULL);
    struct tm now = *localtime(&timestamp);
    for (int i = 0; i < daysInMonth[now.tm_mon]; ++i)
    {
        for (int j = 0; j < tasksCounts[i]; ++j)
        {
            free(tasksMap[i][j].name);
        }

        if (tasksCounts[i] > 0)
        {
            free(tasksMap[i]);
        }
    }
    free(tasksMap);
}

int main()
{
    RemoveConsole();
    time_t timestamp = time(NULL);
    struct tm now = *localtime(&timestamp);

    daysInMonth[1] = (now.tm_year + 1900) % 4 == 0 ? 29 : 28;

    for (int day = 1; day <= daysInMonth[now.tm_mon]; ++day)
    {
        for (int weekday = 0; weekday < 7; ++weekday)
        {
            char *dayName = weekdays[weekday];
            if (TextIsEqual(dayName, GetWeekday(day, now)))
            {
                for (int i = 0; i < 5; ++i)
                {
                    if (weekdaysInMonth[weekday][i] == 0)
                    {
                        weekdaysInMonth[weekday][i] = day;
                        break;
                    }
                }
            }
        }
    }

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_UNDECORATED);
    InitWindow(windowWidth, windowHeight, "Todo Desktop Widget");

    LoadTasks(true);
    if (now.tm_mday == 1)
    {
        int monthCleared = LoadStorageValue(MONTH_CLEARED);
        if (monthCleared != 1)
        {
            FILE *file = fopen("resources/completed.todo", "w");
            if (file == NULL)
            {
                printf("Error opening file!\n");
                return;
            }
            fclose(file);
            SaveStorageValue(MONTH_CLEARED, 1);
        }
    }
    else
    {
        SaveStorageValue(MONTH_CLEARED, 0);
    }

    LoadTasks(false);

    SetTargetFPS(60);
    SetWindowOnBottom(GetWindowHandle());
    SetWindowPosition(LoadStorageValue(X_WINDOW_POSITION), LoadStorageValue(Y_WINDOW_POSITION));

    currentPanel = (Panel){0, 0};
    windowHeight = GetNewWindowSize();
    RefreshWindow();

    Shader panelShader = LoadShader(0, "resources/panel_shader.fs");

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
    taskOffsetX = MeasureTextEx(hourFont, "00:00", 18, 1).x + 20;

    while (!WindowShouldClose())
    {

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            SetWindowOnBottom(GetWindowHandle());
            int mouseX = GetMouseX();
            int mouseY = GetMouseY();
            int buttonCenter = panelOffsetY + dayButtonClicked * buttonSize + dayButtonClicked * buttonSpacing + buttonSize / 2;
            if (dayButtonClicked != -1 && mouseX > buttonSize && (mouseY < currentPanel.y || mouseY > currentPanel.y + panelHeight))
            {
                dayButtonClicked = -1;
                SetWindowSize(windowWidth, windowHeight);
                SaveCompletedTasks();
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
                UnloadTexture(baseButtons);
                UnloadTexture(completedButtons);
                UnloadTexture(currentButtons);
                UnloadTexture(checkTexture);
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

            int previousDayButtonClicked = dayButtonClicked;
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

                if (previousDayButtonClicked != -1 && dayButtonClicked != previousDayButtonClicked)
                {
                    SaveCompletedTasks();
                }
            }
        }

        if (dayButtonClicked != -1)
        {
            if (GetScreenWidth() != extendedWindowWidth)
            {
                SetWindowSize(extendedWindowWidth, windowHeight);
            }
            timestamp = time(NULL);
            now = *localtime(&timestamp);

            BeginShaderMode(panelShader);
            DrawRectangle(currentPanel.x, currentPanel.y, panelWidth, panelHeight, BLANK);
            EndShaderMode();

            char *weekday = GetWeekday(dayButtonClicked + 1, now);
            char buffer[14];
            snprintf(buffer, 14, "%s, %d", weekday, dayButtonClicked + 1);
            DrawTextEx(headerFont, buffer, (Vector2){currentPanel.x + (panelWidth / 2 - MeasureTextEx(headerFont, buffer, 40, 1).x / 2), currentPanel.y - buttonSize / 2 - 20}, 40, 1, WHITE);

            for (int i = panelHeight / 24; i < panelHeight; i += panelHeight / 24)
            {
                DrawLine(currentPanel.x, currentPanel.y + i, currentPanel.x + panelWidth, currentPanel.y + i, DARKGRAY);
            }

            Task *tasks = tasksMap[dayButtonClicked];
            bool allTasksCompleted = true;
            for (int i = 0; i < tasksCounts[dayButtonClicked]; ++i)
            {
                Task *task = &tasks[i];
                int taskFromY = currentPanel.y + panelHeight * ((task->hourFrom * 60 + task->minuteFrom) / 1440.0);
                int taskToY = currentPanel.y + panelHeight * ((task->hourTo * 60 + task->minuteTo) / 1440.0);

                DrawRectangle(currentPanel.x + taskOffsetX, taskFromY, panelWidth - taskOffsetX - 15, taskToY - taskFromY, task->color);

                DrawTextEx(headerFont, task->name, (Vector2){currentPanel.x + taskOffsetX + 15, taskToY - buttonSize / 2}, buttonSize / 2, 1, WHITE);

                int checkboxX = currentPanel.x + panelWidth - 15 - 5 - buttonSize / 3;
                task->completed = GuiCheckBox((Rectangle){checkboxX, taskFromY + (taskToY - taskFromY) - 5 - buttonSize / 3, buttonSize / 3, buttonSize / 3}, NULL, task->completed, checkTexture, task->checkColor);
                if (!task->completed)
                {
                    allTasksCompleted = false;
                }
            }

            daysCompleted[dayButtonClicked] = allTasksCompleted;

            int nowLineY = currentPanel.y + panelHeight * ((now.tm_hour * 60 + now.tm_min) / 1440.0);
            DrawLineEx((Vector2){currentPanel.x, nowLineY}, (Vector2){currentPanel.x + panelWidth, nowLineY}, 2.0, (Color){255, 92, 64, 255});

            for (int i = panelHeight / 24; i < panelHeight; i += panelHeight / 24)
            {
                char number[6];
                int hour = i / (panelHeight / 24);
                snprintf(number, 6, hour < 10 ? "0%d:00" : "%d:00", hour);
                DrawTextEx(hourFont, number, (Vector2){currentPanel.x + 5, currentPanel.y + i - 18}, 18, 1, DARKGRAY);
            }
        }

        EndDrawing();
    }

    UnloadTexture(baseButtons);
    UnloadTexture(completedButtons);
    UnloadTexture(currentButtons);
    UnloadTexture(checkTexture);
    UnloadShader(panelShader);
    UnloadFont(headerFont);
    UnloadFont(hourFont);
    FreeTasks();
    CloseWindow();

    return 0;
}
