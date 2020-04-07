/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2008 Yves Rizoud
    Copyright 2008 Adrien Destugues
    Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)

    Grafx2 is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; version 2
    of the License.

    Grafx2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grafx2; if not, see <http://www.gnu.org/licenses/>
*/
#include "struct.h"
#include "global.h"
#include "hotkeys.h"
#include "keycodes.h"

#if defined(__VBCC__) || defined(_MSC_VER)
  #define false 0
  #define true 1
#endif

T_Key_config ConfigKey[NB_SHORTCUTS] = {
  {0,
  "Scroll up",
  "Scrolls the picture up, both in",
  "magnify and normal mode.",
  "",
  false,
  KEY_UP, // HAUT
  0},
  {1,
  "Scroll down",
  "Scrolls the picture down, both in",
  "magnify and normal mode.",
  "",
  false,
  KEY_DOWN, // BAS
  0},
  {2,
  "Scroll left",
  "Scrolls the picture to the left,",
  "both in magnify and normal mode.",
  "",
  false,
  KEY_LEFT, // GAUCHE
  0},
  {3,
  "Scroll right",
  "Scrolls the picture to the right,",
  "both in magnify and normal mode.",
  "",
  false,
  KEY_RIGHT, // DROITE
  0},
#ifdef GCWZERO
#define FAST_MOD GFX2_MOD_CTRL
#else
#define FAST_MOD GFX2_MOD_SHIFT
#endif
  {4,
  "Faster scroll up",
  "Used to scroll upwards in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  KEY_UP|FAST_MOD, // Shift + Up
  0},
  {5,
  "Faster scroll down",
  "Used to scroll downwards in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  KEY_DOWN|FAST_MOD, // Shift + Down
  0},
  {6,
  "Faster scroll left",
  "Used to scroll to the left in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  KEY_LEFT|FAST_MOD, // Shift + Left
  0},
  {7,
  "Faster scroll right",
  "Used to scroll to the right in the",
  "picture fast, either in magnify and",
  "normal mode.",
  true,
  KEY_RIGHT|FAST_MOD, // Shift + Right
  0},
#undef FAST_MOD
  {8,
  "Slower scroll up",
  "Used to scroll upwards in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  KEY_UP|GFX2_MOD_ALT, // Alt + Haut
  0},
  {9,
  "Slower scroll down",
  "Used to scroll downwards in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  KEY_DOWN|GFX2_MOD_ALT, // Alt + Bas
  0},
  {10,
  "Slower scroll left",
  "Used to scroll to the left in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  KEY_LEFT|GFX2_MOD_ALT, // Alt + Gauche
  0},
  {11,
  "Slower scroll right",
  "Used to scroll to the right in the",
  "picture pixel by pixel, either in",
  "magnify and normal mode.",
  true,
  KEY_RIGHT|GFX2_MOD_ALT, // Alt + Droite
  0},
  {12,
  "Move mouse cursor 1 pixel up",
  "Used to simulate a very small mouse",
  "deplacement up.It's very useful",
  "when you want ultra-high precision.",
  true,
#ifdef GCWZERO
  KEY_UNKNOWN,
#else
  KEY_UP|GFX2_MOD_CTRL, // Ctrl + Haut
#endif
  0},
  {13,
  "Move mouse cursor 1 pixel down",
  "Used to simulate a very small mouse",
  "deplacement down.It's very useful",
  "when you want ultra-high precision.",
  true,
#ifdef GCWZERO
  KEY_UNKNOWN,
#else
  KEY_DOWN|GFX2_MOD_CTRL, // Ctrl + Bas
#endif
  0},
  {14,
  "Move mouse cursor 1 pixel left",
  "Used to simulate a very small mouse",
  "deplacement left.It's very useful",
  "when you want ultra-high precision.",
  true,
#ifdef GCWZERO
  KEY_UNKNOWN,
#else
  KEY_LEFT|GFX2_MOD_CTRL, // Ctrl + Gauche
#endif
  0},
  {15,
  "Move mouse cursor 1 pixel right",
  "Used to simulate a very small mouse",
  "deplacement right.It's very useful",
  "when you want ultra-high precision.",
  true,
#ifdef GCWZERO
  KEY_UNKNOWN,
#else
  KEY_RIGHT|GFX2_MOD_CTRL, // Ctrl + Droite
#endif
  0},
  {16,
  "Simulate left mouse click",
  "Used to simulate a click with the",
  "left mouse button. It's useful",
  "when you want ultra-high precision.",
  true,
#ifdef GCWZERO
  KEY_SPACE, // Space
#else
  KEY_SPACE|GFX2_MOD_CTRL, // Ctrl + Space
#endif
  0},
  {17,
  "Simulate right mouse click",
  "Used to simulate a click with the",
  "right mouse button.. It's useful",
  "when you want ultra-high precision.",
  true,
#ifdef GCWZERO
  KEY_BACKSPACE, // R-shoulderpad
#else
  KEY_SPACE|GFX2_MOD_SHIFT, // Shift + Space
#endif
  0},
  {18,
  "Show/hide menu toolbars",
  "Hides all toolbar menus, or shows",
  "them back.",
  "",
  false,
  KEY_F10, // F10
  0},
  {19,
  "Show/hide cursor",
  "Switch the cursor display on/off.",
  "This only works on the \"small cross\"",
  "and \"hand\" cursors.",
  true,
  KEY_F9, // F9
  0},
  {20,
  "Set paintbrush to 1 pixel",
  "Useful when you want to use a",
  "\"single-pixel-brush\".",
  "",
  true,
  KEY_DELETE, // Del
  0},
  {21,
  "Paintbrush choice",
  "Opens a menu where you can choose a",
  "paintbrush out of 24 predefined",
  "ones.",
  true,
  KEY_F4, // F4
  0},
  {22,
  "Monochrome brush",
  "Turn your current user-defined brush",
  "into a single colored one. All non-",
  "transparent colors are set to FG.",
  true,
  KEY_F4|GFX2_MOD_SHIFT, // Shift + F4
  0},
  {23,
  "Freehand drawing",
  "Set the drawing mode to the",
  "classical freehand one.",
  "",
  true,
  KEY_d, // D
  0},
  {24,
  "Switch freehand drawing mode",
  "Alternates between: continuous,",
  "discontinuous, point by point,",
  "and contour fill",
  true,
  KEY_d|GFX2_MOD_SHIFT, // Shift + D
  0},
  {25,
  "Continuous freehand drawing",
  "Switch directly to continuous",
  "freehand drawing mode.",
  "",
  true,
  KEY_d|GFX2_MOD_CTRL, // Ctrl + D
  0},
  {26,
  "Line",
  "Allows you to draw lines.",
  "",
  "",
  true,
  KEY_l, // L
  0},
  {27,
  "Knotted lines",
  "Allows you to draw linked lines.",
  "This mode can also be called",
  "\"Polyline\".",
  true,
  KEY_l|GFX2_MOD_SHIFT, // Shift + L
  0},
  {28,
  "Spray",
  "Allows you to spray brushes",
  "randomly in the picture.",
  "",
  true,
  KEY_a, // A (Q en AZERTY)
  0},
  {29,
  "Spray menu",
  "Opens a menu in which you can",
  "configure the spray flow and size.",
  "",
  true,
  KEY_a|GFX2_MOD_SHIFT, // Shift + A
  0},
  {30,
  "Flood-fill",
  "Allows you to fill an area of the",
  "picture made of pixels of the same",
  "color.",
  true,
  KEY_f, // F
  0},
  {124,
  "Replace color",
  "This tool replaces all the pixels of",
  "the clicked color to the fore-color",
  "or the back-color.",
  true,
  KEY_f|GFX2_MOD_SHIFT, // Shift + F
  0},
  {31,
  "Bezier's curves",
  "Allows you to draw Bezier's curves.",
  "",
  "",
  true,
  KEY_i, // I
  0},
  {32,
  "Bezier's curve with 3 or 4 points",
  "Allows you to choose whether you",
  "want to draw Bezier's curves with",
  "3 or 4 points.",
  true,
  KEY_i|GFX2_MOD_SHIFT, // Shift + I
  0},
  {33,
  "Empty rectangle",
  "Allows you to draw a rectangle using",
  "the brush.",
  "",
  true,
  KEY_r, // R
  0},
  {34,
  "Filled rectangle",
  "Allows you to draw a filled",
  "rectangle.",
  "",
  true,
  KEY_r|GFX2_MOD_SHIFT, // Shift + R
  0},
  {35,
  "Empty circle",
  "Allows you to draw a circle using",
  "the brush.",
  "",
  true,
  KEY_c, // C
  0},
  {36,
  "Empty ellipse",
  "Allows you to draw an ellipse using",
  "the brush.",
  "",
  true,
  KEY_c|GFX2_MOD_CTRL, // Ctrl + C
  0},
  {37,
  "Filled circle",
  "Allows you to draw a filled circle.",
  "",
  "",
  true,
  KEY_c|GFX2_MOD_SHIFT, // Shift + C
  0},
  {38,
  "Filled ellipse",
  "Allows you to draw a filled ellipse.",
  "",
  "",
  true,
  KEY_c|GFX2_MOD_SHIFT|GFX2_MOD_CTRL, // Shift + Ctrl + C
  0},
  {39,
  "Empty polygon",
  "Allows you to draw a polygon using",
  "the brush.",
  "",
  true,
  KEY_n, // N
  0},
  {40,
  "Empty \"polyform\"",
  "Allows you to draw a freehand",
  "polygon using the brush.",
  "",
  true,
  KEY_n|GFX2_MOD_CTRL, // Ctrl + N
  0},
  {41,
  "Filled polygon",
  "Allows you to draw a filled polygon.",
  "",
  "",
  true,
  KEY_n|GFX2_MOD_SHIFT, // Shift + N
  0},
  {42,
  "Filled \"polyform\"",
  "Allows you to draw a filled freehand",
  "polygon.",
  "",
  true,
  KEY_n|GFX2_MOD_SHIFT|GFX2_MOD_CTRL, // Shift + Ctrl + N
  0},
  {43,
  "Rectangle with gradation",
  "Allows you to draw a rectangle with",
  "a color gradation.",
  "",
  true,
  KEY_r|GFX2_MOD_ALT, // Alt + R
  0},
  {44,
  "Gradation menu",
  "Allows you to configure the way",
  "color gradations are calculated.",
  "",
  true,
  KEY_g|GFX2_MOD_ALT, // Alt + G
  0},
  {45,
  "Sphere with gradation",
  "Allows you to draw a rectangle with",
  "a color gradation.",
  "",
  true,
  KEY_c|GFX2_MOD_ALT, // Alt + C
  0},
  {46,
  "Ellipse with gradation",
  "Allows you to draw an ellipse filled",
  "with a color gradation.",
  "",
  true,
  KEY_c|GFX2_MOD_SHIFT|GFX2_MOD_ALT, // Shift + Alt + C
  0},
  {47,
  "Adjust picture",
  "Allows you to move the whole picture",
  "Around. What gets out from a side",
  "reappears on the other.",
  true,
  KEY_KP5, // Kpad5
  0},
  {48,
  "Picture effects",
  "Opens the 'Picture effects' window.",
  "",
  "",
  true,
  KEY_KP5|GFX2_MOD_SHIFT, // Shift + Kpad5
  0},
  {49,
  "Drawing effects",
  "Opens a menu where you can enable/",
  "disable and configure the drawing",
  "effects.",
  true,
  KEY_e, // E
  0},
  {50,
  "Shade mode",
  "Enables or disables Shade mode",
  "",
  "",
  true,
  KEY_F5, // F5
  0},
  {51,
  "Shade menu",
  "Opens a the menu for Shade settings.",
  "",
  "",
  true,
  KEY_F5|GFX2_MOD_SHIFT, // Shift + F5
  0},
  {131,
  "Quick-shade mode",
  "Enables or disables Quick-shade",
  "mode.",
  "",
  true,
  KEY_F5|GFX2_MOD_CTRL, // Ctrl + F5
  0},
  {132,
  "Quick-shade menu",
  "Opens a the menu for Quick-shade",
  "settings.",
  "",
  true,
  KEY_F5|GFX2_MOD_SHIFT|GFX2_MOD_CTRL, // Shift + Ctrl + F5
  0},
  {52,
  "Stencil mode",
  "Enables or disables Stencil mode.",
  "",
  "",
  true,
  KEY_F6, // F6
  0},
  {53,
  "Stencil menu",
  "Opens a the menu for Stencil",
  "settings.",
  "",
  true,
  KEY_F6|GFX2_MOD_SHIFT, // Shift + F6
  0},
  {54,
  "Mask mode",
  "Enables or disables Mask mode.",
  "",
  "",
  true,
  KEY_F6|GFX2_MOD_ALT, // Alt + F6
  0},
  {55,
  "Mask menu",
  "Opens a the menu for Mask settings.",
  "",
  "",
  true,
  KEY_F6|GFX2_MOD_SHIFT|GFX2_MOD_ALT, // Shift + Alt + F6
  0},
  {56,
  "Grid mode",
  "Enables or disables the Grid mode.",
  "",
  "",
  true,
  KEY_g, // G
  0},
  {57,
  "Grid menu",
  "Open a menu where you can configure",
  "the grid used by Grid mode.",
  "",
  true,
  KEY_g|GFX2_MOD_SHIFT, // Shift + G
  0},
  {58,
  "Sieve mode",
  "Enables or disables the Sieve mode.",
  "",
  "",
  true,
  KEY_g|GFX2_MOD_CTRL, // Ctrl + G
  0},
  {59,
  "Sieve menu",
  "Opens a menu where you can configure",
  "the sieve.",
  "",
  true,
  KEY_g|GFX2_MOD_SHIFT|GFX2_MOD_CTRL, // Shift + Ctrl + G
  0},
  {60,
  "Invert sieve",
  "Inverts the pattern defined in the",
  "Sieve menu.",
  "",
  true,
  KEY_g|GFX2_MOD_CTRL|GFX2_MOD_ALT, // Ctrl + Alt + G
  0},
  {61,
  "Colorize mode",
  "Enables or disables the Colorize",
  "mode.",
  "",
  true,
  KEY_F7, // F7
  0},
  {62,
  "Colorize menu",
  "Opens a menu where you can give the",
  "opacity percentage for Colorize",
  "mode.",
  true,
  KEY_F7|GFX2_MOD_SHIFT, // Shift + F7
  0},
  {63,
  "Smooth mode",
  "Enables or disables the Smooth",
  "mode.",
  "",
  true,
  KEY_F8, // F8
  0},
  {123,
  "Smooth menu",
  "Opens a menu where you can define",
  "the Smooth matrix.",
  "",
  true,
  KEY_F8|GFX2_MOD_SHIFT, // Shift + F8
  0},
  {64,
  "Smear mode",
  "Enables or disables the Smear mode.",
  "",
  "",
  true,
  KEY_F8|GFX2_MOD_ALT, // Alt + F8
  0},
  {65,
  "Tiling mode",
  "Enables or disables the Tiling",
  "mode.",
  "",
  true,
  KEY_b|GFX2_MOD_ALT, // Alt + B
  0},
  {66,
  "Tiling menu",
  "Opens a menu where you can configure",
  "the origin of the tiling.",
  "",
  true,
  KEY_b|GFX2_MOD_SHIFT|GFX2_MOD_ALT, // Shift + Alt + B
  0},
  {206,
  "Tilemap mode",
  "Enables or disables the Tilemap",
  "mode.",
  "",
  true,
  0, // No key
  0},
  {207,
  "Tilemap menu",
  "Opens a menu where you can configure",
  "the tilemap settings.",
  "",
  true,
  0, // No key
  0},
  {67,
  "Classical brush grabbing",
  "Allows you to pick a brush defined",
  "within a rectangle.",
  "",
  true,
  KEY_b, // B
  #if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  KEY_C|GFX2_MOD_META // Right-Amiga + C
  #else
  0
  #endif
  },
  {68,
  "\"Lasso\" brush grabbing",
  "Allows you to pick a brush defined",
  "within a freehand polygon.",
  "",
  true,
  KEY_b|GFX2_MOD_CTRL, // Ctrl + B
  0},
  {69,
  "Get previous brush back",
  "Restore the last user-defined brush.",
  "",
  "",
  true,
  KEY_b|GFX2_MOD_SHIFT, // Shift + B
  0},
  {70,
  "Horizontal brush flipping",
  "Reverse brush horizontally.",
  "",
  "",
  true,
  KEY_x, // X
  0},
  {71,
  "Vertical brush flipping",
  "Reverse brush vertically.",
  "",
  "",
  true,
  KEY_y, // Y
  0},
  {72,
  "90\xb0 brush rotation",
  "Rotate the user-defined brush by 90\xb0",
  "(counter-clockwise).",
  "",
  true,
  KEY_z, // Z (W en AZERTY)
  0},
  {73,
  "180\xb0 brush rotation",
  "Rotate the user-defined brush by",
  "180\xb0.",
  "",
  true,
  KEY_z|GFX2_MOD_SHIFT, // Shift + Z
  0},
  {74,
  "Stretch brush",
  "Allows you to resize the",
  "user-defined brush.",
  "",
  true,
  KEY_s, // S
  0},
  {75,
  "Distort brush",
  "Allows you to distort the",
  "user-defined brush.",
  "",
  true,
  KEY_s|GFX2_MOD_SHIFT, // Shift + S
  0},
  {76,
  "Outline brush",
  "Outlines the user-defined brush",
  "with the fore color.",
  "",
  true,
  KEY_o, // O
  0},
  {77,
  "Nibble brush",
  "Deletes the borders of the",
  "user-defined brush.This does the",
  "opposite of the Outline option.",
  true,
  KEY_o|GFX2_MOD_SHIFT, // Shift + O
  0},
  {78,
  "Get colors from brush",
  "Copy colors of the spare page that",
  "are used in the brush.",
  "",
  true,
  KEY_F11, // F11
  0},
  {79,
  "Recolorize brush",
  "Recolorize the user-defined brush in",
  "order to get a brush which looks as",
  "if it was grabbed in the spare page.",
  true,
  KEY_F12, // F12
  0},
  {80,
  "Rotate by any angle",
  "Rotate the brush by an angle that",
  "you can define.",
  "",
  true,
  KEY_w, // W (Z en AZERTY)
  0},
  {81,
  "Pipette",
  "Allows you to copy the color of a",
  "pixel in the picture into the",
  "foreground or background color.",
  true,
  KEY_BACKQUOTE, // `~ (Key sous le Esc - ² en AZERTY)
  0},
  {82,
  "Swap foreground/background colors",
  "Invert foreground and background",
  "colors.",
  "",
  true,
  KEY_BACKQUOTE|GFX2_MOD_SHIFT, // Shift + `~
  0},
  {83,
  "Magnifier mode",
  "Allows you to zoom into the picture.",
  "",
  "",
  true,
  KEY_m, // M (, ? sur AZERTY)
  KEY_MOUSEMIDDLE},
  {84,
  "Zoom factor menu",
  "Opens a menu where you can choose a",
  "magnifying factor.",
  "",
  true,
  KEY_m|GFX2_MOD_SHIFT, // Shift + M
  0},
  {85,
  "Zoom in",
  "Increase magnifying factor.",
  "",
  "",
  true,
  KEY_KP_PLUS, // Grey +
  KEY_MOUSEWHEELUP},
  {86,
  "Zoom out",
  "Decrease magnifying factor.",
  "",
  "",
  true,
  KEY_KP_MINUS, // Grey -
  KEY_MOUSEWHEELDOWN},
  {87,
  "Brush effects menu",
  "Opens a menu which proposes",
  "different effects on the",
  "user-defined brush.",
  true,
  KEY_b|GFX2_MOD_CTRL|GFX2_MOD_ALT, // Ctrl + Alt + B
  0},
  {88,
  "Text",
  "Opens a menu which permits you to",
  "type in a character string and",
  "render it as a brush.",
  true,
  KEY_t, // T
  0},
  {89,
  "Screen resolution menu",
  "Opens a menu where you can choose",
  "the screen resolution and image",
  "dimensions.",
  true,
  KEY_RETURN, // Enter
  0},
  {90,
  "\"Safety\" resolution",
  "Resets the resolution to a 'safe'",
  "mode that should work everywhere:",
  "usually a 640x400 window.",
  false,
  KEY_RETURN|GFX2_MOD_SHIFT, // Shift + Enter
  0},
  {91,
  "Help and credits",
  "Opens a window where you can get",
  "information about the program,",
  "or contextual help.",
  true,
#ifdef GCWZERO
  KEY_TAB, // L-Shoulderpad
#else
  KEY_F1, // F1
#endif
  0},
  {92,
  "Statistics",
  "Displays miscellaneous more or less",
  "useful information.",
  "",
  true,
  KEY_F1|GFX2_MOD_SHIFT, // Shift + F1
  0},
  {93,
  "Jump to spare page",
  "Swap current page and spare page.",
  "",
  "",
  true,
#ifdef GCWZERO
  KEY_UNKNOWN,
#else
  KEY_TAB, // Tab
#endif
  0},
  {94,
  "Copy current page to spare page",
  "Copy current page to spare page.",
  "",
  "",
  true,
  KEY_TAB|GFX2_MOD_SHIFT, // Shift + Tab
  0},
  {95,
  "Save picture as...",
  "Opens a file-selector that allows",
  "you to save your picture with a new",
  "path-name.",
  true,
  KEY_F2, // F2
  #if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  KEY_A|GFX2_MOD_META // Right-Amiga + A
  #else
  0
  #endif
  },
  {96,
  "Save picture",
  "Saves your picture with the last",
  "name you gave it.",
  "",
  true,
  KEY_F2|GFX2_MOD_SHIFT, // Shift + F2
  #if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  KEY_S|GFX2_MOD_META // Right-Amiga + S
  #else
  0
  #endif
  },
  {97,
  "Load picture",
  "Opens a file-selector that allows",
  "you to load a new picture.",
  "",
  true,
  KEY_F3, // F3
  #if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  KEY_O|GFX2_MOD_META // Right-Amiga + O
  #else
  0
  #endif
  },
  {98,
  "Re-load picture",
  "Re-load the current picture. This",
  "allows you to cancel modifications",
  "made since last saving.",
  true,
  KEY_F3|GFX2_MOD_SHIFT, // Shift + F3
  0},
  {99,
  "Save brush",
  "Opens a file-selector that allows",
  "you to save your current",
  "user-defined brush.",
  true,
  KEY_F2|GFX2_MOD_CTRL, // Ctrl + F2
  0},
  {100,
  "Load brush",
  "Opens a file-selector that allows",
  "you to load a brush.",
  "",
  true,
  KEY_F3|GFX2_MOD_CTRL, // Ctrl + F3
  0},
  {101,
  "Settings",
  "Opens a menu which permits you to",
  "modify some parameters of the",
  "program.",
  true,
  KEY_F10|GFX2_MOD_SHIFT, // Shift + F10
  0},
  {102,
  "Undo (Oops!)",
  "Cancel the last action which",
  "modified the picture.",
  "",
  true,
  KEY_u, // U
  // Secondary shortcut is button I on the Caanoo, L on the Wiz, unset on others
  #if defined (__CAANOO__)
    (KEY_JOYBUTTON+JOY_BUTTON_I)
  #elif defined (__WIZ__)
    (KEY_JOYBUTTON+JOY_BUTTON_L)
  #elif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  KEY_Z|GFX2_MOD_META // Right-Amiga + Z
  #else
  0
  #endif
  // --
  },
  {103,
  "Redo",
  "Redo the last undone action.",
  "",
  "",
  true,
  KEY_u|GFX2_MOD_SHIFT, // Shift + U
  // Secondary shortcut is button II on the Caanoo, R on the Wiz, unset on others
  #if defined (__CAANOO__)
    (KEY_JOYBUTTON+JOY_BUTTON_II)
  #elif defined (__WIZ__)
    (KEY_JOYBUTTON+JOY_BUTTON_R)
  #else
    0
  #endif
  // --
  },
  {133,
  "Kill",
  "Kills the current page. It actually",
  "removes the current page from the",
  "list of \"Undo\" pages.",
  true,
  KEY_DELETE|GFX2_MOD_SHIFT, // Shift + Suppr
  0},
  {104,
  "Clear page",
  "Clears the picture with color 0,",
  "or the transparent color if it's",
  "a layered image.",
  true,
#ifdef GCWZERO
  KEY_UNKNOWN, // BackSpace
#else
  KEY_BACKSPACE, // BackSpace
#endif
  0},
  {105,
  "Clear page with backcolor",
  "Clears the picture with the",
  "current backcolor.",
  "",
  true,
  KEY_BACKSPACE|GFX2_MOD_SHIFT, // Shift + BackSpace
  0},
  {106,
  "Quit program",
  "Allows you to leave the program.",
  "If modifications were not saved,",
  "confirmation is asked.",
  false,
  KEY_q, // Q (A en AZERTY)
  // Secondary shortcut is button Home on the Caanoo, Menu on the Wiz, unset on others
  #if defined (__CAANOO__)
    (KEY_JOYBUTTON+JOY_BUTTON_HOME)
  #elif defined (__WIZ__)
    (KEY_JOYBUTTON+JOY_BUTTON_MENU)
  #eif defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
  KEY_Q|GFX2_MOD_META // Right-Amiga + Q
  #else
  0
  #endif
  // --

  },
  {107,
  "Palette menu",
  "Opens a menu which allows you to",
  "modify the current palette.",
  "",
  true,
  KEY_p, // P
  0},
  {125,
  "Secondary palette menu",
  "Opens a menu which allows you to",
  "define color series and some tagged",
  "colors.",
  true,
  KEY_p|GFX2_MOD_SHIFT, // Shift + P
  0},
  {130,
  "Exclude colors menu",
  "Opens a menu which allows you to",
  "define the colors you don't want to",
  "use in Smooth and Transparency",
  true,
  KEY_p|GFX2_MOD_CTRL, // Ctrl + P
  0},
  {108,
  "Scroll palette to the left",
  "Scroll palette in the tool bar to",
  "the left, column by column.",
  "",
  true,
  KEY_PAGEUP, // PgUp
  0},
  {109,
  "Scroll palette to the right",
  "Scroll palette in the tool bar to",
  "the right, column by column.",
  "",
  true,
  KEY_PAGEDOWN, // PgDn
  0},
  {110,
  "Scroll palette to the left faster",
  "Scroll palette in the tool bar to",
  "the left, 8 columns by 8 columns.",
  "",
  true,
  KEY_PAGEUP|GFX2_MOD_SHIFT, // Shift + PgUp
  0},
  {111,
  "Scroll palette to the right faster",
  "Scroll palette in the tool bar to",
  "the right, 8 columns by 8 columns.",
  "",
  true,
  KEY_PAGEDOWN|GFX2_MOD_SHIFT, // Shift + PgDn
  0},
  {112,
  "Center brush attachment point",
  "Set the attachment of the",
  "user-defined brush to its center.",
  "",
  true,
  KEY_KP5|GFX2_MOD_CTRL, // Ctrl + 5 (pavé numérique)
  0},
  {113,
  "Top-left brush attachment point",
  "Set the attachment of the",
  "user-defined brush to its top-left",
  "corner.",
  true,
  KEY_HOME|GFX2_MOD_CTRL, // Ctrl + 7
  0},
  {114,
  "Top-right brush attachment point",
  "Set the attachment of the",
  "user-defined brush to its top-right",
  "corner.",
  true,
  KEY_PAGEUP|GFX2_MOD_CTRL, // Ctrl + 9
  0},
  {115,
  "Bottom-left brush attachment point",
  "Set the attachment of the",
  "user-defined brush to its",
  "bottom-left corner.",
  true,
  KEY_END|GFX2_MOD_CTRL, // Ctrl + 1
  0},
  {116,
  "Bottom-right brush attachment point",
  "Set the attachment of the",
  "user-defined brush to its",
  "bottom-right corner.",
  true,
  KEY_PAGEDOWN|GFX2_MOD_CTRL, // Ctrl + 3
  0},
  {117,
  "Next foreground color",
  "Set the foreground color to the next",
  "in the palette.",
  "",
  true,
#ifdef GCWZERO
  KEY_RIGHT|GFX2_MOD_SHIFT,
#else
  KEY_RIGHTBRACKET, // ] (0x en AZERTY)
#endif
  0},
  {118,
  "Previous foreground color",
  "Set the foreground color to the",
  "previous in the palette.",
  "",
  true,
#ifdef GCWZERO
  KEY_LEFT|GFX2_MOD_SHIFT,
#else
  KEY_LEFTBRACKET, // [ (^ en AZERTY)
#endif
  0},
  {119,
  "Next background color",
  "Set the background color to the next",
  "in the palette.",
  "",
  true,
#ifdef GCWZERO
  KEY_DOWN|GFX2_MOD_SHIFT,
#else
  KEY_RIGHTBRACKET|GFX2_MOD_SHIFT, // Shift + ]
#endif
  0},
  {120,
  "Previous background color",
  "Set the background color to the",
  "previous in the palette.",
  "",
  true,
#ifdef GCWZERO
  KEY_UP|GFX2_MOD_SHIFT,
#else
  KEY_LEFTBRACKET|GFX2_MOD_SHIFT, // Shift + [
#endif
  0},
  {126,
  "Next user-defined forecolor",
  "Set the foreground color to the next",
  "in the user-defined color series.",
  "",
  true,
  KEY_EQUALS, // "=+"
  0},
  {127,
  "Previous user-defined forecolor",
  "Set the foreground color to the",
  "previous in the user-defined color",
  "series.",
  true,
  KEY_MINUS, // "-_" (")°" en AZERTY
  0},
  {128,
  "Next user-defined backcolor",
  "Set the background color to the next",
  "in the user-defined color series.",
  "",
  true,
  KEY_EQUALS|GFX2_MOD_SHIFT, // Shift + "=+"
  0},
  {129,
  "Previous user-defined backcolor",
  "Set the background color to the",
  "previous in the user-defined color",
  "series.",
  true,
  KEY_MINUS|GFX2_MOD_SHIFT, // Shift + "-_" (")°" en AZERTY
  0},
  {121,
  "Shrink paintbrush",
  "Decrease the width of the paintbrush",
  "if it is special circle or square.",
  "",
  true,
  KEY_COMMA, // , < (;. en AZERTY)
  0},
  {122,
  "Enlarge paintbrush",
  "Increase the width of the paintbrush",
  "if it is special circle or square.",
  "",
  true,
  KEY_PERIOD, // .> (:/ en AZERTY)
  0},
  {134,
  "Effects off",
  "Turns off all drawing effects. This",
  "is the same as the 'All off' button",
  "in the Effects screen",
  true,
  KEY_e|GFX2_MOD_SHIFT, // Shift-E
  0},
  {135,
  "Transparency 10%",
  "Turns transparency on and sets its",
  "opacity at 10%.",
  "",
  true,
  KEY_1, // 1
  0},
  {136,
  "Transparency 20%",
  "Turns transparency on and sets its",
  "opacity at 20%.",
  "",
  true,
  KEY_2, // 2
  0},
  {137,
  "Transparency 30%",
  "Turns transparency on and sets its",
  "opacity at 30%.",
  "",
  true,
  KEY_3, // 3
  0},
  {138,
  "Transparency 40%",
  "Turns transparency on and sets its",
  "opacity at 40%.",
  "",
  true,
  KEY_4, // 4
  0},
  {139,
  "Transparency 50%",
  "Turns transparency on and sets its",
  "opacity at 50%.",
  "",
  true,
  KEY_5, // 5
  0},
  {140,
  "Transparency 60%",
  "Turns transparency on and sets its",
  "opacity at 60%.",
  "",
  true,
  KEY_6, // 6
  0},
  {141,
  "Transparency 70%",
  "Turns transparency on and sets its",
  "opacity at 70%.",
  "",
  true,
  KEY_7, // 7
  0},
  {142,
  "Transparency 80%",
  "Turns transparency on and sets its",
  "opacity at 80%.",
  "",
  true,
  KEY_8, // 8
  0},
  {143,
  "Transparency 90%",
  "Turns transparency on and sets its",
  "opacity at 90%.",
  "",
  true,
  KEY_9, // 9
  0},
  {144,
  "Transparency 0%",
  "Turns transparency on and sets its",
  "opacity at 0%.",
  "",
  true,
  KEY_0, // 0
  0},
  {145,
  "Zoom 1:1",
  "Turns magnifier mode off.",
  "",
  "",
  true,
  KEY_1|GFX2_MOD_CTRL, /* Ctrl + 1 */
  0},
  {146,
  "Zoom 2:1",
  "Turns magnifier mode on and set its",
  "factor to 2:1",
  "",
  true,
  KEY_2|GFX2_MOD_CTRL, /* Ctrl + 2 */
  0},
  {147,
  "Zoom 3:1",
  "Turns magnifier mode on and set its",
  "factor to 3:1",
  "",
  true,
  KEY_3|GFX2_MOD_CTRL, /* Ctrl + 3 */
  0},
  {148,
  "Zoom 4:1",
  "Turns magnifier mode on and set its",
  "factor to 4:1",
  "",
  true,
  KEY_4|GFX2_MOD_CTRL, /* Ctrl + 4 */
  0},
  {149,
  "Zoom 5:1",
  "Turns magnifier mode on and set its",
  "factor to 5:1",
  "",
  true,
  KEY_5|GFX2_MOD_CTRL, /* Ctrl + 5 */
  0},
  {150,
  "Zoom 6:1",
  "Turns magnifier mode on and set its",
  "factor to 6:1",
  "",
  true,
  KEY_6|GFX2_MOD_CTRL, /* Ctrl + 6 */
  0},
  {151,
  "Zoom 8:1",
  "Turns magnifier mode on and set its",
  "factor to 8:1",
  "",
  true,
  KEY_7|GFX2_MOD_CTRL, /* Ctrl + 7 */
  0},
  {152,
  "Zoom 10:1",
  "Turns magnifier mode on and set its",
  "factor to 10:1",
  "",
  true,
  KEY_8|GFX2_MOD_CTRL, /* Ctrl + 8 */
  0},
  {153,
  "Zoom 12:1",
  "Turns magnifier mode on and set its",
  "factor to 12:1",
  "",
  true,
  0,
  0},
  {154,
  "Zoom 14:1",
  "Turns magnifier mode on and set its",
  "factor to 14:1",
  "",
  true,
  0,
  0},
  {155,
  "Zoom 16:1",
  "Turns magnifier mode on and set its",
  "factor to 16:1",
  "",
  true,
  0,
  0},
  {156,
  "Zoom 18:1",
  "Turns magnifier mode on and set its",
  "factor to 18:1",
  "",
  true,
  0,
  0},
  {157,
  "Zoom 20:1",
  "Turns magnifier mode on and set its",
  "factor to 20:1",
  "",
  true,
  0,
  0},
  {158,
  "Show/Hide Grid",
  "Turns on or off the visible grid in ",
  "the magnified view. Grid cells match",
  "the size ",
  true,
  KEY_g|GFX2_MOD_SHIFT|GFX2_MOD_ALT, // Shift + Alt + G,
  0},
  {159,
  "Select layer 1",
  "Makes the layer 1 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {160,
  "Toggle layer 1",
  "Makes layer 1 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {161,
  "Select layer 2",
  "Makes the layer 2 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {162,
  "Toggle layer 2",
  "Makes layer 2 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {163,
  "Select layer 3",
  "Makes the layer 3 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {164,
  "Toggle layer 3",
  "Makes layer 3 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {165,
  "Select layer 4",
  "Makes the layer 4 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {166,
  "Toggle layer 4",
  "Makes layer 4 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {167,
  "Select layer 5",
  "Makes the layer 5 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {168,
  "Toggle layer 5",
  "Makes layer 5 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {169,
  "Select layer 6",
  "Makes the layer 6 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {170,
  "Toggle layer 6",
  "Makes layer 6 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {171,
  "Select layer 7",
  "Makes the layer 7 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {172,
  "Toggle layer 7",
  "Makes layer 7 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {173,
  "Select layer 8",
  "Makes the layer 8 visible and",
  "set it as the active one, where",
  "you can draw.",
  true,
  0,
  0},
  {174,
  "Toggle layer 8",
  "Makes layer 8 visible or invisible.",
  "If it's the current active layer,",
  "toggle all other layers instead.",
  true,
  0,
  0},
  {175,
  "Add a layer",
  "Adds a new layer on top of the",
  "active one. The new layer is filled",
  "with transparent color.",
  true,
  KEY_INSERT|GFX2_MOD_ALT, // Alt + Insert
  0},
  {209,
  "Duplicate layer",
  "Adds a new layer on top of the",
  "active one. The new layer is a copy",
  "of the current one.",
  true,
  0, // No shortcut
  0},
  {176,
  "Delete a layer",
  "Delete the current layer.",
  "You can't delete the last",
  "layer.",
  true,
  KEY_DELETE|GFX2_MOD_ALT, // Alt + Delete
  0},
  {177,
  "Merge a layer",
  "Merges the current layer with",
  "the one directly below it.",
  "",
  true,
  KEY_END|GFX2_MOD_ALT, // Alt + End
  0},
  {178,
  "Swap layer (up)",
  "Moves the current layer one position",
  "up the stack. No effect if already",
  "on top.",
  true,
  KEY_PAGEUP|GFX2_MOD_ALT, // Alt + PageUp
  0},
  {179,
  "Swap layer (down)",
  "Moves the current layer one position",
  "down the stack. No effect if already",
  "on bottom.",
  true,
  KEY_PAGEDOWN|GFX2_MOD_ALT, // Alt + PageDown
  0},
  {180,
  "Layers menu",
  "Opens a window with options related",
  "to layers and image transparency.",
  "",
  true,
  KEY_HOME|GFX2_MOD_ALT, // Alt + Home
  0},
  {181,
  "Brush factory",
  "Opens a window where you can run a",
  "Lua script.",
  "",
  true,
  0, // No shortcut
  0},
  {182,
  "Repeat script",
  "Re-run the last script selected",
  "in the Brush factory window.",
  "",
  true,
  0, // No shortcut
  0},
  {183,
  "Double brush size",
  "Resizes the current user brush",
  "by doubling width and height.",
  "",
  true,
  KEY_h|GFX2_MOD_SHIFT, // Shift+H
  0},
  {184,
  "Double brush width",
  "Resizes the current user brush",
  "by doubling its width.",
  "",
  true,
  KEY_x|GFX2_MOD_SHIFT, // Shift+X
  0},
  {185,
  "Double brush height",
  "Resizes the current user brush",
  "by doubling its height.",
  "",
  true,
  KEY_y|GFX2_MOD_SHIFT, // Shift+Y
  0},
  {186,
  "Halve brush size",
  "Resizes the current user brush",
  "by halving its width and height",
  "",
  true,
  KEY_h, // H
  0},
  {187,
  "Run script #1",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {188,
  "Run script #2",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {189,
  "Run script #3",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {190,
  "Run script #4",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {191,
  "Run script #5",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {192,
  "Run script #6",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {193,
  "Run script #7",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {194,
  "Run script #8",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {195,
  "Run script #9",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {196,
  "Run script #10",
  "Runs a recorded Lua script.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {197,
  "Toggle color cycling",
  "Activates or desactivates color",
  "cycling, if the current image has",
  "cycling colors. (See gradient menu)",
  true,
  KEY_BACKQUOTE|GFX2_MOD_CTRL, // Ctrl + `~
  0},
  {198,
  "Format checker",
  "Performs a format check on the",
  "current image.",
  "",
  true,
  0,
  0},
  {199,
  "Format checker menu",
  "Allows you to setup the checks",
  "performed by the format checker.",
  "",
  true,
  0,
  0},
  {200,
  "Set frame time",
  "Opens a window where you",
  "can set the current animation",
  "frame duration.",
  true,
  0, // No shortcut
  0},
  {201,
  "Go to first frame",
  "Edits the first frame of",
  "an animation",
  "",
  true,
  0, // No shortcut
  0},
  {202,
  "Go to last frame",
  "Edits the last frame of",
  "an animation",
  "",
  true,
  0, // No shortcut
  0},
  {203,
  "Go to previous frame",
  "Edits the previous frame of",
  "an animation",
  "",
  true,
  0, // No shortcut
  0},
  {204,
  "Go to next frame",
  "Edits the next frame of",
  "an animation",
  "",
  true,
  0, // No shortcut
  0},
  {205,
  "Preview animation",
  "Runs the current animation.",
  "",
  "",
  true,
  0, // No shortcut
  0},
  {208,
  "Pan view",
  "While this key is being held,",
  "click and drag the mouse to",
  "pan the view.",
  true,
#ifdef GCWZERO
  KEY_F1, // Space
#else
  KEY_SPACE, // Space
#endif
  0},
  {209,
  "Zoom in more",
  "Increase magnifying factor more.",
  "",
  "",
  true,
  KEY_KP_PLUS|GFX2_MOD_SHIFT, // Shift++
  KEY_MOUSEWHEELUP|GFX2_MOD_SHIFT},
  {210,
  "Zoom out more",
  "Increase magnifying factor more.",
  "",
  "",
  true,
  KEY_KP_MINUS|GFX2_MOD_SHIFT, // Shift+-
  KEY_MOUSEWHEELDOWN|GFX2_MOD_SHIFT},
};

word Ordering[NB_SHORTCUTS]=
{
  SPECIAL_SCROLL_UP,                // Scroll up
  SPECIAL_SCROLL_DOWN,              // Scroll down
  SPECIAL_SCROLL_LEFT,              // Scroll left
  SPECIAL_SCROLL_RIGHT,             // Scroll right
  SPECIAL_SCROLL_UP_FAST,           // Scroll up faster
  SPECIAL_SCROLL_DOWN_FAST,         // Scroll down faster
  SPECIAL_SCROLL_LEFT_FAST,         // Scroll left faster
  SPECIAL_SCROLL_RIGHT_FAST,        // Scroll right faster
  SPECIAL_SCROLL_UP_SLOW,           // Scroll up slower
  SPECIAL_SCROLL_DOWN_SLOW,         // Scroll down slower
  SPECIAL_SCROLL_LEFT_SLOW,         // Scroll left slower
  SPECIAL_SCROLL_RIGHT_SLOW,        // Scroll right slower
  SPECIAL_MOUSE_UP,                 // Emulate mouse up
  SPECIAL_MOUSE_DOWN,               // Emulate mouse down
  SPECIAL_MOUSE_LEFT,               // Emulate mouse left
  SPECIAL_MOUSE_RIGHT,              // Emulate mouse right
  SPECIAL_CLICK_LEFT,               // Emulate mouse click left
  SPECIAL_CLICK_RIGHT,              // Emulate mouse click right
  0x100+BUTTON_HIDE,                // Show / Hide menus
  SPECIAL_SHOW_HIDE_CURSOR,         // Show / Hide cursor
  SPECIAL_DOT_PAINTBRUSH,           // Paintbrush = "."
  0x100+BUTTON_PAINTBRUSHES,        // Paintbrush choice
  0x200+BUTTON_PAINTBRUSHES,        // Monochrome brush
  0x100+BUTTON_DRAW,                // Freehand drawing
  0x200+BUTTON_DRAW,                // Switch freehand drawing mode
  SPECIAL_CONTINUOUS_DRAW,          // Continuous freehand drawing
  0x100+BUTTON_LINES,               // Line
  0x200+BUTTON_LINES,               // Knotted lines
  0x100+BUTTON_AIRBRUSH,            // Spray
  0x200+BUTTON_AIRBRUSH,            // Spray menu
  0x100+BUTTON_FLOODFILL,           // Floodfill
  0x200+BUTTON_FLOODFILL,           // Replace color
  0x100+BUTTON_CURVES,              // Bézier's curves
  0x200+BUTTON_CURVES,              // Bézier's curve with 3 or 4 points
  0x100+BUTTON_RECTANGLES,          // Empty rectangle
  0x100+BUTTON_FILLRECT,            // Filled rectangle
  0x100+BUTTON_CIRCLES,             // Empty circle
  0x200+BUTTON_CIRCLES,             // Empty ellipse
  0x100+BUTTON_FILLCIRC,            // Filled circle
  0x200+BUTTON_FILLCIRC,            // Filled ellipse
  0x100+BUTTON_POLYGONS,            // Empty polygon
  0x200+BUTTON_POLYGONS,            // Empty polyform
  0x100+BUTTON_POLYFILL,            // Polyfill
  0x200+BUTTON_POLYFILL,            // Filled polyform
  0x100+BUTTON_GRADRECT,            // Gradient rectangle
  0x200+BUTTON_GRADRECT,            // Gradation menu
  0x100+BUTTON_SPHERES,             // Spheres
  0x200+BUTTON_SPHERES,             // Gradient ellipses
  0x100+BUTTON_ADJUST,              // Adjust picture
  0x200+BUTTON_ADJUST,              // Flip picture menu
  0x100+BUTTON_EFFECTS,             // Menu des effets
  SPECIAL_SHADE_MODE,               // Shade mode
  SPECIAL_SHADE_MENU,               // Shade menu
  SPECIAL_QUICK_SHADE_MODE,         // Quick-shade mode
  SPECIAL_QUICK_SHADE_MENU,         // Quick-shade menu
  SPECIAL_STENCIL_MODE,             // Stencil mode
  SPECIAL_STENCIL_MENU,             // Stencil menu
  SPECIAL_MASK_MODE,                // Mask mode
  SPECIAL_MASK_MENU,                // Mask menu
  SPECIAL_GRID_MODE,                // Grid mode
  SPECIAL_GRID_MENU,                // Grid menu
  SPECIAL_SIEVE_MODE,               // Sieve mode
  SPECIAL_SIEVE_MENU,               // Sieve menu
  SPECIAL_INVERT_SIEVE,             // Inverser la trame du mode Sieve
  SPECIAL_COLORIZE_MODE,            // Colorize mode
  SPECIAL_COLORIZE_MENU,            // Colorize menu
  SPECIAL_SMOOTH_MODE,              // Smooth mode
  SPECIAL_SMOOTH_MENU,              // Smooth menu
  SPECIAL_SMEAR_MODE,               // Smear mode
  SPECIAL_TILING_MODE,              // Tiling mode
  SPECIAL_TILING_MENU,              // Tiling menu
  SPECIAL_TILEMAP_MODE,             // Tilemap mode
  SPECIAL_TILEMAP_MENU,             // Tilemap menu
  0x100+BUTTON_BRUSH,               // Pick brush
  0x100+BUTTON_POLYBRUSH,           // Pick polyform brush
  0x200+BUTTON_BRUSH,               // Restore brush
  SPECIAL_FLIP_X,                   // Flip X
  SPECIAL_FLIP_Y,                   // Flip Y
  SPECIAL_ROTATE_90,                // 90° brush rotation
  SPECIAL_ROTATE_180,               // 180° brush rotation
  SPECIAL_STRETCH,                  // Stretch brush
  SPECIAL_DISTORT,                  // Distort brush
  SPECIAL_OUTLINE,                  // Outline brush
  SPECIAL_NIBBLE,                   // Nibble brush
  SPECIAL_GET_BRUSH_COLORS,         // Get colors from brush
  SPECIAL_RECOLORIZE_BRUSH,         // Recolorize brush
  SPECIAL_ROTATE_ANY_ANGLE,         // Rotate brush by any angle
  0x100+BUTTON_COLORPICKER,         // Pipette
  0x200+BUTTON_COLORPICKER,         // Swap fore/back color
  0x100+BUTTON_MAGNIFIER,           // Magnifier mode
  0x200+BUTTON_MAGNIFIER,           // Zoom factor menu
  SPECIAL_ZOOM_IN,                  // Zoom in
  SPECIAL_ZOOM_OUT,                 // Zoom out
  0x100+BUTTON_BRUSH_EFFECTS,       // Brush effects menu
  0x100+BUTTON_TEXT,                // Text
  0x100+BUTTON_RESOL,               // Resolution menu
  0x200+BUTTON_RESOL,               // Safety resolution
  0x100+BUTTON_HELP,                // Help & credits
  0x200+BUTTON_HELP,                // Statistics
  0x100+BUTTON_PAGE,                // Go to spare page
  0x200+BUTTON_PAGE,                // Copy to spare page
  0x100+BUTTON_SAVE,                // Save as
  0x200+BUTTON_SAVE,                // Save
  0x100+BUTTON_LOAD,                // Load
  0x200+BUTTON_LOAD,                // Re-load
  SPECIAL_SAVE_BRUSH,               // Save brush
  SPECIAL_LOAD_BRUSH,               // Load brush
  0x100+BUTTON_SETTINGS,            // Settings
  0x100+BUTTON_UNDO,                // Undo
  0x200+BUTTON_UNDO,                // Redo
  0x100+BUTTON_KILL,                // Kill
  0x100+BUTTON_CLEAR,               // Clear
  0x200+BUTTON_CLEAR,               // Clear with backcolor
  0x100+BUTTON_QUIT,                // Quit
  0x100+BUTTON_PALETTE,             // Palette menu
  0x200+BUTTON_PALETTE,             // Palette menu secondaire
  SPECIAL_EXCLUDE_COLORS_MENU,      // Exclude colors menu
  0x100+BUTTON_PAL_LEFT,            // Scroll palette left
  0x100+BUTTON_PAL_RIGHT,           // Scroll palette right
  0x200+BUTTON_PAL_LEFT,            // Scroll palette left faster
  0x200+BUTTON_PAL_RIGHT,           // Scroll palette right faster
  SPECIAL_CENTER_ATTACHMENT,        // Center brush attachment
  SPECIAL_TOP_LEFT_ATTACHMENT,      // Top-left brush attachment
  SPECIAL_TOP_RIGHT_ATTACHMENT,     // Top-right brush attachment
  SPECIAL_BOTTOM_LEFT_ATTACHMENT,   // Bottom-left brush attachment
  SPECIAL_BOTTOM_RIGHT_ATTACHMENT,  // Bottom right brush attachment
  SPECIAL_NEXT_FORECOLOR,           // Next foreground color
  SPECIAL_PREVIOUS_FORECOLOR,       // Previous foreground color
  SPECIAL_NEXT_BACKCOLOR,           // Next background color
  SPECIAL_PREVIOUS_BACKCOLOR,       // Previous background color
  SPECIAL_NEXT_USER_FORECOLOR,      // Next user-defined foreground color
  SPECIAL_PREVIOUS_USER_FORECOLOR,  // Previous user-defined foreground color
  SPECIAL_NEXT_USER_BACKCOLOR,      // Next user-defined background color
  SPECIAL_PREVIOUS_USER_BACKCOLOR,  // Previous user-defined background color
  SPECIAL_SMALLER_PAINTBRUSH,       // Sets paintbrush size: smaller
  SPECIAL_BIGGER_PAINTBRUSH,        // Sets paintbrush size: bigger
  SPECIAL_EFFECTS_OFF,              // Turns off all effects
  SPECIAL_TRANSPARENCY_1,           // Sets transparency level 10%
  SPECIAL_TRANSPARENCY_2,           // Sets transparency level 20%
  SPECIAL_TRANSPARENCY_3,           // Sets transparency level 30%
  SPECIAL_TRANSPARENCY_4,           // Sets transparency level 40%
  SPECIAL_TRANSPARENCY_5,           // Sets transparency level 50%
  SPECIAL_TRANSPARENCY_6,           // Sets transparency level 60%
  SPECIAL_TRANSPARENCY_7,           // Sets transparency level 70%
  SPECIAL_TRANSPARENCY_8,           // Sets transparency level 80%
  SPECIAL_TRANSPARENCY_9,           // Sets transparency level 90%
  SPECIAL_TRANSPARENCY_0,           // Sets transparency level 00%
  SPECIAL_ZOOM_1,                   /**< Sets zoom factor to 1:1 (no magnification) */
  SPECIAL_ZOOM_2,                   /**< Sets zoom factor to 2:1 */
  SPECIAL_ZOOM_3,                   /**< Sets zoom factor to 3:1 */
  SPECIAL_ZOOM_4,                   /**< Sets zoom factor to 4:1 */
  SPECIAL_ZOOM_5,                   /**< Sets zoom factor to 5:1 */
  SPECIAL_ZOOM_6,                   /**< Sets zoom factor to 6:1 */
  SPECIAL_ZOOM_8,                   /**< Sets zoom factor to 8:1 */
  SPECIAL_ZOOM_10,                  /**< Sets zoom factor to 10:1 */
  SPECIAL_ZOOM_12,                  /**< Sets zoom factor to 12:1 */
  SPECIAL_ZOOM_14,                  /**< Sets zoom factor to 14:1 */
  SPECIAL_ZOOM_16,                  /**< Sets zoom factor to 16:1 */
  SPECIAL_ZOOM_18,                  /**< Sets zoom factor to 18:1 */
  SPECIAL_ZOOM_20,                  /**< Sets zoom factor to 20:1 */
  SPECIAL_SHOW_GRID,
  SPECIAL_LAYER1_SELECT,
  SPECIAL_LAYER1_TOGGLE,
  SPECIAL_LAYER2_SELECT,
  SPECIAL_LAYER2_TOGGLE,
  SPECIAL_LAYER3_SELECT,
  SPECIAL_LAYER3_TOGGLE,
  SPECIAL_LAYER4_SELECT,
  SPECIAL_LAYER4_TOGGLE,
  SPECIAL_LAYER5_SELECT,
  SPECIAL_LAYER5_TOGGLE,
  SPECIAL_LAYER6_SELECT,
  SPECIAL_LAYER6_TOGGLE,
  SPECIAL_LAYER7_SELECT,
  SPECIAL_LAYER7_TOGGLE,
  SPECIAL_LAYER8_SELECT,
  SPECIAL_LAYER8_TOGGLE,
  0x100+BUTTON_LAYER_ADD,
  0x200+BUTTON_LAYER_ADD,
  0x100+BUTTON_LAYER_REMOVE,
  0x100+BUTTON_LAYER_MERGE,
  0x100+BUTTON_LAYER_UP,
  0x100+BUTTON_LAYER_DOWN,
  0x100+BUTTON_LAYER_MENU,
  0x200+BUTTON_BRUSH_EFFECTS,
  SPECIAL_REPEAT_SCRIPT,
  SPECIAL_BRUSH_DOUBLE,
  SPECIAL_BRUSH_DOUBLE_WIDTH,
  SPECIAL_BRUSH_DOUBLE_HEIGHT,
  SPECIAL_BRUSH_HALVE,
  SPECIAL_RUN_SCRIPT_1,
  SPECIAL_RUN_SCRIPT_2,
  SPECIAL_RUN_SCRIPT_3,
  SPECIAL_RUN_SCRIPT_4,
  SPECIAL_RUN_SCRIPT_5,
  SPECIAL_RUN_SCRIPT_6,
  SPECIAL_RUN_SCRIPT_7,
  SPECIAL_RUN_SCRIPT_8,
  SPECIAL_RUN_SCRIPT_9,
  SPECIAL_RUN_SCRIPT_10,
  SPECIAL_CYCLE_MODE,
  SPECIAL_FORMAT_CHECKER,       /// unused at this time
  SPECIAL_FORMAT_CHECKER_MENU,  /// unused at this time
  0x100+BUTTON_ANIM_TIME,
  0x100+BUTTON_ANIM_FIRST_FRAME,
  0x100+BUTTON_ANIM_LAST_FRAME,
  0x100+BUTTON_ANIM_PREV_FRAME,
  0x100+BUTTON_ANIM_NEXT_FRAME,
  0x100+BUTTON_ANIM_PLAY, // Unused at this time
  SPECIAL_HOLD_PAN,
  SPECIAL_ZOOM_IN_MORE,             // Zoom in more
  SPECIAL_ZOOM_OUT_MORE,            // Zoom out more
};
