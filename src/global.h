/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2014 Sergii Pylypenko
    Copyright 2009 Franck Charlet
    Copyright 2009 Yves Rizoud
    Copyright 2007 Adrien Destugues
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

//////////////////////////////////////////////////////////////////////////////
///@file global.h
/// This file contains all global variables.
/// They are prefixed by ::GFX2_GLOBAL so they are extern when needed.
//////////////////////////////////////////////////////////////////////////////
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "struct.h"

// MAIN declares the variables,
// other files only have an extern definition.
#ifdef GLOBAL_VARIABLES
  /// Magic prefix to make all declarations extern, except when included by main.c.
  #define GFX2_GLOBAL
#else
  #define GFX2_GLOBAL extern
#endif

// -- CONFIGURATION variables

/// Current configuration.
GFX2_GLOBAL T_Config Config;

/// Array of special keys.
GFX2_GLOBAL word Config_Key[NB_SPECIAL_SHORTCUTS][2];

/// A video mode (resolution) usable by Grafx2.
typedef struct
{
  short  Width;      ///< Screen width
  short  Height;     ///< Screen height
  byte   Mode;       ///< Unused (used to be Mode-X, SVGA, etc)
  word   Fullscreen; ///< 0 for window, 1 for fullscreen
  byte   State;      ///< How good is the mode supported. 0:Good (white) 1:OK (light) 2:So-so (dark) 4:User-disabled (black); +128 => System doesn't support it at all.
} T_Video_mode;

/// Array of all video modes supported by your platform. Actually filled up to ::Nb_video_modes, excluded.
GFX2_GLOBAL T_Video_mode Video_mode[MAX_VIDEO_MODES];

/// Actual number of video modes in ::Video_mode.
GFX2_GLOBAL int  Nb_video_modes;

// -- Menu colors

GFX2_GLOBAL byte MC_Black; ///< Index of color to use as "black" in the GUI menus.
GFX2_GLOBAL byte MC_Dark;  ///< Index of color to use as "dark grey" in the GUI menus.
GFX2_GLOBAL byte MC_Light; ///< Index of color to use as "light grey" in the GUI menus.
GFX2_GLOBAL byte MC_White; ///< Index of color to use as "white" in the GUI menus.
GFX2_GLOBAL byte MC_Trans; ///< Index of color to use as "transparent" while loading the GUI file.

GFX2_GLOBAL byte MC_OnBlack; ///< Index of color immediately lighter than "black" in the GUI menus.
GFX2_GLOBAL byte MC_Window; ///< Index of color to use as window background in the GUI menus.
GFX2_GLOBAL byte MC_Lighter; ///< Index of color lighter than window in the GUI menus.
GFX2_GLOBAL byte MC_Darker; ///< Index of color darker than window in the GUI menus.


// Input state
GFX2_GLOBAL word Mouse_X; ///< Current mouse cursor position.
GFX2_GLOBAL word Mouse_Y; ///< Current mouse cursor position.
GFX2_GLOBAL byte Mouse_K; ///< Current mouse buttons state. Bitfield: 1 for RMB, 2 for LMB.
GFX2_GLOBAL byte Keyboard_click_allowed; ///< Set to 0 when you edit a textfield so you can use space without exiting it

/// Helper macro to take only one button when both are pressed (LMB has priority)
#define Mouse_K_unique (Mouse_K==0?0:(Mouse_K&1?1:(Mouse_K&2?2:0)))

/// Last key pressed, 0 if none. Set by the latest call to ::Get_input()
GFX2_GLOBAL dword Key;

///
/// Last character typed, converted to ANSI character set (Windows-1252).
/// This is mostly used when the user enters text (filename, etc).
GFX2_GLOBAL dword Key_ANSI;

GFX2_GLOBAL dword Key_UNICODE;

/// from SDL_TextInputEvent
#if defined(USE_SDL2)
GFX2_GLOBAL char Key_Text[32];
#endif

// Keyboard modifiers

/// Key modifier for SHIFT key. Used as mask in ::Key, for example.
#define GFX2_MOD_SHIFT 0x1000
/// Key modifier for CONTROL key. Used as mask in ::Key, for example.
#define GFX2_MOD_CTRL  0x2000
/// Key modifier for ALT key. Used as mask in ::Key, for example.
#define GFX2_MOD_ALT   0x4000
/// Key modifier for META key. Used as mask in ::Key, for example.
#define GFX2_MOD_META  0x8000

/// Boolean set to true when the OS/window manager requests the application to close. ie: [X] button
GFX2_GLOBAL byte Quit_is_required;

///
/// This boolean is true when the current operation allows changing the
/// foreground or background color.
GFX2_GLOBAL byte Allow_color_change_during_operation;

// -- Mouse cursor data

/// Current mouse cursor. Value is in enum ::CURSOR_SHAPES
GFX2_GLOBAL byte Cursor_shape;
/// Backup of ::Cursor_shape, used while a window is open (and needs a different cursor)
GFX2_GLOBAL byte Cursor_shape_before_window;
/// Boolean, means the cursor should not be drawn. It's togglable by the user.
GFX2_GLOBAL byte Cursor_hidden;
/// Boolean, means the cursor is currently hovering over a menu GUI element.
GFX2_GLOBAL byte Cursor_in_menu;
/// Boolean, means the cursor was hovering over a menu GUI element.
GFX2_GLOBAL byte Cursor_in_menu_previous;
/// Storage for the graphics under the mouse cursor. Used by ::Hide_cursor and ::Display_cursor
GFX2_GLOBAL byte Cursor_background[CURSOR_SPRITE_HEIGHT][CURSOR_SPRITE_WIDTH];

// -- Paintbrush data

/// Active paintbrush. It's an index in enum ::PAINTBRUSH_SHAPES
GFX2_GLOBAL byte  Paintbrush_shape;
/// Backup of ::Paintbrush_shape, before fill operation
GFX2_GLOBAL byte  Paintbrush_shape_before_fill;
/// Backup of ::Paintbrush_shape, before color picker operation
GFX2_GLOBAL byte  Paintbrush_shape_before_colorpicker;
/// Backup of ::Paintbrush_shape, before lasso operation
GFX2_GLOBAL byte  Paintbrush_shape_before_lasso;
/// Boolean, true when the preview paintbrush shouldn't be drawn.
GFX2_GLOBAL byte  Paintbrush_hidden;
/// Cordinate of the preview paintbrush in image space.
GFX2_GLOBAL short Paintbrush_X;
/// Cordinate of the preview paintbrush in image space.
GFX2_GLOBAL short Paintbrush_Y;
/// Pixel data of the current brush
GFX2_GLOBAL byte * Paintbrush_sprite;
/// Current paintbrush's width
GFX2_GLOBAL short Paintbrush_width;
/// Current paintbrush's height
GFX2_GLOBAL short Paintbrush_height;
/// Position of current paintbrush's handle
GFX2_GLOBAL short Paintbrush_offset_X;
/// Position of current paintbrush's handle
GFX2_GLOBAL short Paintbrush_offset_Y;

// -- Graphic commands

/// On the screen, draw a point.
GFX2_GLOBAL Func_pixel Pixel;
/// Test a pixel color from screen.
GFX2_GLOBAL Func_read Read_pixel;
/// Redraw all screen, without overwriting the menu.
GFX2_GLOBAL Func_display Display_screen;
/// Draw a rectangle on screen.
GFX2_GLOBAL Func_block Block;
/// Draw a point from the image to screen (no zoom).
GFX2_GLOBAL Func_pixel Pixel_preview_normal;
/// Draw a point from the image to screen (magnified part).
GFX2_GLOBAL Func_pixel Pixel_preview_magnifier;
/// Draw a point from the image to screen (zoomed if needed).
GFX2_GLOBAL Func_pixel Pixel_preview;
/// Draw a horizontal XOR line on screen.
GFX2_GLOBAL Func_line_XOR Horizontal_XOR_line;
/// Draw a vertical XOR line on screen.
GFX2_GLOBAL Func_line_XOR Vertical_XOR_line;
/// Display part of the brush on screen, color mode.
GFX2_GLOBAL Func_display_brush_color Display_brush_color;
/// Display part of the brush on screen, monochrome mode.
GFX2_GLOBAL Func_display_brush_mono  Display_brush_mono;
/// Clear the brush currently displayed on screen, redrawing the image instead.
GFX2_GLOBAL Func_display_brush_color Clear_brush;
/// Remap part of the screen after the menu colors have changed.
GFX2_GLOBAL Func_remap     Remap_screen;
/// Draw a line on screen.
GFX2_GLOBAL Func_procsline Display_line;
/// Draw a line on screen, without doubling it if using wide pixels. (to be used when the line is already doubled in the input buffer)
GFX2_GLOBAL Func_procsline Display_line_fast;
/// Read a line of pixels from screen.
GFX2_GLOBAL Func_procsline Read_line;
/// Redraw all magnified part on screen, without overwriting the menu.
GFX2_GLOBAL Func_display_zoom Display_zoomed_screen;
/// Display part of the brush on the magnified part of screen, color mode.
GFX2_GLOBAL Func_display_brush_color_zoom Display_brush_color_zoom;
/// Display part of the brush on the magnified part of screen, monochrome mode.
GFX2_GLOBAL Func_display_brush_mono_zoom  Display_brush_mono_zoom;
/// Clear the brush currently displayed on the magnified part of screen, redrawing the image instead.
GFX2_GLOBAL Func_display_brush_color_zoom Clear_brush_scaled;
/// Draw an arbitrary brush on screen (not the current brush)
GFX2_GLOBAL Func_draw_brush Display_brush;

// -- Screen data

/// Requested window width. This is set when the user resizes the window.
GFX2_GLOBAL int   Resize_width;
/// Requested window height. This is set when the user resizes the window.
GFX2_GLOBAL int   Resize_height;
/// Current window state (standard, minimized, maximized)
GFX2_GLOBAL enum GFX2_WINDOW_STATES Window_state;
/// Default window width (not maximized)
GFX2_GLOBAL int   Default_window_width;
/// Default window height (not maximized)
GFX2_GLOBAL int   Default_window_height;
/// Current video mode. Index in ::Video_mode
GFX2_GLOBAL int   Current_resolution;
/// After loading an image, this holds the "original screen width", if the file format supported it.
GFX2_GLOBAL short Original_screen_X;
/// After loading an image, this holds the "original screen height", if the file format supported it.
GFX2_GLOBAL short Original_screen_Y;
///
/// Current screen (or window) width, in pixels.
/// Note that this takes ::Pixel_ratio into account.
GFX2_GLOBAL short Screen_width;
///
/// Current screen (or window) height, in pixels.
/// Note that this takes ::Pixel_ratio into account.
GFX2_GLOBAL short Screen_height;
/// Coordinate (in image space) of the topmost visible pixel.
GFX2_GLOBAL short Limit_top;
///
/// Coordinate (in image space) of the lowest visible pixel.
/// This can be larger than the image height, if the screen is bigger than image.
GFX2_GLOBAL short Limit_bottom;
/// Coordinate (in image space) of the leftmost visible pixel.
GFX2_GLOBAL short Limit_left;
///
/// Coordinate (in image space) of the rightmost visible pixel.
/// This can be larger than the image width, if the screen is bigger than image.
GFX2_GLOBAL short Limit_right;
///
/// Coordinate (in image space) of the lowest visible pixel, limited by the
/// image height. Compare with ::Limit_bottom, which is not clipped.
GFX2_GLOBAL short Limit_visible_bottom;
///
/// Coordinate (in image space) of the rightmost visible pixel, limited by the
/// image width. Compare with ::Limit_right, which is not clipped.
GFX2_GLOBAL short Limit_visible_right;

/// Coordinate (in image space) of the pixel at the top of the magnified view.
GFX2_GLOBAL short Limit_top_zoom;
///
/// Coordinate (in image space) of the pixel at the bottom of the magnified view.
/// This can be larger than the image height, if the screen is bigger than image.
GFX2_GLOBAL short Limit_bottom_zoom;
/// Coordinate (in image space) of the pixel at the left of the magnified view.
GFX2_GLOBAL short Limit_left_zoom;
///
/// Coordinate (in image space) of the pixel at the right of the magnified view.
/// This can be larger than the image width, if the screen is bigger than image.
GFX2_GLOBAL short Limit_right_zoom;
///
/// Coordinate (in image space) of the lowest visible pixel, limited by the
/// image height. Compare with ::Limit_bottom, which is not clipped.
GFX2_GLOBAL short Limit_visible_bottom_zoom;
/// Coordinate (in image space) of the rightmost visible pixel.
/// This can be larger than the image width, if the screen is bigger than image.
GFX2_GLOBAL short Limit_visible_right_zoom;

/// Buffer of pixels, used when drawing something to screen.
GFX2_GLOBAL byte * Horizontal_line_buffer;

/// Current pixel ratio. Index in enum ::PIXEL_RATIO
GFX2_GLOBAL int Pixel_ratio;
/// Current width of pixels, according to ::Pixel_ratio
GFX2_GLOBAL int Pixel_width;
/// Current height of pixels, according to ::Pixel_ratio
GFX2_GLOBAL int Pixel_height;


/// Current image data
GFX2_GLOBAL T_Document Main;

/// Pointer to the pixel data of the main image
GFX2_GLOBAL byte *    Main_screen;
/// Lookup table for XOR effects, pointing each color to the most different one
GFX2_GLOBAL byte xor_lut[256];

/// Spare page data
GFX2_GLOBAL T_Document Spare;

// -- Image backups

/// Backup of the current screen, used during drawing when FX feedback is OFF.
GFX2_GLOBAL byte * Screen_backup;

// -- Palette load/Save selector

/// Fileselector settings
GFX2_GLOBAL T_Selector_settings Palette_selector;

// -- Brush data

/// Pixel data of the current brush (remapped).
GFX2_GLOBAL byte * Brush;
/// Pixel data of the current brush (before remap).
GFX2_GLOBAL byte * Brush_original_pixels;
/// Palette of the brush, from when it was grabbed.
GFX2_GLOBAL T_Palette Brush_original_palette;
/// Back_color used when the brush was grabbed
GFX2_GLOBAL byte Brush_original_back_color;
/// Color mapping from ::Brush_original_pixels to ::Brush
GFX2_GLOBAL byte Brush_colormap[256];
/// X coordinate of the brush's "hot spot". It is < ::Brush_width
GFX2_GLOBAL word Brush_offset_X;
/// Y coordinate of the brush's "hot spot". It is < ::Brush_height
GFX2_GLOBAL word Brush_offset_Y;
/// Width of the current brush.
GFX2_GLOBAL word Brush_width;
/// Height of the current brush.
GFX2_GLOBAL word Brush_height;
/// Name of the directory that holds the brush fil (after loading or saving it).
GFX2_GLOBAL char * Brush_file_directory;
/// Filename (without directory) of the brush (after loading or saving it).
GFX2_GLOBAL char * Brush_filename;
/// Filename (without directory) of the brush (after loading or saving it) unicode.
GFX2_GLOBAL word * Brush_filename_unicode;
/// File format of the brush. It's a value of enum ::FILE_FORMATS
GFX2_GLOBAL byte  Brush_fileformat;
/// Fileselector settings
GFX2_GLOBAL T_Selector_settings Brush_selector;
/// Indicator used for the "Rotate brush" operation.
GFX2_GLOBAL byte  Brush_rotation_center_is_defined;
/// Position of the brush's rotation center, in screen coordinates.
GFX2_GLOBAL short Brush_rotation_center_X;
/// Position of the brush's rotation center, in screen coordinates.
GFX2_GLOBAL short Brush_rotation_center_Y;

// -- Menu data (toolbox)

/// Boolean, true if the menu has to be displayed.
GFX2_GLOBAL byte  Menu_is_visible;
/// Height of the menu, when it's displayed
GFX2_GLOBAL word  Menu_height;
///
/// Y position (in screen coordinates) where the menu begins.
/// This is always either ::Screen_height (when menu is hidden) or (::Screen_height - ::Menu_height)
/// As a result, the drawing algoritm always draws the image from 0 to ::Menu_Y-1
GFX2_GLOBAL word  Menu_Y;
/// Y position of the status bar (in screen coordinates)
GFX2_GLOBAL word  Menu_status_Y;
/// Scaling factor for the menu and all GUI elements
GFX2_GLOBAL byte  Menu_factor_X;
/// Scaling factor for the menu and all GUI elements
GFX2_GLOBAL byte  Menu_factor_Y;
/// Size of a color cell in the menu's palette.
GFX2_GLOBAL word  Menu_palette_cell_width;

// -- Window data

/// Number of stacked windows currently displayed. 0 when no window is present.
GFX2_GLOBAL byte Windows_open;
/// Backup of ::Menu_is_visible, used to store it while a window is open.
GFX2_GLOBAL byte Menu_is_visible_before_window;
/// Backup of ::Menu_Y, used to store it while a window is open.
GFX2_GLOBAL word Menu_Y_before_window;
/// Backup of ::Paintbrush_hidden, used to store it while a window is open.
GFX2_GLOBAL byte Paintbrush_hidden_before_window;

/// The global stack of editor screens.
GFX2_GLOBAL T_Window Window_stack[8];

/// Position of the left border of the topmost window (in screen coordinates)
#define Window_pos_X Window_stack[Windows_open-1].Pos_X

/// Position of the top border of the topmost window (in screen coordinates)
#define Window_pos_Y Window_stack[Windows_open-1].Pos_Y

///
/// Width of the topmost window, in "window pixels"
/// (multiply by ::Menu_factor_X to get screen pixels)
#define Window_width Window_stack[Windows_open-1].Width

///
/// Height of the topmost window, in "window pixels"
/// (multiply by ::Menu_factor_Y to get screen pixels)
#define Window_height Window_stack[Windows_open-1].Height

/// Total number of buttons/controls in the topmost window.
#define Window_nb_buttons Window_stack[Windows_open-1].Nb_buttons

/// List of normal buttons in the topmost window.
#define Window_normal_button_list Window_stack[Windows_open-1].Normal_button_list

/// List of "palette" buttons in the topmost window.
#define Window_palette_button_list Window_stack[Windows_open-1].Palette_button_list

/// List of sliders (scrollers) in the topmost window.
#define Window_scroller_button_list Window_stack[Windows_open-1].Scroller_button_list

/// List of special buttons in the topmost window.
#define Window_special_button_list Window_stack[Windows_open-1].Special_button_list

/// List of dropdown buttons in the topmost window.
#define Window_dropdown_button_list Window_stack[Windows_open-1].Dropdown_button_list

/// List of list buttons in the topmost window.
#define Window_list_button_list Window_stack[Windows_open-1].List_button_list

///
/// The function ::Window_clicked_button() set this to ::LEFT_SIDE or ::RIGHT_SIDE
/// after a button is activated through left or right mouse click.
#define Window_attribute1 Window_stack[Windows_open-1].Attribute1

///
/// The function ::Window_clicked_button() set this to return extra information:
/// - When a scroller was clicked: the scroller position (0-n)
/// - When a palette was clicked: the color index (0-255)
/// - When a dropdown was used: the selected item's number T_Dropdown_choice::Number
#define Window_attribute2 Window_stack[Windows_open-1].Attribute2

#define Window_draggable Window_stack[Windows_open-1].Draggable

// -- Information about the different drawing modes (effects)

/// Current effecting function. When no effect is selected this is ::No_effect()
GFX2_GLOBAL Func_effect Effect_function;

///
/// Array of booleans, indicates which colors should never be picked by
/// ::Best_color()
GFX2_GLOBAL byte Exclude_color[256];

// -- Smear mode

/// Smear mode is activated
GFX2_GLOBAL byte  Smear_mode;
/// Boolean, indicates that a smear is in progress.
GFX2_GLOBAL byte  Smear_start;
/// Pointer to the sprite to use for smear; it contains pixels from the image.
GFX2_GLOBAL byte * Smear_brush;
/// Width of the ::Smear_brush
GFX2_GLOBAL word  Smear_brush_width;
/// Height of the ::Smear_brush
GFX2_GLOBAL word  Smear_brush_height;
/// Limits of the smear.
GFX2_GLOBAL short Smear_min_X;
/// Limits of the smear.
GFX2_GLOBAL short Smear_max_X;
/// Limits of the smear.
GFX2_GLOBAL short Smear_min_Y;
/// Limits of the smear.
GFX2_GLOBAL short Smear_max_Y;

// -- Shade mode
/// List of the shade tables
GFX2_GLOBAL T_Shade        Shade_list[8];
/// Shade currently selected (index in ::Shade_list)
GFX2_GLOBAL byte           Shade_current;
/// Conversion table in use
GFX2_GLOBAL byte *         Shade_table;
/// Conversion table for a left click
GFX2_GLOBAL byte           Shade_table_left[256];
/// Conversion table for a right click
GFX2_GLOBAL byte           Shade_table_right[256];
/// Boolean, true when the shade mode is active.
GFX2_GLOBAL byte           Shade_mode;

/// Boolean, true when the quick-shade mode is active.
GFX2_GLOBAL byte           Quick_shade_mode;
/// Size of the step, in Quick-shade mode. It's the number of colors to "jump".
GFX2_GLOBAL byte           Quick_shade_step;
/// Determines how colors should loop in Quick-shade more. Value in enum ::SHADE_MODES
GFX2_GLOBAL byte           Quick_shade_loop;

// -- Stencil mode

/// Boolean, true when stencil mode is active.
GFX2_GLOBAL byte Stencil_mode;
/// Array of the protected colors by Stencil mode.
GFX2_GLOBAL byte Stencil[256];

// -- Grid mode

/// Boolean, true when the Grid mode is active.
GFX2_GLOBAL byte  Snap_mode;
/// Boolean, true when the Grid is displayed in zoomed view.
GFX2_GLOBAL byte  Show_grid;
/// Width of the grid in Grid mode.
GFX2_GLOBAL word Snap_width;
/// Height of the grid in Grid mode.
GFX2_GLOBAL word Snap_height;
/// Position of the starting pixel, in Grid mode.
GFX2_GLOBAL word Snap_offset_X;
/// Position of the starting pixel, in Grid mode.
GFX2_GLOBAL word Snap_offset_Y;

// -- Sieve mode

/// Boolean, true when the Sieve mode is active
GFX2_GLOBAL byte  Sieve_mode;
/// Sprite of the sieve pattern. It's actually an array of booleans.
GFX2_GLOBAL byte  Sieve[16][16];
/// Width of the sieve pattern, in Sieve mode.
GFX2_GLOBAL short Sieve_width;
/// Height of the sieve pattern, in Sieve mode.
GFX2_GLOBAL short Sieve_height;

// -- Colorize mode

/// Boolean, true when the Colorize mode is active.
GFX2_GLOBAL byte Colorize_mode;
/// % of opacity of Colorize mode (for translucency)
GFX2_GLOBAL byte Colorize_opacity;
/// Sets the colorization mode: 0 transparency, 1 additive, 2 substractive
GFX2_GLOBAL byte Colorize_current_mode;
///
/// Table of precomputed factors used by Colorize mode. It hold 0 to 255 when
/// opacity is 100%, 0 to 128 when opacity is 50%, etc.
// FIXME: This only caches a multiplication and a division. Maybe we should scrap it
GFX2_GLOBAL word Factors_table[256];
///
/// Table of precomputed factors used by Colorize mode. It hold 255 to 0 when
/// opacity is 100%, 128 to 0 when opacity is 50%, etc.
// FIXME: This only caches a multiplication, a division, a substraction. Maybe we should scrap it
GFX2_GLOBAL word Factors_inv_table[256];

// -- Smooth mode

/// Boolean, true when the Smooth mode is active
GFX2_GLOBAL byte Smooth_mode;
/// Matrix of "weights" used by the Smooth mode.
GFX2_GLOBAL byte Smooth_matrix[3][3];

// -- Tiling mode

/// Boolean, true when the Tiling mode is active
GFX2_GLOBAL byte  Tiling_mode;
/// Position of the starting pixel in Tiling mode.
GFX2_GLOBAL short Tiling_offset_X;
/// Position of the starting pixel in Tiling mode.
GFX2_GLOBAL short Tiling_offset_Y;

// -- Mask mode

/// Boolean, true when the Tiling mode is active
GFX2_GLOBAL byte Mask_mode;
/// Array of booleans. True if the indexed color is protected by the mask.
GFX2_GLOBAL byte Mask_table[256];

// -- 8 bit constraints mode

/// Selected constraint mode
GFX2_GLOBAL enum IMAGE_MODES Selected_Constraint_Mode;


// -- Magnifier data

#ifdef GLOBAL_VARIABLES
  const byte ZOOM_FACTOR[NB_ZOOM_FACTORS]={2,3,4,5,6,8,10,12,14,16,18,20, 24, 28, 32};
#else
/// Successive zoom factors, used by the Magnifier.
  extern const byte ZOOM_FACTOR[NB_ZOOM_FACTORS];
#endif

// -- Data for gradients

/// First color of the gradient.
GFX2_GLOBAL short Gradient_lower_bound;
/// Last color of the gradient
GFX2_GLOBAL short Gradient_upper_bound;
/// Boolean, true if the gradient should use colors in descending order
GFX2_GLOBAL int   Gradient_is_inverted;
/// Number of colors in the range ::Gradient_lower_bound to ::Gradient_upper_bound (included)
GFX2_GLOBAL long  Gradient_bounds_range;
/// Maximum value passed to the gradient function. The pixels assigned this value should use last gradient color.
GFX2_GLOBAL long  Gradient_total_range;
/// Amount of randomness to use in gradient (1-256+)
GFX2_GLOBAL long  Gradient_random_factor;
/// Gradient speed of cycling (0-64)
GFX2_GLOBAL byte  Gradient_speed;
/// Pointer to a gradient function, depending on the selected method.
GFX2_GLOBAL Func_gradient Gradient_function;
///
/// Pointer to the pixel-drawing function that gradients should use:
/// either ::Pixel (if the gradient must be drawn on menus only)
/// or ::Display_pixel (if the gradient must be drawn on the image)
GFX2_GLOBAL Func_pixel Gradient_pixel;
/// Index in ::T_Page::Gradients of the currently selected gradient.
GFX2_GLOBAL byte Current_gradient;
/// Boolean, true when the color cycling is active.
GFX2_GLOBAL byte Cycling_mode;

// -- Airbrush data

/// Mode to use in airbrush: 0 for multicolor, 1 for mono.
GFX2_GLOBAL byte  Airbrush_mode;
/// Diameter of the airbrush, in pixels.
GFX2_GLOBAL short Airbrush_size;
/// Delay between two airbrush "shots", in 1/100s
GFX2_GLOBAL byte  Airbrush_delay;
/// Number of pixels that are emitted by the airbrush, in mono mode.
GFX2_GLOBAL byte  Airbrush_mono_flow;
/// Number of pixels that are emitted by the airbrush for each color (multi mode)
GFX2_GLOBAL byte  Airbrush_multi_flow[256];

/// -- Misc data about the program

/// Boolean, set to true to exit the program.
GFX2_GLOBAL byte Quitting;
/// Name of the directory that was current when the program was run.
GFX2_GLOBAL char * Initial_directory;
/// Name of the directory that holds the program's (read-only) data: skins, icon, etc.
GFX2_GLOBAL char * Data_directory;
/// Name of the directory where grafx2 reads and writes configuration (gfx2.ini, gfx2.cfg)
GFX2_GLOBAL char * Config_directory;
/// Current foreground color for drawing.
GFX2_GLOBAL byte Fore_color;
/// Current background color for drawing.
GFX2_GLOBAL byte Back_color;
/// For the "Freehand draw" tool, this determines which variant is selected, from ::OPERATION_CONTINUOUS_DRAW to ::OPERATION_FILLED_CONTOUR
GFX2_GLOBAL byte Selected_freehand_mode;
/// For the Curve tool, this determines which variant is selected, either ::OPERATION_3_POINTS_CURVE or ::OPERATION_4_POINTS_CURVE
GFX2_GLOBAL byte Selected_curve_mode;
/// For the Line tool, this determines which variant is selected, either ::OPERATION_LINE, ::OPERATION_K_LINE or ::OPERATION_CENTERED_LINES
GFX2_GLOBAL byte Selected_line_mode;
/// Select which kind of Circle/ellipse mode is selected from 0 to 3 : CIRCLE_CTR/CIRCLE_CRN/ELLIPSE_CTR/ELLIPSE_CRN
GFX2_GLOBAL byte Selected_circle_ellipse_mode;
/// Determines which color appears in the first cell of the menu palette. Change this value to "scroll" the palette.
GFX2_GLOBAL byte First_color_in_palette;
/// Boolean, true if Grafx2 was run with a command-line argument to set a resolution on startup (overrides config)
GFX2_GLOBAL byte Resolution_in_command_line;

// - Graphic

/// Pointer to the font selected for menus.
GFX2_GLOBAL byte * Menu_font;

/// additional fonts for unicode characters
GFX2_GLOBAL T_Unicode_Font * Unicode_fonts;

/// Pointer to the current active skin.
GFX2_GLOBAL T_Gui_skin * Gfx;

/// Pointer to the current active skin.
GFX2_GLOBAL T_Paintbrush Paintbrush[NB_PAINTBRUSH_SPRITES];

// -- Help data

/// Index of the ::Help_section shown by the Help screen.
GFX2_GLOBAL byte Current_help_section;
/// Line number of the help viewer, in current ::Help_section. 0 for top, increase value to scroll down.
GFX2_GLOBAL word Help_position;

// -- Operation data

/// Index of the operation which was selected (ex: drawing rectangle) before the current interruption (ex: colorpicking).
GFX2_GLOBAL word Operation_before_interrupt;
/// Index of the current operation. This is the active "tool".
GFX2_GLOBAL word Current_operation;
///
/// This stack is used to memorize all parameters needed during the course of
/// an operation. For example when drawing a rectangle: color, starting
/// coordinates, ending coordinates.
GFX2_GLOBAL word Operation_stack[OPERATION_STACK_SIZE];
/// Number of parameters stored in ::Operation_stack (0=empty)
GFX2_GLOBAL byte Operation_stack_size;
/// Boolean, true if the operation (drawing) started in the magnified area.
GFX2_GLOBAL byte Operation_in_magnifier;
/// Last color hovered by the colorpicker. -1 if it wasn't over the image.
GFX2_GLOBAL short Colorpicker_color;
/// Position of the colorpicker tool, in image coordinates.
GFX2_GLOBAL short Colorpicker_X;
/// Position of the colorpicker tool, in image coordinates.
GFX2_GLOBAL short Colorpicker_Y;

GFX2_GLOBAL short * Polyfill_table_of_points;
GFX2_GLOBAL int Polyfill_number_of_points;

/// Brush container
GFX2_GLOBAL T_Brush_template Brush_container[BRUSH_CONTAINER_COLUMNS*BRUSH_CONTAINER_ROWS];

#ifdef GLOBAL_VARIABLES
  const byte CURSOR_FOR_OPERATION[NB_OPERATIONS]=
  {
    CURSOR_SHAPE_TARGET            , // Freehand continuous draw
    CURSOR_SHAPE_TARGET            , // Freehand discontinuous draw
    CURSOR_SHAPE_TARGET            , // Freehand point-by-point draw
    CURSOR_SHAPE_TARGET            , // Filled contour
    CURSOR_SHAPE_TARGET            , // Lines
    CURSOR_SHAPE_TARGET            , // Linked lines
    CURSOR_SHAPE_TARGET            , // Centered lines
    CURSOR_SHAPE_XOR_TARGET        , // Empty rectangle
    CURSOR_SHAPE_XOR_TARGET        , // Filled rectangle
    CURSOR_SHAPE_XOR_TARGET        , // Empty circle (center radius)
    CURSOR_SHAPE_XOR_TARGET        , // Empty circle (corners)
    CURSOR_SHAPE_XOR_TARGET        , // Empty ellipse (center radius)
    CURSOR_SHAPE_XOR_TARGET        , // Empty ellipse (corners)
    CURSOR_SHAPE_XOR_TARGET        , // Filled circle (center radius)
    CURSOR_SHAPE_XOR_TARGET        , // Filled circle (corners)
    CURSOR_SHAPE_XOR_TARGET        , // Filled ellipse (center radius)
    CURSOR_SHAPE_XOR_TARGET        , // Filled ellipse (corners)
    CURSOR_SHAPE_BUCKET            , // Fill
    CURSOR_SHAPE_BUCKET            , // Color replacer
    CURSOR_SHAPE_XOR_TARGET        , // Rectangular brush grabbing
    CURSOR_SHAPE_TARGET            , // Polygonal brush grabbing
    CURSOR_SHAPE_COLORPICKER       , // Colorpicker
    CURSOR_SHAPE_XOR_RECTANGLE     , // Position the magnify window
    CURSOR_SHAPE_TARGET            , // Curve with 3 control points
    CURSOR_SHAPE_TARGET            , // Curve with 4 control points
    CURSOR_SHAPE_TARGET            , // Airbrush
    CURSOR_SHAPE_TARGET            , // Polygon
    CURSOR_SHAPE_TARGET            , // Polyform
    CURSOR_SHAPE_TARGET            , // Filled polygon
    CURSOR_SHAPE_TARGET            , // Filled polyform
    CURSOR_SHAPE_MULTIDIRECTIONAL  , // Scroll image
    CURSOR_SHAPE_XOR_TARGET        , // Gradient-filled circle (center radius)
    CURSOR_SHAPE_XOR_TARGET        , // Gradient-filled circle (corners)
    CURSOR_SHAPE_XOR_TARGET        , // Gradient-filled ellipse (center radius)
    CURSOR_SHAPE_XOR_TARGET        , // Gradient-filled ellipse (corners)
    CURSOR_SHAPE_XOR_ROTATION      , // Rotate brush
    CURSOR_SHAPE_XOR_TARGET        , // Stretch brush
    CURSOR_SHAPE_TARGET            , // Distort brush
    CURSOR_SHAPE_XOR_TARGET        , // Gradient-filled rectangle
    CURSOR_SHAPE_COLORPICKER       , // Colorpick on right mouse button
    CURSOR_SHAPE_MULTIDIRECTIONAL  , // Pan view
  };
#else
  /// ::Cursor_shape to use for each operation.
  extern const byte CURSOR_FOR_OPERATION[NB_OPERATIONS];
#endif

///
/// Procedures to call for each state (determined by ::Operation_stack_size) of
/// each operation, and for each mouse state (no button,left button,right button)
GFX2_GLOBAL struct
{
  Func_action Action; ///< Function to call
  byte Hide_cursor;   ///< Boolean: Need to hide/unhide cursor during this step
  byte Fast_mouse;    ///< Operation should take shortcuts with mouse movements
} Operation[NB_OPERATIONS][3][OPERATION_STACK_SIZE];

// -- misc

///
/// Indicator of error in previous file operations.
/// -  0: OK
/// -  1: Error when beginning operation. Existing data should be ok.
/// -  2: Error while operation was in progress. Data is modified.
/// - -1: Interruption of a preview.
GFX2_GLOBAL signed char File_error;
/// Current line number when reading/writing gfx2.ini
GFX2_GLOBAL int Line_number_in_INI_file;

/// Set to true when the .cfg and .ini files are along the executable
GFX2_GLOBAL byte Portable_Installation_Detected;

// -- For iconv

#ifdef ENABLE_FILENAMES_ICONV

#include <iconv.h>

#define TOCODE   "CP1252"
#ifdef __macosx__
#define FROMCODE "UTF-8-MAC"
#else
#define FROMCODE "UTF-8"
#endif
GFX2_GLOBAL iconv_t cd;             // FROMCODE => TOCODE
GFX2_GLOBAL iconv_t cd_inv;         // TOCODE => FROMCODE
GFX2_GLOBAL iconv_t cd_utf16;       // FROMCODE => UTF16
GFX2_GLOBAL iconv_t cd_utf16_inv;   // UTF16 => FROMCODE
#endif /* ENABLE_FILENAMES_ICONV */

#endif
