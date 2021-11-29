/*******************************************************************************************
*
*   raygui v3.0-dev - A simple and easy-to-use immediate-mode gui library
*
*   DESCRIPTION:
*
*   raygui is a tools-dev-focused immediate-mode-gui library based on raylib but also
*   available as a standalone library, as long as input and drawing functions are provided.
*
*   Controls provided:
*
*   # Container/separators Controls
*       - WindowBox
*       - GroupBox
*       - Line
*       - Panel
*
*   # Basic Controls
*       - Label
*       - Button
*       - LabelButton   --> Label
*       - ImageButton   --> Button
*       - ImageButtonEx --> Button
*       - Toggle
*       - ToggleGroup   --> Toggle
*       - CheckBox
*       - ComboBox
*       - DropdownBox
*       - TextBox
*       - TextBoxMulti
*       - ValueBox      --> TextBox
*       - Spinner       --> Button, ValueBox
*       - Slider
*       - SliderBar     --> Slider
*       - ProgressBar
*       - StatusBar
*       - ScrollBar
*       - ScrollPanel
*       - DummyRec
*       - Grid
*
*   # Advance Controls
*       - ListView
*       - ColorPicker   --> ColorPanel, ColorBarHue
*       - MessageBox    --> Window, Label, Button
*       - TextInputBox  --> Window, Label, TextBox, Button
*
*   It also provides a set of functions for styling the controls based on its properties (size, color).
*
*
*   GUI STYLE (guiStyle):
*
*   raygui uses a global data array for all gui style properties (allocated on data segment by default),
*   when a new style is loaded, it is loaded over the global style... but a default gui style could always be 
*   recovered with GuiLoadStyleDefault() function, that overwrites the current style to the default one
*
*   The global style array size is fixed and depends on the number of controls and properties:
*
*       static unsigned int guiStyle[RAYGUI_MAX_CONTROLS*(RAYGUI_MAX_PROPS_BASE + RAYGUI_MAX_PROPS_EXTENDED)];
*
*   guiStyle size is by default: 16*(16 + 8) = 384*4 = 1536 bytes = 1.5 KB
*
*   Note that the first set of BASE properties (by default guiStyle[0..15]) belong to the generic style
*   used for all controls, when any of those base values is set, it is automatically populated to all
*   controls, so, specific control values overwriting generic style should be set after base values.
*
*   After the first BASE set we have the EXTENDED properties (by default guiStyle[16..23]), those
*   properties are actually common to all controls and can not be overwritten individually (like BASE ones)
*   Some of those properties are: TEXT_SIZE, TEXT_SPACING, LINE_COLOR, BACKGROUND_COLOR
*
*   Custom control properties can be defined using the EXTENDED properties for each independent control.
*
*   TOOL: rGuiStyler is a visual tool to customize raygui style.
*
*
*   GUI ICONS (guiIcons):
*
*   raygui could use a global array containing icons data (allocated on data segment by default),
*   a custom icons set could be loaded over this array using GuiLoadIcons(), but loaded icons set 
*   must be same RICON_SIZE and no more than RICON_MAX_ICONS will be loaded
*
*   Every icon is codified in binary form, using 1 bit per pixel, so, every 16x16 icon 
*   requires 8 integers (16*16/32) to be stored in memory.
*
*   When the icon is draw, actually one quad per pixel is drawn if the bit for that pixel is set.
*
*   The global icons array size is fixed and depends on the number of icons and size:
*
*       static unsigned int guiIcons[RICON_MAX_ICONS*RICON_DATA_ELEMENTS];
*
*   guiIcons size is by default: 256*(16*16/32) = 2048*4 = 8192 bytes = 8 KB
*
*   TOOL: rGuiIcons is a visual tool to customize raygui icons.
*
*
*   CONFIGURATION:
*
*   #define RAYGUI_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers
*       or source files without problems. But only ONE file should hold the implementation.
*
*   #define RAYGUI_STANDALONE
*       Avoid raylib.h header inclusion in this file. Data types defined on raylib are defined
*       internally in the library and input management and drawing functions must be provided by
*       the user (check library implementation for further details).
*
*   #define RAYGUI_SUPPORT_RICONS
*       Includes embedded ricons data (binary format) and definitions (by default 256 16x16 pixels, 2KB)
*
*   #define RAYGUI_SUPPORT_CUSTOM_RICONS
*       Includes custom ricons.h header defining a set of custom icons, 
*       this file can be generated using rGuiIcons tool
*
*
*   VERSIONS HISTORY:
*
*       3.0 (xx-Sep-2021) Integrated ricons data to avoid external file
*                         REDESIGNED: GuiTextBoxMulti()
*       2.9 (17-Mar-2021) Removed tooltip API
*       2.8 (03-May-2020) Centralized rectangles drawing to GuiDrawRectangle()
*       2.7 (20-Feb-2020) Added possible tooltips API
*       2.6 (09-Sep-2019) ADDED: GuiTextInputBox()
*                         REDESIGNED: GuiListView*(), GuiDropdownBox(), GuiSlider*(), GuiProgressBar(), GuiMessageBox()
*                         REVIEWED: GuiTextBox(), GuiSpinner(), GuiValueBox(), GuiLoadStyle()
*                         Replaced property INNER_PADDING by TEXT_PADDING, renamed some properties
*                         Added 8 new custom styles ready to use
*                         Multiple minor tweaks and bugs corrected
*       2.5 (28-May-2019) Implemented extended GuiTextBox(), GuiValueBox(), GuiSpinner()
*       2.3 (29-Apr-2019) Added rIcons auxiliar library and support for it, multiple controls reviewed
*                         Refactor all controls drawing mechanism to use control state
*       2.2 (05-Feb-2019) Added GuiScrollBar(), GuiScrollPanel(), reviewed GuiListView(), removed Gui*Ex() controls
*       2.1 (26-Dec-2018) Redesign of GuiCheckBox(), GuiComboBox(), GuiDropdownBox(), GuiToggleGroup() > Use combined text string
*                         Complete redesign of style system (breaking change)
*       2.0 (08-Nov-2018) Support controls guiLock and custom fonts, reviewed GuiComboBox(), GuiListView()...
*       1.9 (09-Oct-2018) Controls review: GuiGrid(), GuiTextBox(), GuiTextBoxMulti(), GuiValueBox()...
*       1.8 (01-May-2018) Lot of rework and redesign to align with rGuiStyler and rGuiLayout
*       1.5 (21-Jun-2017) Working in an improved styles system
*       1.4 (15-Jun-2017) Rewritten all GUI functions (removed useless ones)
*       1.3 (12-Jun-2017) Redesigned styles system
*       1.1 (01-Jun-2017) Complete review of the library
*       1.0 (07-Jun-2016) Converted to header-only by Ramon Santamaria.
*       0.9 (07-Mar-2016) Reviewed and tested by Albert Martos, Ian Eito, Sergio Martinez and Ramon Santamaria.
*       0.8 (27-Aug-2015) Initial release. Implemented by Kevin Gato, Daniel Nicolás and Ramon Santamaria.
*
*
*   CONTRIBUTORS:
*
*       Ramon Santamaria:   Supervision, review, redesign, update and maintenance
*       Vlad Adrian:        Complete rewrite of GuiTextBox() to support extended features (2019)
*       Sergio Martinez:    Review, testing (2015) and redesign of multiple controls (2018)
*       Adria Arranz:       Testing and Implementation of additional controls (2018)
*       Jordi Jorba:        Testing and Implementation of additional controls (2018)
*       Albert Martos:      Review and testing of the library (2015)
*       Ian Eito:           Review and testing of the library (2015)
*       Kevin Gato:         Initial implementation of basic components (2014)
*       Daniel Nicolas:     Initial implementation of basic components (2014)
*
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2014-2020 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RAYGUI_H
#define RAYGUI_H

#define RAYGUI_VERSION  "3.0-dev"

#if !defined(RAYGUI_STANDALONE)
    #include "raylib.h"
#endif

#ifndef RAYGUIDEF
    #define RAYGUIDEF       // We are building or using rlgl as a static library (or Linux shared library)
#endif

// Define functions scope to be used internally (static) or externally (extern) to the module including this file
#if defined(_WIN32)
    // Microsoft attibutes to tell compiler that symbols are imported/exported from a .dll
    #if defined(BUILD_LIBTYPE_SHARED)
        #define RAYGUIDEF __declspec(dllexport)     // We are building raygui as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define RAYGUIDEF __declspec(dllimport)     // We are using raygui as a Win32 shared library (.dll)
    #endif
#endif

#if !defined(RAYGUI_MALLOC) && !defined(RAYGUI_CALLOC) && !defined(RAYGUI_FREE)
    #include <stdlib.h>                 // Required for: malloc(), calloc(), free()
#endif

// Allow custom memory allocators
#ifndef RAYGUI_MALLOC
    #define RAYGUI_MALLOC(sz)       malloc(sz)
#endif
#ifndef RAYGUI_CALLOC
    #define RAYGUI_CALLOC(n,sz)     calloc(n,sz)
#endif
#ifndef RAYGUI_FREE
    #define RAYGUI_FREE(p)          free(p)
#endif

#define BORDER_WIDTH 2
#define CHECK_PADDING 2

bool GuiTextureButtonEx(Rectangle bounds, const char *text, Texture2D texture, Rectangle texSource)
{
    bool clicked = false;
    Color tintColor = WHITE;
    Vector2 mousePoint = GetMousePosition();

    // Check button state
    if (CheckCollisionPointRec(mousePoint, bounds))
    {
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            clicked = true;
        }
        else
        {
            //tintColor = (Color){255, 150, 150, 255};
            tintColor = (Color){ 216, 191, 255, 255 };
        }
    }

    if (texture.id > 0)
    {
        DrawTextureRec(texture, texSource, (Vector2){bounds.x + bounds.width / 2 - texSource.width / 2, bounds.y + bounds.height / 2 - texSource.height / 2}, tintColor);
    }

    return clicked;
}

void GuiDrawRectangle(Rectangle rec, int borderWidth, Color borderColor, Color color)
{
    if (color.a > 0)
    {
        // Draw rectangle filled with color
        DrawRectangle((int)rec.x, (int)rec.y, (int)rec.width, (int)rec.height, color);
    }

    if (borderWidth > 0)
    {
        // Draw rectangle border lines with color
        DrawRectangle((int)rec.x, (int)rec.y, (int)rec.width, borderWidth, borderColor);
        DrawRectangle((int)rec.x, (int)rec.y + borderWidth, borderWidth, (int)rec.height - 2 * borderWidth, borderColor);
        DrawRectangle((int)rec.x + (int)rec.width - borderWidth, (int)rec.y + borderWidth, borderWidth, (int)rec.height - 2 * borderWidth, borderColor);
        DrawRectangle((int)rec.x, (int)rec.y + (int)rec.height - borderWidth, (int)rec.width, borderWidth, borderColor);
    }
}

bool GuiCheckBox(Rectangle bounds, const char *text, bool checked, Texture2D checkedTexture, Color checkColor)
{
    Vector2 mousePoint = GetMousePosition();

    Rectangle totalBounds = {
        bounds.x,
        bounds.y,
        bounds.width,
        bounds.height,
    };

    Color checkBoxColor = checked ? checkColor : WHITE;

    // Check checkbox state
    if (CheckCollisionPointRec(mousePoint, totalBounds))
    {
        checkBoxColor = checked ? WHITE : checkColor;
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            checked = !checked;
        }
    }

    GuiDrawRectangle(bounds, BORDER_WIDTH, checkBoxColor, BLANK);

    if (checked)
    {
        Rectangle check = {bounds.x + BORDER_WIDTH + CHECK_PADDING,
                           bounds.y + BORDER_WIDTH + CHECK_PADDING,
                           bounds.width - 2 * (BORDER_WIDTH + CHECK_PADDING),
                           bounds.height - 2 * (BORDER_WIDTH + CHECK_PADDING)};
        // GuiDrawRectangle(check, 0, BLANK, RED);
        DrawTextureEx(checkedTexture, (Vector2){bounds.x, bounds.y}, 1.0, bounds.width / 64, checkColor);
    }

    return checked;
}
#endif