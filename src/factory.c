/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2009-2011 Adrien Destugues

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

/*! \file factory.c
 *  \brief Brush factory - generates brush from lua scripts
 *
 * The brush factory allows you to generate brushes with Lua code.
 */

#include <math.h>

#include "brush.h"
#include "buttons.h"
#include "engine.h"
#include "errors.h"
#include "filesel.h" // Get_item_by_index
#include "fileseltools.h"
#include "global.h"
#include "graph.h"
#include "io.h"     // find_last_separator
#include "misc.h"
#include "pages.h"  // Backup()
#include "readline.h"
#include "screen.h"
#include "windows.h"
#include "palette.h"
#include "input.h" // Is_shortcut()
#include "help.h" // Window_help()
#include "graph.h"
#include "filesel.h" // Read_list_of_drives()
#include "realpath.h"
#include "setup.h"
#include "tiles.h"

/// Lua scripts bound to shortcut keys.
char * Bound_script[10];

#ifdef __ENABLE_LUA__

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <float.h> // for DBL_MAX
#include <limits.h> //for INT_MIN
#include <string.h> // strncpy()
#include <stdlib.h> // for atof()
#if defined(_MSC_VER)
#define strdup _strdup
#define putenv _putenv
#endif

#include "unicode.h"
#include "gfx2mem.h"

///
/// Number of characters for name in fileselector.
/// Window is adjusted according to it.
#define NAME_WIDTH 34
/// Number of characters for the description block
#define DESC_WIDTH ((NAME_WIDTH+2)*8/6)
/// Position of fileselector top, in window space
#define FILESEL_Y 30

// Work data that can be used during a script
static byte * Brush_backup = NULL;
static word Brush_backup_width;
static word Brush_backup_height;
static byte Palette_has_changed;
static byte Brush_was_altered;
static byte Original_fore_color;
static byte Original_back_color;
static byte Is_backed_up;
static T_Page * Main_backup_page;
static byte * Main_backup_screen;
static byte Cursor_is_visible;
static byte Window_needs_update;

/// Helper function to clamp a double to 0-255 range
static byte clamp_byte(double value)
{
  if (value<0.0)
    return 0;
  else if (value>255.0)
    return 255;
  else return (byte)value;
}

///
/// This macro reads a Lua argument into a double or integral lvalue.
/// If argument is invalid, it will break the caller and raise a verbose message.
/// This macro uses 2 existing symbols: L for the context, and nb_args=lua_gettop(L)
/// @param index     Index of the argument to check, starting at 1.
/// @param func_name The name of the lua callback, to display a message in case of error.
/// @param dest      Destination lvalue. Can be a double, or any integral type. Conversion will "floor".
/// @param min_value Check for minimum value. Pass a double, or if you don't care, -DBL_MAX.
/// @param max_value Check for maximum value. Pass a double, or if you don't care, DBL_MAX.
#define LUA_ARG_NUMBER(index, func_name, dest, min_value, max_value) \
do { \
  double value; \
  if (nb_args < (index)) return luaL_error(L, "%s: Argument %d is missing.", func_name, (index)); \
  if (!lua_isnumber(L, (index))) return luaL_error(L, "%s: Argument %d is not a number.", func_name, (index)); \
  value = lua_tonumber(L, (index)); \
  if ((min_value) != -DBL_MAX && value<(min_value)) return luaL_error(L, "%s: Argument %d was too small, it had value of %f and minimum should be %f.", func_name, (index), value, (double)(min_value)); \
  if ((max_value) != DBL_MAX && value>(max_value)) return luaL_error(L, "%s: Argument %d was too big, it had value of %f and maximum should be %f.", func_name, (index), value, (double)(max_value)); \
  dest = value; \
} while(0)

///
/// This macro reads a Lua argument into a string.
/// If argument is invalid, it will break the caller and raise a verbose message.
/// This macro uses 2 existing symbols: L for the context, and nb_args=lua_gettop(L)
/// @param index     Index of the argument to check, starting at 1.
/// @param func_name The name of the lua callback, to display a message in case of error.
/// @param dest      Destination string pointer, ideally a const char *.
#define LUA_ARG_STRING(index, func_name, dest) \
do { \
  if (nb_args < (index)) return luaL_error(L, "%s: Argument %d is missing.", func_name, index); \
  if (!lua_isstring(L, (index))) return luaL_error(L, "%s: Argument %d is not a string.", func_name, index); \
  dest = lua_tostring(L, (index)); \
} while (0)

///
/// This macro reads a Lua argument into a boolean.
/// If argument is invalid, it will break the caller and raise a verbose message.
/// This macro uses 2 existing symbols: L for the context, and nb_args=lua_gettop(L)
/// @param index     Index of the argument to check, starting at 1.
/// @param func_name The name of the lua callback, to display a message in case of error.
/// @param dest      Destination variable, any numeric type.
#define LUA_ARG_BOOL(index, func_name, dest) \
do { \
  if (nb_args < (index)) return luaL_error(L, "%s: Argument %d is missing.", func_name, index); \
  if (!lua_isboolean(L, (index))) return luaL_error(L, "%s: Argument %d is not a boolean.", func_name, index); \
  dest = lua_toboolean(L, (index)); \
} while (0)

///
/// This macro checks that a Lua argument is a function.
/// If argument is invalid, it will break the caller and raise a verbose message.
/// This macro uses 2 existing symbols: L for the context, and nb_args=lua_gettop(L)
/// @param index     Index of the argument to check, starting at 1.
/// @param func_name The name of the lua callback, to display a message in case of error.
#define LUA_ARG_FUNCTION(index, func_name) \
do { \
  if (nb_args < (index)) return luaL_error(L, "%s: Argument %d is missing.", func_name, index); \
  if (!lua_isfunction(L, (index))) return luaL_error(L, "%s: Argument %d is not a function.", func_name, index); \
} while (0)

/// Check if 'num' arguments were provided exactly
#define LUA_ARG_LIMIT(num, func_name) \
do { \
  if (nb_args != (num)) \
    return luaL_error(L, "%s: Expected %d arguments, but found %d.", func_name, (num), nb_args); \
} while(0)

/// Declares a function in the form BIND_unsaved() : for example L_PutPicturePixel_unsaved()
#define DECLARE_UNSAVED(BIND) \
int BIND ## _unsaved(lua_State* L) \
{ \
  Backup_if_necessary(L, Main.current_layer); \
  Register_main_writable(L); \
  return BIND(L); \
}

// forward declarations
void Register_main_readonly(lua_State* L);
void Register_main_writable(lua_State* L);
int L_SetColor(lua_State* L);
int L_SetColor_unsaved(lua_State* L);
//

const char * Lua_version(void)
{
  // LUA_RELEASE is only available since 5.2+, with format "Lua x.y.z"
  // The only version information available since Lua 5.0 is LUA_VERSION,
  // with the format "Lua x.y"
  #if defined(LUA_RELEASE)
  return LUA_RELEASE;
  #elif defined (LUA_VERSION)
  return LUA_VERSION;
  #else
  return "Unknown";
  #endif
}

// Updates the screen colors after a running screen has modified the palette.
void Update_colors_during_script(void)
{
  if (Palette_has_changed)
  {
    Set_palette(Main.palette);
    Compute_optimal_menu_colors(Main.palette);
    Display_menu();
    Palette_has_changed=0;
  }
}

/// Paint a pixel in image without updating the screen
static void Pixel_figure_no_screen(word x_pos, word y_pos, byte color)
{
  if (x_pos < Main.image_width && y_pos < Main.image_height)
    Pixel_in_current_screen(x_pos, y_pos, color);
}

void Backup_if_necessary(lua_State* L, int layer)
{
  if (!Is_backed_up)
  {
    Backup_layers(layer);
    Is_backed_up = 1;
    if (layer == Main.current_layer)
    {
      Main_backup_page = Main.backups->Pages->Next;
      Main_backup_screen = Screen_backup;
      Register_main_writable(L);
    }
  }
  else
  {
    if (layer == LAYER_NONE)
    {
      // Already OK
      return;
    }
    if (Dup_layer_if_shared(Main.backups->Pages, layer))
    {
      // Depth buffer etc to modify too ?

      Register_main_writable(L);
    }
  }
}

// Wrapper functions to call C from Lua

int L_SetBrushSize(lua_State* L)
{
  int i;
  int w;
  int h;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "setbrushsize");
  LUA_ARG_NUMBER(1, "setbrushsize", w, 1, 10000);
  LUA_ARG_NUMBER(2, "setbrushsize", h, 1, 10000);
  
  if (Realloc_brush(w, h, NULL, NULL))
  {
    return luaL_error(L, "setbrushsize: Resize failed");
  }
  Brush_was_altered=1;
  // Fill with Back_color
  memset(Brush_original_pixels,Back_color,(long)Brush_width*Brush_height);
  memset(Brush,Back_color,(long)Brush_width*Brush_height);
  // Adopt the current palette.
  memcpy(Brush_original_palette, Main.palette,sizeof(T_Palette));
  for (i=0; i<256; i++)
    Brush_colormap[i]=i;
  //--

  // Center the handle
  Brush_offset_X=(Brush_width>>1);
  Brush_offset_Y=(Brush_height>>1);
  return 0;
}

int L_GetBrushSize(lua_State* L)
{
  lua_pushinteger(L, Brush_width);  
  lua_pushinteger(L, Brush_height); 
  return 2;
}

int L_PutBrushPixel(lua_State* L)
{
  int x;
  int y;
  uint8_t c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (3, "putbrushpixel");
  LUA_ARG_NUMBER(1, "putbrushpixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "putbrushpixel", y, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "putbrushpixel", c, INT_MIN, INT_MAX);

  if (!Brush_was_altered)
  {
    int i;
    
    // First time writing in brush:
    // Adopt the current palette.
    memcpy(Brush_original_palette, Main.palette,sizeof(T_Palette));
    memcpy(Brush_original_pixels, Brush, Brush_width*Brush_height);
    for (i=0; i<256; i++)
      Brush_colormap[i]=i;
    //--
    Brush_was_altered=1;
  }
  
  if (x<0 || y<0 || x>=Brush_width || y>=Brush_height)
  ;
  else
  {
    Pixel_in_brush(x, y, c);
  }
  return 0; // no values returned for lua
}

int L_GetBrushPixel(lua_State* L)
{
  int x;
  int y;
  uint8_t c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "getbrushpixel");
  LUA_ARG_NUMBER(1, "getbrushpixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "getbrushpixel", y, INT_MIN, INT_MAX);

  if (x<0 || y<0 || x>=Brush_width || y>=Brush_height)
  {
    c = Back_color; // Return 'transparent'
  }
  else
  {
    c = Read_pixel_from_brush(x, y);
  }
  lua_pushinteger(L, c);
  return 1;
}

int L_GetBrushBackupPixel(lua_State* L)
{
  int x;
  int y;
  uint8_t c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "getbrushbackuppixel");
  LUA_ARG_NUMBER(1, "getbrushbackuppixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "getbrushbackuppixel", y, INT_MIN, INT_MAX);
  
  if (x<0 || y<0 || x>=Brush_backup_width || y>=Brush_backup_height)
  {
    c = Back_color; // Return 'transparent'
  }
  else
  {
    c = *(Brush_backup + y * Brush_backup_width + x);
  }
  lua_pushinteger(L, c);
  return 1;
}

int L_SetPictureSize(lua_State* L)
{
  
  int w;
  int h;
  int nb_args=lua_gettop(L);
  int i;
  
  LUA_ARG_LIMIT (2, "setpicturesize");
  LUA_ARG_NUMBER(1, "setpicturesize", w, 1, 9999);
  LUA_ARG_NUMBER(2, "setpicturesize", h, 1, 9999);
  
  if (w == Main.image_width && h == Main.image_height)
  {
    // nothing to do at all
    return 0;
  }
  
  if (Is_backed_up)
  {
    // Script has already modified pixels or palette :
    // Resize without counting an additional Undo step.
    Backup_in_place(w, h);
  }
  else
  {
    // Script has not modified the image/palette yet, so it's not backed up:
    // Make a backup that counts as a new Undo step.
    Upload_infos_page(&Main);
    Backup_with_new_dimensions(w, h);
  }
  // part of Resize_image() : the pixel copy part.
  for (i=0; i<Main.backups->Pages->Nb_layers; i++)
  {
    Copy_part_of_image_to_another(
      Main.backups->Pages->Next->Image[i].Pixels,0,0,Min(Main.backups->Pages->Next->Width,Main.image_width),
      Min(Main.backups->Pages->Next->Height,Main.image_height),Main.backups->Pages->Next->Width,
      Main.backups->Pages->Image[i].Pixels,0,0,Main.image_width);
  }
  Redraw_layered_image();

  Is_backed_up = 1;
  Main_backup_page = Main.backups->Pages->Next;
  Main_backup_screen = Screen_backup;
  Register_main_writable(L);
  lua_register(L,"setcolor",L_SetColor);
  
  return 0;
}


int L_SetSparePictureSize(lua_State* L)
{
  
  int w;
  int h;
  int nb_args=lua_gettop(L);
  int i;
  
  LUA_ARG_LIMIT (2, "setsparepicturesize");
  LUA_ARG_NUMBER(1, "setsparepicturesize", w, 1, 9999);
  LUA_ARG_NUMBER(2, "setsparepicturesize", h, 1, 9999);
    
  Backup_and_resize_the_spare(w, h);
  // part of Resize_image() : the pixel copy part.
  for (i=0; i<Spare.backups->Pages->Nb_layers; i++)
  {
    Copy_part_of_image_to_another(
      Spare.backups->Pages->Next->Image[i].Pixels,0,0,Min(Spare.backups->Pages->Next->Width,Spare.image_width),
      Min(Spare.backups->Pages->Next->Height,Spare.image_height),Spare.backups->Pages->Next->Width,
      Spare.backups->Pages->Image[i].Pixels,0,0,Spare.image_width);
  }
  Redraw_spare_image();
  
  return 0;
}

int L_GetPictureSize(lua_State* L)
{
  lua_pushinteger(L, Main.image_width);
  lua_pushinteger(L, Main.image_height);
  return 2;
}

int L_ClearPicture(lua_State* L)
{
  int c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "clearpicture");
  LUA_ARG_NUMBER(1, "clearpicture", c, INT_MIN, INT_MAX);
  
  if (Stencil_mode && Config.Clear_with_stencil)
    Clear_current_image_with_stencil(c,Stencil);
  else
    Clear_current_image(c);
  Redraw_layered_image();
  
  return 0; // no values returned for lua
}

int L_PutPicturePixel(lua_State* L)
{
  int x;
  int y;
  int c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (3, "putpicturepixel");
  LUA_ARG_NUMBER(1, "putpicturepixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "putpicturepixel", y, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "putpicturepixel", c, INT_MIN, INT_MAX);
  
  // Bound check
  if (x<0 || y<0 || x>=Main.image_width || y>=Main.image_height)
  {
    // Silently ignored
    return 0;
  }
  Pixel_in_current_screen(x, y, c);
  return 0; // no values returned for lua
}


int L_PutSparePicturePixel(lua_State* L)
{
  int x;
  int y;
  int c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (3, "putsparepicturepixel");
  LUA_ARG_NUMBER(1, "putsparepicturepixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "putsparepicturepixel", y, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "putsparepicturepixel", c, INT_MIN, INT_MAX);
  
  // Bound check
  if (x<0 || y<0 || x>=Spare.image_width || y>=Spare.image_height)
  {
    // Silently ignored
    return 0;
  }
  Pixel_in_spare(x, y, c);
  return 0; // no values returned for lua
}


int L_DrawLine(lua_State* L)
{
  int x1, y1, x2, y2, c;

  int nb_args = lua_gettop(L);

  LUA_ARG_LIMIT(5, "drawline");
  LUA_ARG_NUMBER(1, "drawline", x1, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "drawline", y1, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "drawline", x2, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(4, "drawline", y2, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(5, "drawline", c,  INT_MIN, INT_MAX);

  Set_Pixel_figure(Pixel_figure_no_screen);
  Draw_line_general(x1, y1, x2, y2, c);

  return 0;
}


int L_DrawFilledRect(lua_State* L)
{
  int x1, y1, x2, y2, c;
  int min_x,min_y,max_x,max_y, x_pos, y_pos;

  int nb_args = lua_gettop(L);

  LUA_ARG_LIMIT(5, "drawfilledrect");
  LUA_ARG_NUMBER(1, "drawfilledrect", x1, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "drawfilledrect", y1, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "drawfilledrect", x2, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(4, "drawfilledrect", y2, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(5, "drawfilledrect", c,  INT_MIN, INT_MAX);

  // Put bounds in ascending order
  if (x2>x1)
  {
    min_x=x1;
    max_x=x2;
  }
  else
  {
    min_x=x2;
    max_x=x1;
  }
  if (y2>y1)
  {
    min_y=y1;
    max_y=y2;
  }
  else
  {
    min_y=y2;
    max_y=y1;
  }

  // Clipping limits
  if (max_x>Main.image_width)
    max_x=Main.image_width-1;
  if (max_y>Main.image_height)
    max_y=Main.image_height-1;
  if (min_x<0)
    min_x=0;
  if (min_y<0)
    min_y=0;

  // Perform drawing
  for (y_pos=min_y; y_pos<=max_y;y_pos++)
    for (x_pos=min_x; x_pos<=max_x;x_pos++)
      Pixel_in_current_screen(x_pos,y_pos,c);
  return 0;
  
}


int L_DrawCircle(lua_State* L)
{
  int x1, y1, r, c;

  int nb_args = lua_gettop(L);

  LUA_ARG_LIMIT(4, "drawcircle");
  LUA_ARG_NUMBER(1, "drawcircle", x1, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "drawcircle", y1, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "drawcircle", r, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(4, "drawcircle", c, INT_MIN, INT_MAX);

  Set_Pixel_figure(Pixel_figure_no_screen);
  Draw_empty_circle_general(x1, y1, r*r, c);

  return 0;
}


int L_DrawDisk(lua_State* L)
{
  int center_x, center_y, diameter, c, r, even;
  long circle_limit;
  short x_pos,y_pos;
  short min_x,max_x,min_y,max_y;
  double r_float;

  int nb_args = lua_gettop(L);

  LUA_ARG_LIMIT(4, "drawdisk");
  LUA_ARG_NUMBER(1, "drawdisk", center_x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "drawdisk", center_y, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "drawdisk", r_float, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(4, "drawdisk", c, INT_MIN, INT_MAX);

  if (r_float<0.0)
    return 0;
  diameter=(int)(floor(r_float*2.0+1.0));
  r=diameter/2;
  even=!(diameter&1);
  
  circle_limit = Circle_squared_diameter(diameter);
  
  // Compute clipping limits
  min_x=center_x-r+even<0 ? 0 : center_x-r+even;
  max_x=center_x+r>=Main.image_width? Main.image_width-1 : center_x+r;
  min_y=center_y-r+even<0 ? 0 : center_y-r+even;
  max_y=center_y+r>=Main.image_height? Main.image_height-1 : center_y+r;

  for (y_pos=min_y;y_pos<=max_y;y_pos++)
  {
    short y=(y_pos-center_y)*2-even;
    for (x_pos=min_x;x_pos<=max_x;x_pos++)
    {
      short x=(x_pos-center_x)*2-even;
      if (x*x+y*y <= circle_limit)
        Pixel_in_current_screen(x_pos,y_pos,c);
    }
  }

  return 0;
}


int L_GetPicturePixel(lua_State* L)
{
  int x;
  int y;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "getpicturepixel");
  LUA_ARG_NUMBER(1, "getpicturepixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "getpicturepixel", y, INT_MIN, INT_MAX);
  
  // Bound check
  if (x<0 || y<0 || x>=Main.image_width || y>=Main.image_height)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Main.backups->Pages->Transparent_color);
    return 1;
  }
  lua_pushinteger(L, Read_pixel_from_current_screen(x,y));
  return 1;
}

int L_GetBackupPixel(lua_State* L)
{
  int x;
  int y;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "getbackuppixel");
  LUA_ARG_NUMBER(1, "getbackuppixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "getbackuppixel", y, INT_MIN, INT_MAX);
  
  // Bound check
  if (x<0 || y<0 || x>=Main_backup_page->Width || y>=Main_backup_page->Height)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Main_backup_page->Transparent_color);
    return 1;
  }
  // Can't use Read_pixel_from_backup_screen(), because in a Lua script
  // the "backup" can use a different screen dimension.
  lua_pushinteger(L, *(Main_backup_screen + x + Main_backup_page->Width * y));
  
  return 1;
}

int L_GetLayerPixel(lua_State* L)
{
  int x;
  int y;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "getlayerpixel");
  LUA_ARG_NUMBER(1, "getlayerpixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "getlayerpixel", y, INT_MIN, INT_MAX);
  
  // Bound check
  if (x<0 || y<0 || x>=Main.image_width || y>=Main.image_height)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Main.backups->Pages->Transparent_color);
    return 1;
  }
  lua_pushinteger(L, Read_pixel_from_current_layer(x,y));
  return 1;
}

// Spare

int L_GetSparePictureSize(lua_State* L)
{
  lua_pushinteger(L, Spare.image_width);
  lua_pushinteger(L, Spare.image_height);
  return 2;
}

int L_GetSpareLayerPixel(lua_State* L)
{
  int x;
  int y;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "getsparelayerpixel");
  LUA_ARG_NUMBER(1, "getsparelayerpixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "getsparelayerpixel", y, INT_MIN, INT_MAX);
  
  // Bound check
  if (x<0 || y<0 || x>=Spare.image_width || y>=Spare.image_height)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Spare.backups->Pages->Transparent_color);
    return 1;
  }
  lua_pushinteger(L, *(Spare.backups->Pages->Image[Spare.current_layer].Pixels + y*Spare.image_width + x));
  return 1;
}

int L_GetSparePicturePixel(lua_State* L)
{
  int x;
  int y;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (2, "getsparepicturepixel");
  LUA_ARG_NUMBER(1, "getsparepicturepixel", x, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "getsparepicturepixel", y, INT_MIN, INT_MAX);
  
  // Some bound checking is done by the function itself, here's the rest.
  if (x<0 || y<0)
  {
    // Silently return the image's transparent color
    lua_pushinteger(L, Spare.backups->Pages->Transparent_color);
    return 1;
  }
  lua_pushinteger(L, Read_pixel_from_spare_screen(x,y));
  return 1;
}

int L_GetSpareColor(lua_State* L)
{
  byte c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "getsparecolor");
  LUA_ARG_NUMBER(1, "getsparecolor", c, INT_MIN, INT_MAX);

  lua_pushinteger(L, Spare.palette[c].R);
  lua_pushinteger(L, Spare.palette[c].G);
  lua_pushinteger(L, Spare.palette[c].B);
  return 3;
}

int L_GetSpareTransColor(lua_State* L)
{
  lua_pushinteger(L, Spare.backups->Pages->Transparent_color);
  return 1;
}


int L_SetColor(lua_State* L)
{
  byte c;
  double r, g, b;
  int nb_args=lua_gettop(L);
    
  LUA_ARG_LIMIT (4, "setcolor");
  LUA_ARG_NUMBER(1, "setcolor", c, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(2, "setcolor", r, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(3, "setcolor", g, INT_MIN, INT_MAX);
  LUA_ARG_NUMBER(4, "setcolor", b, INT_MIN, INT_MAX);
  
    
  Main.palette[c].R=Round_palette_component(clamp_byte(r));
  Main.palette[c].G=Round_palette_component(clamp_byte(g));
  Main.palette[c].B=Round_palette_component(clamp_byte(b));
  // Set_color(c, r, g, b); Not needed. Update screen when script is finished
  Palette_has_changed=1;
  return 0;
}

int L_SetColor_unsaved(lua_State* L)
{
  Backup_if_necessary(L, LAYER_NONE);
  lua_register(L,"setcolor",L_SetColor);
  return L_SetColor(L);
}


int L_GetColor(lua_State* L)
{
  byte c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "getcolor");
  LUA_ARG_NUMBER(1, "getcolor", c, INT_MIN, INT_MAX);

  lua_pushinteger(L, Main.palette[c].R);
  lua_pushinteger(L, Main.palette[c].G);
  lua_pushinteger(L, Main.palette[c].B);
  return 3;
}

int L_GetBackupColor(lua_State* L)
{
  byte c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "getbackupcolor");
  LUA_ARG_NUMBER(1, "getbackupcolor", c, INT_MIN, INT_MAX);

  lua_pushinteger(L, Main_backup_page->Palette[c].R);
  lua_pushinteger(L, Main_backup_page->Palette[c].G);
  lua_pushinteger(L, Main_backup_page->Palette[c].B);
  return 3;
}

int L_MatchColor(lua_State* L)
{
  double r, g, b;
  int c;
  int nb_args=lua_gettop(L);

  LUA_ARG_LIMIT (3, "matchcolor");
  LUA_ARG_NUMBER(1, "matchcolor", r, -DBL_MAX, DBL_MAX);
  LUA_ARG_NUMBER(2, "matchcolor", g, -DBL_MAX, DBL_MAX);
  LUA_ARG_NUMBER(3, "matchcolor", b, -DBL_MAX, DBL_MAX);
  
  c = Best_color_nonexcluded(clamp_byte(r),clamp_byte(g),clamp_byte(b));
  lua_pushinteger(L, c);
  return 1;
}

int L_MatchColor2(lua_State* L)
{
  double r, g, b;
  double l_weight = 0.25;
  byte best_color = 0;
  int nb_args=lua_gettop(L);

  if (nb_args < 3 || nb_args > 4)
  {
    return luaL_error(L, "matchcolor2: Expected 3 or 4 arguments, but found %d.", nb_args);
  }
  LUA_ARG_NUMBER(1, "matchcolor2", r, -DBL_MAX, DBL_MAX);
  LUA_ARG_NUMBER(2, "matchcolor2", g, -DBL_MAX, DBL_MAX);
  LUA_ARG_NUMBER(3, "matchcolor2", b, -DBL_MAX, DBL_MAX);
  if (nb_args > 3)
  {
    LUA_ARG_NUMBER(4, "matchcolor2", l_weight, 0.0, 1.0);
  }
  
  // Similar to Best_color_perceptual(), but with floating point
  {
    int col;  
    double best_diff=9e99;
    double target_bri;
    double bri;
    double diff_b, diff_c, diff;
  
    if (r<0.0)
      r=0;
    else if (r>255.0)
      r=255.0;
    if (g<0.0)
      g=0;
    else if (g>255.0)
      g=255.0;
    if (b<0.0)
      b=0;
    else if (b>255.0)
      b=255.0;
  
    // Similar to Perceptual_lightness();
    target_bri = sqrt(0.26*r*0.26*r + 0.55*g*0.55*g + 0.19*b*0.19*b);
    
    for (col=0; col<256; col++)
    {
      if (Exclude_color[col])
        continue;
  
      diff_c = sqrt(
        (0.26*(Main.palette[col].R-r))*
        (0.26*(Main.palette[col].R-r))+
        (0.55*(Main.palette[col].G-g))*
        (0.55*(Main.palette[col].G-g))+
        (0.19*(Main.palette[col].B-b))*
        (0.19*(Main.palette[col].B-b)));
      // Exact match
      if (diff_c<1.0)
      {
          lua_pushinteger(L, col);
          return 1;
      }
  
      bri = sqrt(0.26*0.26*(Main.palette[col].R*Main.palette[col].R) + 0.55*0.55*(Main.palette[col].G*Main.palette[col].G) + 0.19*0.19*(Main.palette[col].B*Main.palette[col].B));
      diff_b = fabs(target_bri-bri);
  
      diff=l_weight*(diff_b-diff_c)+diff_c;
      if (diff<best_diff)
      {
        best_diff=diff;
        best_color=col;
      } 
    }
  }
  
  lua_pushinteger(L, best_color);
  return 1;
}

int L_GetForeColor(lua_State* L)
{
  lua_pushinteger(L, Fore_color);
  return 1;
}

int L_GetBackColor(lua_State* L)
{
  lua_pushinteger(L, Back_color);
  return 1;
}

int L_GetTransColor(lua_State* L)
{
  lua_pushinteger(L, Main.backups->Pages->Transparent_color);
  return 1;
}

int L_SetForeColor(lua_State* L)
{
  byte c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "setforecolor");
  LUA_ARG_NUMBER(1, "setforecolor", c, -DBL_MAX, DBL_MAX);

  Fore_color = c;

  return 0;
}

int L_SetBackColor(lua_State* L)
{
  byte c;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "setbackcolor");
  LUA_ARG_NUMBER(1, "setbackcolor", c, -DBL_MAX, DBL_MAX);

  Back_color = c;

  return 0;
}


int L_InputBox(lua_State* L)
{
#define max_settings (9)
#define args_per_setting (5)
  double min_value[max_settings];
  double max_value[max_settings];
  double decimal_places[max_settings];
  double current_value[max_settings];
  const char * label[max_settings];
  unsigned short control[max_settings*3+1]; // Each value has at most 3 widgets.
  enum CONTROL_TYPE {
    CONTROL_OK          = 0x0100,
    CONTROL_CANCEL      = 0x0200,
    CONTROL_INPUT       = 0x0300,
    CONTROL_INPUT_MINUS = 0x0400,
    CONTROL_INPUT_PLUS  = 0x0500,
    CONTROL_CHECKBOX    = 0x0600,
    CONTROL_VALUE_MASK  = 0x00FF,
    CONTROL_TYPE_MASK   = 0xFF00
  };
  const char * window_caption;
  int caption_length;
  int nb_settings;
  int nb_args;
  unsigned int max_label_length;
  int setting;
  short clicked_button;
  char  str[40];
  short close_window = 0;
  
  nb_args = lua_gettop (L);
  
  
  if (nb_args < 6)
  {
    return luaL_error(L, "inputbox: Less than 6 arguments");
  }
  if ((nb_args - 1) % args_per_setting)
  {
    return luaL_error(L, "inputbox: Wrong number of arguments");
  }
  nb_settings = (nb_args-1)/args_per_setting;
  if (nb_settings > max_settings)
  {
    return luaL_error(L, "inputbox: Too many settings, limit reached");
  }
  
  max_label_length=4; // Minimum size to account for OK / Cancel buttons
  
  // First argument is window caption
  LUA_ARG_STRING(1, "inputbox", window_caption);
  caption_length = strlen(window_caption);
  if ( caption_length > 14)
    max_label_length = caption_length - 10;
  
  for (setting=0; setting<nb_settings; setting++)
  {
    int label_length;
    
    LUA_ARG_STRING(setting*args_per_setting+2, "inputbox", label[setting]);
    label_length=strlen(label[setting]);

    LUA_ARG_NUMBER(setting*args_per_setting+3, "inputbox", current_value[setting], -DBL_MAX, DBL_MAX);
    LUA_ARG_NUMBER(setting*args_per_setting+4, "inputbox", min_value[setting], -DBL_MAX, DBL_MAX);
    /*if (min_value[setting] < -999999999999999.0)
      min_value[setting] = -999999999999999.0;*/
    LUA_ARG_NUMBER(setting*args_per_setting+5, "inputbox", max_value[setting], -DBL_MAX, DBL_MAX);
    /*if (max_value[setting] > 999999999999999.0)
      max_value[setting] = 999999999999999.0;*/
    LUA_ARG_NUMBER(setting*args_per_setting+6, "inputbox", decimal_places[setting], -15.0, 15.0);
    if (decimal_places[setting]>15)
        decimal_places[setting]=15;
    if (min_value[setting]!=0 || max_value[setting]!=1)
      if (decimal_places[setting]<0)
        decimal_places[setting]=0;
    // Keep current value in range
    if (decimal_places[setting]>=0)
      current_value[setting] = Fround(current_value[setting], decimal_places[setting]);

    if (current_value[setting] < min_value[setting])
      current_value[setting] = min_value[setting];
    else if (current_value[setting] > max_value[setting])
      current_value[setting] = max_value[setting];

    if (min_value[setting]==0 && max_value[setting]==0)
      label_length-=12;
    if (label_length > (int)max_label_length)
      max_label_length = label_length;
  }
  // Max is 25 to keep window under 320 pixel wide
  if (max_label_length>25)
    max_label_length=25;

  Update_colors_during_script();
  if (!Cursor_is_visible)
    Display_cursor();
  Open_window(115+max_label_length*8,44+nb_settings*17,window_caption);

  // Normally this index is unused, but this initialization avoids
  // any weird behavior if it was used by mistake.
  control[0]=0;
  
  // OK
  Window_set_normal_button( 7, 25 + 17 * nb_settings, 51,14,"OK" , 0,1,KEY_RETURN);
  control[Window_nb_buttons] = CONTROL_OK;

  // Cancel
  Window_set_normal_button( 64, 25 + 17 * nb_settings, 51,14,"Cancel" , 0,1,KEY_ESC);
  control[Window_nb_buttons] = CONTROL_CANCEL;
  
  for (setting=0; setting<nb_settings; setting++)
  {
    if (min_value[setting]==0 && max_value[setting]==0)
    {
      // Label
      Print_in_window_limited(12,22+setting*17,label[setting],max_label_length+12,MC_Black,MC_Light);
    }
    else if (min_value[setting]==0 && max_value[setting]==1 && decimal_places[setting]<=0)
    {
      // Checkbox or Radio button
      byte outline = decimal_places[setting]==0 ? 2 : 0;
      Window_set_normal_button(12+max_label_length*8+44-outline, 21+setting*17-outline, 12+outline*2,10+outline*2,current_value[setting]?"X":"", 0,1,KEY_NONE);
      control[Window_nb_buttons] = CONTROL_CHECKBOX | setting;

      Print_in_window_limited(12,22+setting*17,label[setting],max_label_length,MC_Black,MC_Light);
    }
    else
    {
      // - Button
      Window_set_repeatable_button(12+max_label_length*8+5, 20+setting*17, 13,11,"-" , 0,1,KEY_NONE);
      control[Window_nb_buttons] = CONTROL_INPUT_MINUS | setting;

      // Numeric input field
      Window_set_input_button(12+max_label_length*8+21, 20+setting*17,7);
      // Print editable value
      Sprint_double(str,current_value[setting],decimal_places[setting],7);
      Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 7,MC_Black,MC_Light);
      //
      control[Window_nb_buttons] = CONTROL_INPUT | setting;

      // + Button
      Window_set_repeatable_button(12+max_label_length*8+84, 20+setting*17, 13,11,"+" , 0,1,KEY_NONE);
      control[Window_nb_buttons] = CONTROL_INPUT_PLUS | setting;

      Print_in_window_limited(12,22+setting*17,label[setting],max_label_length,MC_Black,MC_Light);
    }
  }
  
  // Draw outlines for series of radio buttons
  for (setting=0; setting<nb_settings; setting++)
  {
    if (min_value[setting]==0 && max_value[setting]==1 && decimal_places[setting]<0)
    {
      word first_setting=setting;
      for (;setting+1<nb_settings; setting++)
        if (min_value[setting+1]!=0 || max_value[setting+1]!=1 || decimal_places[setting+1]!=decimal_places[first_setting])
          break;
      
      Window_display_frame_in(12+max_label_length*8+41,18+first_setting*17,18,(setting-first_setting+1)*17-1);
    }
  }
  
  Update_window_area(0,0,Window_width, Window_height);
  Cursor_shape=CURSOR_SHAPE_ARROW;
  Display_cursor();

  while (!close_window)
  {
    if (Quit_is_required)
      close_window = CONTROL_CANCEL;
    clicked_button=Window_clicked_button();
    if (clicked_button>0)
    {
      setting = control[clicked_button] & (CONTROL_VALUE_MASK);
      
      switch (control[clicked_button] & CONTROL_TYPE_MASK)
      {
        case CONTROL_OK:
          close_window = CONTROL_OK;
          break;
          
        case CONTROL_CANCEL:
          close_window = CONTROL_CANCEL;
          break;
          
        case CONTROL_INPUT:

          Sprint_double(str,current_value[setting],decimal_places[setting],0);
          Readline_ex(12+max_label_length*8+23, 22+setting*17,str,7,40,INPUT_TYPE_DECIMAL,decimal_places[setting]);
          current_value[setting]=atof(str);

          if (current_value[setting] < min_value[setting])
            current_value[setting] = min_value[setting];
          else if (current_value[setting] > max_value[setting])
            current_value[setting] = max_value[setting];
          // Print editable value
          Sprint_double(str,current_value[setting],decimal_places[setting],7);
          Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 7,MC_Black,MC_Light);
          //
          Display_cursor();
          
          break;
          
        case CONTROL_INPUT_MINUS:
          if (current_value[setting] > min_value[setting])
          {
            current_value[setting]--;
            if (current_value[setting] < min_value[setting])
              current_value[setting] = min_value[setting];
              
            Hide_cursor();
            // Print editable value
            Sprint_double(str,current_value[setting],decimal_places[setting],7);
            Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 7,MC_Black,MC_Light);
            //
            Display_cursor();
          }
          break;
          
        case CONTROL_INPUT_PLUS:
          if (current_value[setting] < max_value[setting])
          {
            current_value[setting]++;
            if (current_value[setting] > max_value[setting])
              current_value[setting] = max_value[setting];
            
            Hide_cursor();
            // Print editable value
            Sprint_double(str,current_value[setting],decimal_places[setting],7);
            Print_in_window_limited(12+max_label_length*8+23, 22+setting*17, str, 7,MC_Black,MC_Light);
            //
            Display_cursor();
          }
          break;
          
        case CONTROL_CHECKBOX:
          if (decimal_places[setting]==0 || current_value[setting]==0.0)
          {
            current_value[setting] = (current_value[setting]==0.0);
            Hide_cursor();
            Print_in_window(12+max_label_length*8+46, 22+setting*17, current_value[setting]?"X":" ",MC_Black,MC_Light);
            // Uncheck other buttons of same family
            if (decimal_places[setting]<0)
            {
              byte button;
              for (button=3; button<=Window_nb_buttons; button++)
              {
                if (button != clicked_button && control[button] & CONTROL_CHECKBOX)
                {
                  byte other_setting = control[button] & (CONTROL_VALUE_MASK);
                  if (decimal_places[other_setting] == decimal_places[setting])
                  {
                    // Same family: unset and uncheck
                    current_value[other_setting]=0.0;
                    Print_in_window(12+max_label_length*8+46, 22+other_setting*17, " ",MC_Black,MC_Light);
                  }
                }
              }
            }
            Display_cursor();
          }
          break;
      }
    }
  }
  
  Close_window();
  Cursor_shape=CURSOR_SHAPE_HOURGLASS;
  Display_cursor();
  Cursor_is_visible=1;
  
  // Return values:

  // One boolean to tell if user pressed ok or cancel
  lua_pushboolean(L, (close_window == CONTROL_OK));
  
  // One value per control
  for (setting=0; setting<nb_settings; setting++)
    lua_pushnumber(L, current_value[setting]);
    
  return 1 + nb_settings;
#undef max_settings
#undef args_per_setting
}

int L_SelectBox(lua_State* L)
{
#define max_settings (10)
  const char * label[max_settings];
#undef max_settings

  const char * window_caption;
  unsigned int caption_length;
  int nb_args;
  
  unsigned int max_label_length;
  int button;
  int nb_buttons;
  short clicked_button;
  //char  str[40];
  //short close_window = 0;
  
  nb_args = lua_gettop (L);
  
  if (nb_args < 2)
  {
    return luaL_error(L, "selectbox: Less than 2 arguments");
  }
  if ((nb_args - 1) % 2)
  {
    return luaL_error(L, "selectbox: Wrong number of arguments");
  }
  
  
  nb_buttons=(nb_args-1) /2;
  
  max_label_length=4; // Minimum size
  
  // First argument is window caption
  LUA_ARG_STRING(1, "selectbox", window_caption);
  caption_length = strlen(window_caption);
  if ( caption_length > max_label_length)
    max_label_length = caption_length;
  
  for (button=0; button<nb_buttons; button++)
  {
    LUA_ARG_STRING(button*2+2, "selectbox", label[button]);
    if (strlen(label[button]) > max_label_length)
      max_label_length = strlen(label[button]);
    LUA_ARG_FUNCTION(button*2+3, "selectbox");
  }
  // Max is 25 to keep window under 320 pixel wide
  if (max_label_length>25)
    max_label_length=25;

  Update_colors_during_script();
  if (!Cursor_is_visible)
    Display_cursor();
  Open_window(28+max_label_length*8,26+nb_buttons*17,window_caption);

  for (button=0; button<nb_buttons; button++)
  {
    Window_set_normal_button( 7, 22 + 17 * button, 14+max_label_length*8,14,label[button], 0,1,KEY_NONE);
  }

  Update_window_area(0,0,Window_width, Window_height);
  Cursor_shape=CURSOR_SHAPE_ARROW;
  Display_cursor();
 
  Key=0;
  do
  {
    clicked_button=Window_clicked_button();
  } while (clicked_button<1 && Key != KEY_ESC && !Quit_is_required);
    
  Close_window();
  Cursor_shape=CURSOR_SHAPE_HOURGLASS;
  Display_cursor();
  Cursor_is_visible=1;

  if (clicked_button)
  {
    // Push the function on top of the stack
    lua_pushvalue(L, 1+2*clicked_button);
    // Call it
    lua_call(L, 0, 0);
  }
  // No return value:    
  return 0;
}

int L_MessageBox(lua_State* L)
{
  const char * caption;
  const char * message;
  int nb_args = lua_gettop (L);
  
  if (nb_args == 1)
  {
    caption = "Script message";
    LUA_ARG_STRING(1, "messagebox", message);
  }
  else if (nb_args == 2)
  {
    LUA_ARG_STRING(1, "messagebox", caption);
    LUA_ARG_STRING(2, "messagebox", message);
  }
  else
  {
    return luaL_error(L, "messagebox: Needs one or two arguments.");
  }

  Update_colors_during_script();
  if (!Cursor_is_visible)
    Display_cursor();
  Verbose_message(caption, message);
  Cursor_is_visible=1;
  return 0;
}

int L_Wait(lua_State* L)
{
  float delay;
  dword end;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "wait");
  LUA_ARG_NUMBER(1, "wait", delay, 0.0, 10.0);

  if (!Cursor_is_visible)
  {
    Display_cursor();
    Cursor_is_visible=1;
  }
  // Simple 'yield'
  if (delay == 0.0)
  {
    Get_input(0);
    return 0;
  }
  // Wait specified time
  end = GFX2_GetTicks()+(int)(delay*1000.0);
  
  do
  {
    Get_input(20);
  } while (GFX2_GetTicks()<end);
  return 0;
}

int L_WaitBreak(lua_State* L)
{
  float delay;
  dword end;
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "waitbreak");
  LUA_ARG_NUMBER(1, "waitbreak", delay, 0.0, DBL_MAX);

  if (!Cursor_is_visible)
  {
    Display_cursor();
    Cursor_is_visible=1;
  }
  // Simple 'yield'
  if (delay == 0.0)
  {
    Get_input(0);
    lua_pushinteger(L, (Key == KEY_ESC));
    return 1;
  }
  // Wait specified time
  end = GFX2_GetTicks()+(int)(delay*1000.0);
  
  do
  {
    Get_input(20);
    if (Key == KEY_ESC)
    {
      lua_pushinteger(L, 1);
      return 1;
    }
  } while (GFX2_GetTicks()<end);

  lua_pushinteger(L, 0);
  return 1;
}

int L_WaitInput(lua_State* L)
{
  float delay;
  dword end;
  int nb_args=lua_gettop(L);
  int moved;
  
  /*word key;
  word button;
  word mouse_x;
  word mouse_y;*/
  
  LUA_ARG_LIMIT (1, "waitinput");
  LUA_ARG_NUMBER(1, "waitinput", delay, 0.0, DBL_MAX);

  if (!Cursor_is_visible)
  {
    Display_cursor();
    Cursor_is_visible=1;
  }
  // Simple 'yield'
  if (delay == 0.0)
  {
    moved=Get_input(0);
  }
  else
  {
    // Wait specified time
    end = GFX2_GetTicks()+(int)(delay*1000.0);
  
    moved=0;
    do
    {
      moved=Get_input(20);
    } while (!moved && GFX2_GetTicks()<end);
  }
  
  lua_pushboolean(L, moved ? 1 : 0);
  lua_pushinteger(L, (Key == KEY_ESC) ? KEY_ESCAPE : Key);
  lua_pushinteger(L, Mouse_X);
  lua_pushinteger(L, Mouse_Y);
  lua_pushinteger(L, Mouse_K);
  lua_pushinteger(L, Paintbrush_X);
  lua_pushinteger(L, Paintbrush_Y);
  
  // The event arguments are in the stack.
  // Need to reset "Key" here, so that a key event will not be
  // created again before an actual key repeat occurs.
  Key=0;
  
  return 7;
}

int L_WindowOpen(lua_State* L)
{
  int w, h;
  int nb_args = lua_gettop(L);
  const char *title="";
  
  LUA_ARG_NUMBER(1, "windowopen", w, 10, 310);
  LUA_ARG_NUMBER(2, "windowopen", h, 10, 190);
  if (nb_args >= 3)
  {
    LUA_ARG_STRING(3, "windowopen", title);
    LUA_ARG_LIMIT (3, "windowopen");
  }
  
  if (Windows_open>=7)
  {
    return luaL_error(L, "windowopen: Too many nested windows!");
  }
  
  if (!Cursor_is_visible)
    Display_cursor();
  Open_window(w,h,title);
  Cursor_is_visible=0;

  return 0;
}

int L_WindowClose(lua_State* L)
{
  int nb_args = lua_gettop(L);
  
  LUA_ARG_LIMIT (0, "windowclose");
  
  if (!Windows_open)
    return luaL_error(L, "windowclose: No window is open");
  
  if (!Cursor_is_visible)
    Display_cursor();
  Close_window();
  Cursor_is_visible=0;
  
  return 0;
}

int L_WindowDoDialog(lua_State* L)
{
  int button;
  int nb_args = lua_gettop(L);
  
  LUA_ARG_LIMIT (0, "windowdodialog");
  
  if (!Windows_open)
    return luaL_error(L, "windowdodialog: No window is open");
  
  //Update_window_area(0,0,Window_width, Window_height);
  if (!Cursor_is_visible)
  {
    Display_cursor();
    Cursor_is_visible=1;
  }
  if (Window_needs_update)
  {
    Update_window_area(0,0,Window_width, Window_height);
    Window_needs_update=0;
  }
  button=Window_clicked_button();

  lua_pushinteger(L, button);
  lua_pushinteger(L, Window_attribute2);
  lua_pushinteger(L, Key);
  return 3;
}

int L_WindowButton(lua_State* L)
{
  int x, y, w, h, key=KEY_NONE;
  const char *text="";
  int nb_args = lua_gettop(L);
  
  LUA_ARG_NUMBER(1, "windowbutton", x, 1, Window_width-1);
  LUA_ARG_NUMBER(2, "windowbutton", y, 1, Window_height-1);
  LUA_ARG_NUMBER(3, "windowbutton", w, 1, Window_width-1-x);
  LUA_ARG_NUMBER(4, "windowbutton", h, 1, Window_height-1-y);
  if (nb_args >= 5)
  {
    LUA_ARG_STRING(5, "windowbutton", text);
  }
  if (nb_args >= 6)
  {
    LUA_ARG_NUMBER(6, "windowbutton", key, -1, INT_MAX);
    LUA_ARG_LIMIT (6, "windowbutton");
  }
  
  if (!Windows_open)
    return luaL_error(L, "windowbutton: No window is open");
  
  if (Cursor_is_visible)
  {
    Hide_cursor();
    Cursor_is_visible=0;
  }
  Window_set_normal_button(x, y, w, h, text, 0, 1, key);
  Window_needs_update=1;

  return 0;
}

int L_WindowRepeatButton(lua_State* L)
{
  int x, y, w, h, key=KEY_NONE;
  const char *text="";
  int nb_args = lua_gettop(L);
  
  LUA_ARG_NUMBER(1, "windowrepeatbutton", x, 1, Window_width-1);
  LUA_ARG_NUMBER(2, "windowrepeatbutton", y, 1, Window_height-1);
  LUA_ARG_NUMBER(3, "windowrepeatbutton", w, 1, Window_width-1-x);
  LUA_ARG_NUMBER(4, "windowrepeatbutton", h, 1, Window_height-1-y);
  if (nb_args >= 5)
  {
    LUA_ARG_STRING(5, "windowrepeatbutton", text);
  }
  if (nb_args >= 6)
  {
    LUA_ARG_NUMBER(6, "windowrepeatbutton", key, -1, INT_MAX);
    LUA_ARG_LIMIT (6, "windowrepeatbutton");
  }
  
  if (!Windows_open)
    return luaL_error(L, "windowrepeatbutton: No window is open");
  
  if (Cursor_is_visible)
  {
    Hide_cursor();
    Cursor_is_visible=0;
  }
  Window_set_repeatable_button(x, y, w, h, text, 0, 1, key);
  Window_needs_update=1;

  return 0;
}

int L_WindowInput(lua_State* L)
{
  int x, y, nbchar;
  const char *text=NULL;
  T_Special_button *button;
  int nb_args = lua_gettop(L);
  
  LUA_ARG_NUMBER(1, "windowinput", x, 3, Window_width-3-8);
  LUA_ARG_NUMBER(2, "windowinput", y, 3, Window_height-3-8);
  LUA_ARG_NUMBER(3, "windowinput", nbchar, 1, (Window_width-3-8-x)/8);
  if (nb_args >= 4)
  {
    LUA_ARG_STRING(4, "windowinput", text);
    LUA_ARG_LIMIT (4, "windowinput");
  }
  
  if (!Windows_open)
    return luaL_error(L, "windowinput: No window is open");
  
  if (Cursor_is_visible)
  {
    Hide_cursor();
    Cursor_is_visible=0;
  }
  button = Window_set_input_button(x-2, y-2, nbchar);
  if (text!=NULL)
    Window_input_content(button, text);
  Window_needs_update=1;

  return 0;
}

int L_WindowReadline(lua_State* L)
{
  int x, y, visible_size, max_size, decimal_places=0;
  const char *valuetext;
  double valuenumber;
  char text[255+1]="";
  int result;
  int input_type=0; // see Readline_ex()
  int nb_args = lua_gettop(L);
  
  LUA_ARG_NUMBER(1, "windowreadline", x, 3, Window_width-3-8);
  LUA_ARG_NUMBER(2, "windowreadline", y, 3, Window_height-3-8);
  // arg 3 can be either string or number, it's checked below
  if (nb_args < (3)) return luaL_error(L, "%s: Argument %d is missing.", "windowreadline", (3));
  LUA_ARG_NUMBER(4, "windowreadline", visible_size, 1, (Window_height-3-8)/8);
  LUA_ARG_NUMBER(5, "windowreadline", max_size, 1, 255);
  LUA_ARG_NUMBER(6, "windowreadline", decimal_places, 0, 16);
  LUA_ARG_NUMBER(7, "windowreadline", input_type, 0, 4);
  LUA_ARG_LIMIT (7, "windowreadline");
  
  if (input_type == 3 || input_type == 1)
  {
    // expect number
    LUA_ARG_NUMBER(3, "windowreadline", valuenumber, -DBL_MAX, DBL_MAX);
    Sprint_double(text, valuenumber, decimal_places, 0);
  }
  else
  {
    // expect string
    LUA_ARG_STRING(3, "windowreadline", valuetext);
    if (strlen(valuetext)>255)
      return luaL_error(L, "%s: Argument %d, string too long.", "windowreadline", (3));
    strcpy(text, valuetext);
  }
    
  if (!Windows_open)
    return luaL_error(L, "windowreadline: No window is open");
  
  if (Cursor_is_visible)
  {
    Hide_cursor();
    Cursor_is_visible=0;
  }
  result = Readline_ex(x, y, text, visible_size, max_size, input_type, decimal_places);
  Window_needs_update=1;

  lua_pushboolean(L, result); // 0=ESC, 1= confirm
  lua_pushstring(L, text);
  return 2;
}

int L_WindowPrint(lua_State* L)
{
  int x, y, fg=0, bg=2;
  int colors[4] = {MC_Black, MC_Dark, MC_Light, MC_White};
  const char *text="";
  int nb_args = lua_gettop(L);
  
  LUA_ARG_NUMBER(1, "windowprint", x, 1, Window_width-8);
  LUA_ARG_NUMBER(2, "windowprint", y, 1, Window_height-8);
  LUA_ARG_STRING(3, "windowprint", text);
  if (nb_args >= 4)
  {
    LUA_ARG_NUMBER(4, "windowprint", fg, 0, 3);
  }
  if (nb_args >= 5)
  {
    LUA_ARG_NUMBER(5, "windowprint", bg, 0, 3);
    LUA_ARG_LIMIT (5, "windowprint");
  }
  
  if (!Windows_open)
    return luaL_error(L, "windowprint: No window is open");
  
  if (Cursor_is_visible)
  {
    Hide_cursor();
    Cursor_is_visible=0;
  }

  Print_in_window_limited(x, y, text, (Window_width-x)/8,colors[fg], colors[bg]);
  Window_needs_update=1;
  
  return 0;
}

int L_WindowSlider(lua_State* L)
{
  word x, y, height, nb_elements, nb_elements_visible, initial_position;
  int horizontal;
  int nb_args = lua_gettop(L);
  
  LUA_ARG_LIMIT (7, "windowslider");
  LUA_ARG_BOOL  (7, "windowslider", horizontal);
  if (horizontal)
  {
    LUA_ARG_NUMBER(1, "windowslider", x, 1, Window_width-27);
    LUA_ARG_NUMBER(2, "windowslider", y, 1, Window_height-13);
    LUA_ARG_NUMBER(3, "windowslider", height, 1, Window_width-27-x);
  }
  else
  {
    LUA_ARG_NUMBER(1, "windowslider", x, 1, Window_width-13);
    LUA_ARG_NUMBER(2, "windowslider", y, 1, Window_height-27);
    LUA_ARG_NUMBER(3, "windowslider", height, 1, Window_height-27-y);
  }
  LUA_ARG_NUMBER(4, "windowslider", nb_elements, 1, INT_MAX);
  LUA_ARG_NUMBER(5, "windowslider", nb_elements_visible, 1, nb_elements);
  LUA_ARG_NUMBER(6, "windowslider", initial_position, 0, nb_elements-nb_elements_visible);
  
  if (!Windows_open)
    return luaL_error(L, "windowslider: No window is open");
  
  if (Cursor_is_visible)
  {
    Hide_cursor();
    Cursor_is_visible=0;
  }
  if (horizontal)
    Window_set_horizontal_scroller_button(x, y, height+24, nb_elements, nb_elements_visible, initial_position);
  else
    Window_set_scroller_button(x, y, height+24, nb_elements, nb_elements_visible, initial_position);
  
  Window_needs_update=1;

  return 0;
}

int L_WindowMoveSlider(lua_State* L)
{
  word nb_elements, nb_elements_visible, position;
  int slider_number;
  T_Scroller_button *button = Window_scroller_button_list;
  int nb_args = lua_gettop(L);
  
  if (!Windows_open)
    return luaL_error(L, "windowmoveslider: No window is open");
  
  LUA_ARG_NUMBER(1, "windowmoveslider", slider_number, 1, 256);
  button = Window_scroller_button_list;
  if (! button)
    return luaL_error(L, "windowmoveslider: No slider to move");
  while (slider_number>1)
  {
    slider_number--;    
    button = button->Next;
    if (! button)
      return luaL_error(L, "windowmoveslider: Not so many sliders in this window");
  }
  LUA_ARG_NUMBER(2, "windowmoveslider", nb_elements, 1, INT_MAX);
  LUA_ARG_NUMBER(3, "windowmoveslider", nb_elements_visible, 1, nb_elements);
  LUA_ARG_NUMBER(4, "windowmoveslider", position, 0, nb_elements-nb_elements_visible);
  LUA_ARG_LIMIT (4, "windowmoveslider");
  
  if (Cursor_is_visible)
  {
    Hide_cursor();
    Cursor_is_visible=0;
  }
  button->Nb_elements   =nb_elements;
  button->Nb_visibles   =nb_elements_visible;
  button->Position      =position;
  Compute_slider_cursor_length(button);
  Window_draw_slider(button);
  
  Window_needs_update=1;

  return 0;
}

int L_UpdateScreen(lua_State* L)
{
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (0, "updatescreen");
  
  Update_colors_during_script();
  if (Cursor_is_visible)
    Hide_cursor();
  Display_all_screen();
  Display_cursor();
  Cursor_is_visible=1;
  //Flush_update();

  return 0;
}


int L_StatusMessage(lua_State* L)
{
  const char* msg;
  char msg2[25];
  int len;
  int nb_args = lua_gettop(L);
  LUA_ARG_LIMIT(1,"statusmessage");

  LUA_ARG_STRING(1, "statusmessage", msg);
  len=strlen(msg);
  if (len<=24)
  {
    strcpy(msg2, msg);
    // fill remainder with spaces
    for (;len<24;len++)
      msg2[len]=' ';
  }
  else
  {
    strncpy(msg2, msg, 24);
  }
  msg2[24]='\0';
  if (Cursor_is_visible)
    Hide_cursor();
  Print_in_menu(msg2,0);
  Cursor_is_visible=0;
  return 0;
}


int L_GetLayerCount(lua_State* L)
{
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (0, "layercount");

  lua_pushinteger(L, Main.backups->Pages->Nb_layers);
  return 1;
}


int L_GetSpareLayerCount(lua_State* L)
{
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (0, "layercount");

  lua_pushinteger(L, Spare.backups->Pages->Nb_layers);
  return 1;
}


int L_SelectSpareLayer(lua_State* L)
{
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "selectsparelayer");
  LUA_ARG_NUMBER(1, "selectsparelayer", Spare.current_layer, 0, Spare.backups->Pages->Nb_layers - 1);

  if (Spare.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    if (! ((1 << Spare.current_layer) & Spare.layers_visible))
    {
      Spare.layers_visible |= (1 << Spare.current_layer);
    }
  }
  return 0;
}


int L_SelectLayer(lua_State* L)
{
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "selectlayer");
  LUA_ARG_NUMBER(1, "selectlayer", Main.current_layer, 0, Main.backups->Pages->Nb_layers - 1);

  Backup_if_necessary(L, Main.current_layer);
  // 
  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
  {
    if (! ((1 << Main.current_layer) & Main.layers_visible))
    {
      Main.layers_visible |= (1 << Main.current_layer);
      Redraw_layered_image();
    }
    else
    {
      Update_depth_buffer(); // Only need the depth buffer
    }
  }
  return 0;
}


int L_FinalizePicture(lua_State* L)
{
  int nb_args=lua_gettop(L);
  
  LUA_ARG_LIMIT (0, "finalizepicture");
  
  Update_colors_during_script();
  if (Is_backed_up)
  {
    End_of_modification();
  }
  Is_backed_up = 0;
  Main_backup_page = Main.backups->Pages;
  Main_backup_screen = Main_screen;
  Register_main_readonly(L);
  lua_register(L,"setcolor",L_SetColor_unsaved);
  
  return 0;
}


int L_GetFileName(lua_State* L)
{
  int nb_args=lua_gettop(L);

  LUA_ARG_LIMIT (0, "getfilename");

#if defined(WIN32)
  if (Main.backups->Pages->Filename_unicode == NULL || Main.backups->Pages->Filename_unicode[0] == 0)
    lua_pushstring(L, Main.backups->Pages->Filename);
  else
  {
    // convert Filename_unicode to UTF8 (Lua strings are 8bits)
    size_t unicode_len = Unicode_strlen(Main.backups->Pages->Filename_unicode);
    int len = WideCharToMultiByte(CP_UTF8, 0, Main.backups->Pages->Filename_unicode, unicode_len, NULL, 0, NULL, NULL);
    if (len <= 0)
    {
      GFX2_Log(GFX2_WARNING, "WideCharToMultiByte() failed\n");
      lua_pushstring(L, Main.backups->Pages->Filename);
    }
    else
    {
      char * utf8name = (char *)GFX2_malloc(len);
      if (utf8name == NULL)
        lua_pushstring(L, Main.backups->Pages->Filename);
      else
      {
        if (WideCharToMultiByte(CP_UTF8, 0, Main.backups->Pages->Filename_unicode, unicode_len, utf8name, len, NULL, NULL) > 0)
          lua_pushlstring(L, utf8name, len);
        else
        {
          DWORD error = GetLastError();
          GFX2_Log(GFX2_WARNING, "WideCharToMultiByte() failed error=%u\n", error);
          lua_pushstring(L, Main.backups->Pages->Filename);
        }
        free(utf8name);
      }
    }
  }
  // still using the "short name" directory path at the moment
  lua_pushstring(L, Main.backups->Pages->File_directory);
#else
  lua_pushstring(L, Main.backups->Pages->Filename);
  lua_pushstring(L, Main.backups->Pages->File_directory);
#endif
  return 2;
}

/// Run a script while changing the current directory
int L_Run(lua_State* L)
{
  const char * script_arg;
  const char * message;
  char * saved_directory;
  char * full_path;
  const char * path;
  char * file_name;
  static int nested_calls = 0;
    
  int nb_args = lua_gettop(L);
  
  LUA_ARG_LIMIT (1, "run");
  LUA_ARG_STRING(1, "run", script_arg);

  nested_calls++;
  if (nested_calls > 100)
    return luaL_error(L, "run: too many nested calls (100)");
    
  // store the current directory (on the stack)
  saved_directory = Get_current_directory(NULL, NULL, 0);

  full_path = strdup(script_arg);
  #if defined (__AROS__)
  // Convert path written on Linux/Windows norms to AROS norms :
  // Each element like ../ and ..\ is replaced by /
  // Each path separator \ is replaced by /
  {
    const char *src = script_arg;
    char *dst = full_path;
    char c;
    do
    {
      while (src[0]=='.' && src[1]=='.' && (src[2]=='/' || src[2]=='\\'))
      {
        *dst='/';
        dst++;
        src+=3;
      }
      // copy until / or \ or NUL included
      do
      {
        c=*src;
        *dst=c=='\\' ? '/' : c; // Replace \ by / anyway
        src++;
        dst++;
      } while (c != '\0' && c != '/' && c != '\\');
    } while (c!='\0');
  }
  #else
  {
    // Convert any number of '\' in the path to '/'
    // It is not useful on Windows, but does no harm (both path separators work)
    // On Linux/Unix, this ensures that scripts written and tested on Windows
    // work similarly.
    char *pos;
    for (pos = strchr(full_path, '\\'); pos != NULL; pos = strchr(pos, '\\'))
      *pos = '/';
  }
  #endif
  file_name = Find_last_separator(full_path);
  if (file_name != NULL)
  {
    path = full_path;
    *file_name = '\0';
    file_name++;
    if (path[0] == '\0')  // the file was in ROOT directory
      path = PATH_SEPARATOR;
    if (!Directory_exists(path))
    {
      free(saved_directory);
      free(full_path);
      return luaL_error(L, "run: directory of script doesn't exist");
    }
    Change_directory(path);
  }
  else
  {
    path = NULL;  // only file name
    file_name = full_path;
  }

  if (luaL_loadfile(L, file_name) != 0)
  {
    int r;
    nb_args = lua_gettop(L);
    if (nb_args>0 && (message = lua_tostring(L, nb_args))!=NULL)
      r = luaL_error(L, message);
    else
      r = luaL_error(L, "run: Unknown error loading script %s", file_name);
    Change_directory(saved_directory);
    free(saved_directory);
    free(full_path);
    return r;
  }
  free(full_path);
  if (lua_pcall(L, 0, 0, 0) != 0)
  {
    int r;
    nb_args = lua_gettop(L);
    // We COULD build the call stack, but I think this would be more
    // confusing than helpful.

    if (nb_args>0 && (message = lua_tostring(L, nb_args))!=NULL)
      r = luaL_error(L, message);
    else
      r = luaL_error(L, "run: Unknown error running script!");
    Change_directory(saved_directory);
    free(saved_directory);
    return r;
  }
  nested_calls--;
  // restore directory
  Change_directory(saved_directory);
  free(saved_directory);
  return 0;
}

// "unsaved" version of the bindings

DECLARE_UNSAVED(L_ClearPicture)
DECLARE_UNSAVED(L_DrawCircle)
DECLARE_UNSAVED(L_DrawDisk)
DECLARE_UNSAVED(L_DrawFilledRect)
DECLARE_UNSAVED(L_DrawLine)
DECLARE_UNSAVED(L_PutPicturePixel)

/// Bindings for screen-drawing Lua functions, if the current image is backed up.
void Register_main_writable(lua_State* L)
{
  lua_register(L,"putpicturepixel",L_PutPicturePixel);
  lua_register(L,"drawline",L_DrawLine);
  lua_register(L,"drawfilledrect",L_DrawFilledRect);
  lua_register(L,"drawcircle",L_DrawCircle);
  lua_register(L,"drawdisk",L_DrawDisk);
  lua_register(L,"clearpicture",L_ClearPicture);
}

/// Bindings for screen-drawing Lua functions, if the current image is not backed up yet.
void Register_main_readonly(lua_State* L)
{
  lua_register(L,"putpicturepixel",L_PutPicturePixel_unsaved);
  lua_register(L,"drawline",L_DrawLine_unsaved);
  lua_register(L,"drawfilledrect",L_DrawFilledRect_unsaved);
  lua_register(L,"drawcircle",L_DrawCircle_unsaved);
  lua_register(L,"drawdisk",L_DrawDisk_unsaved);
  lua_register(L,"clearpicture",L_ClearPicture_unsaved);
}


// Handlers for window internals
T_Fileselector Scripts_selector;

// Callback to display a skin name in the list
void Draw_script_name(word x, word y, word index, byte highlighted)
{
  T_Fileselector_item * current_item;

  if (Scripts_selector.Nb_elements)
  {
    byte fg, bg;
    
    current_item = Get_item_by_index(&Scripts_selector, index);

    if (current_item->Type==0) // Files
    {
      fg=(highlighted)?MC_White:MC_Light;
      bg=(highlighted)?MC_Dark:MC_Black;
    }
    else if (current_item->Type==1) // Directories
    {
      fg=(highlighted)?MC_Light:MC_Dark;
      bg=(highlighted)?MC_Dark:MC_Black;
    }
    else // Drives
    {
      fg=(highlighted)?MC_Light:MC_Dark;
      bg=(highlighted)?MC_Dark:MC_Black;
      
      if (current_item->Icon != ICON_NONE)
	  {
      	Window_display_icon_sprite(x,y,current_item->Icon);
      	x+=8;
	  }
    }
    
    if (current_item->Unicode_short_name != NULL)
      Print_in_window_unicode(x, y, current_item->Unicode_short_name, fg, bg);
    else
      Print_in_window(x, y, current_item->Short_name, fg,bg);

    Update_window_area(x,y,NAME_WIDTH*8,8);
  }
}

///
/// Displays first lines of comments from a lua script in the window.
void Draw_script_information(T_Fileselector_item * script_item, const char *full_directory)
{
  FILE *script_file;
  char * full_name;
  char text_block[3][DESC_WIDTH+1];
  int x, y;
  int i;

  // Blank the target area
  Window_rectangle(7, FILESEL_Y + 89, DESC_WIDTH*6+2, 4*8, MC_Black);

  if (script_item && script_item->Full_name && script_item->Full_name[0] != '\0')
  {
    full_name = Filepath_append_to_dir(full_directory, script_item->Full_name);

    x = 0;
    y = 0;
    text_block[0][0] = text_block[1][0] = text_block[2][0] = '\0';

    if (script_item->Type == FSOBJECT_FILE)
    {
      // Start reading
      script_file = fopen(full_name, "r");
      if (script_file != NULL)
      {
        int c;
        c = fgetc(script_file);
        while (c != EOF && y<3)
        {
          if (c == '\n')
          {
            if (x<2)
              break; // Carriage return without comment: Stopping
            y++;
            x=0;
          }
          else if (x==0 || x==1)
          {
            if (c != '-')
              break; // Non-comment line was encountered. Stopping.
            x++;
          }
          else
          {
            if (x < DESC_WIDTH+2)
            {
              // Adding character
              text_block[y][x-2] = (c<32 || c>255) ? ' ' : c;
              text_block[y][x-1] = '\0';
            }
            x++;
          }
          // Read next
          c = fgetc(script_file);
        }
        fclose(script_file);
      }
      Print_help(8, FILESEL_Y + 89   , text_block[0], 'N', 0, 0);
      Print_help(8, FILESEL_Y + 89+ 8, text_block[1], 'N', 0, 0);
      Print_help(8, FILESEL_Y + 89+16, text_block[2], 'N', 0, 0);

      // Display a line with the keyboard shortcut
      Print_help(8, FILESEL_Y + 89+24, "Key:", 'N', 0, 0);
      for (i=0; i<10; i++)
        if (Bound_script[i]!=NULL && !strcmp(Bound_script[i], full_name))
          break;

      if (i<10)
      {
        const char *shortcut;
        shortcut = Keyboard_shortcut_value(SPECIAL_RUN_SCRIPT_1+i);
        Print_help(8+4*6, FILESEL_Y + 89+24, shortcut, 'K', 0, strlen(shortcut));
      }
      else
      {
        Print_help(8+4*6, FILESEL_Y + 89+24, "None", 'K', 0, 4);
      }
    }
    free(full_name);
  }

  Update_window_area(8, FILESEL_Y + 89, DESC_WIDTH*6+2, 4*8);
}

// Add a script to the list
static void Add_script(void * pdata, const char *file_name, const word *unicode_name, byte is_file, byte is_directory, byte is_hidden)
{
  int len;
  T_Fileselector_item * item;
  (void)pdata;

  if (is_file)
  {
    // Only files ending in ".lua"
    len=strlen(file_name);
    if (len<=4 || strcasecmp(file_name+len-4, ".lua"))
      return;
    // Hidden
    if (is_hidden && !Config.Show_hidden_files)
      return;
      
    item = Add_element_to_list(&Scripts_selector, file_name, Format_filename(file_name, NAME_WIDTH+1, 0), FSOBJECT_FILE, ICON_NONE);
    if (item != NULL && unicode_name != NULL)
    {
      item->Unicode_full_name = Unicode_strdup(unicode_name);
      item->Unicode_short_name = Unicode_strdup(Format_filename_unicode(unicode_name, NAME_WIDTH+1, 1));
    }
  }
  else if (is_directory)
  {
    // Ignore current directory
    if ( !strcmp(file_name, "."))
      return;
    // Ignore parent directory entry
    if (!strcmp(file_name, PARENT_DIR))
      return;
    // Hidden
    if (is_hidden && !Config.Show_hidden_directories)
      return;
    
    item = Add_element_to_list(&Scripts_selector, file_name, Format_filename(file_name, NAME_WIDTH+1, 1), FSOBJECT_DIR, ICON_NONE);
    if (item != NULL && unicode_name != NULL)
    {
      item->Unicode_full_name = Unicode_strdup(unicode_name);
      item->Unicode_short_name = Unicode_strdup(Format_filename_unicode(unicode_name, NAME_WIDTH+1, 1));
    }
  }
}

void Highlight_script(T_Fileselector *selector, T_List_button *list, const char *selected_file)
{
  short index;

  index=Find_file_in_fileselector(selector, selected_file);
  if (index < 0)
    index = 0;  // 1st item if selected_file is not found
  Locate_list_item(list, index);
}

static char * Last_run_script = NULL;

// Before: Cursor hidden
// After: Cursor shown
void Run_script(const char *script_subdirectory, const char *script_filename)
{
  lua_State* L;
  const char* message;
  byte  old_cursor_shape = Cursor_shape;
  char * path;
  int original_image_width = Main.image_width;
  int original_image_height = Main.image_height;
  int original_current_layer = Main.current_layer;

  // Some scripts are slow
  Cursor_shape = CURSOR_SHAPE_HOURGLASS;
  Display_cursor();
  Flush_update();
  Cursor_is_visible=1;

  free(Last_run_script);
  if (script_subdirectory && script_subdirectory[0]!='\0')
    Last_run_script = Filepath_append_to_dir(script_subdirectory, script_filename);
  else
    Last_run_script = strdup(script_filename);
  
  // This chdir is for the script's sake. Grafx2 itself will (try to)
  // not rely on what is the system's current directory.
  path = Extract_path(NULL, Last_run_script);
  Change_directory(path);
  free(path);

  L = luaL_newstate(); // used to be lua_open() on Lua 5.1, deprecated on 5.2

  /// @todo as the value doesn't vary, this should be
  /// done once at the start of the program
  path = GFX2_malloc(strlen(Data_directory) + strlen(SCRIPTS_SUBDIRECTORY) + strlen(LUALIB_SUBDIRECTORY) + 5 + 3 * strlen(PATH_SEPARATOR) + 9 + 1);
  if (path == NULL)
    return;
  strcpy(path, Data_directory);
  Append_path(path, SCRIPTS_SUBDIRECTORY, NULL);
  Append_path(path, LUALIB_SUBDIRECTORY, NULL);
  Append_path(path, "?.lua", NULL);
  // SetEnvironmentVariableA() won't work because lua uses getenv()
#if defined(_MSC_VER)
  if (_putenv_s("LUA_PATH", path) < 0)
    GFX2_Log(GFX2_ERROR, "_putenv_s(\"LUA_PATH\", \"%s\") failed\n", path);
#elif defined(WIN32)
  // Mingw has neither setenv() nor _putenv_s()
  memmove(path + 9, path, strlen(path) + 1);
  memcpy(path, "LUA_PATH=", 9);
  if (putenv(path) < 0)
    GFX2_Log(GFX2_ERROR, "putenv(\"%s\") failed\n", path);
#else
  /* From linux man :
   * This function makes
   * copies of the strings pointed to by name and value (by contrast with
   * putenv(3)).
   */
  if (setenv("LUA_PATH", path, 1) < 0)
    GFX2_Log(GFX2_ERROR, "setenv(\"LUA_PATH\", \"%s\", 1) failed\n", path);
#endif
  free(path);
  
  // Drawing
  lua_register(L,"putbrushpixel",L_PutBrushPixel);
  lua_register(L,"putsparepicturepixel",L_PutSparePicturePixel);
  Register_main_readonly(L);

  // Reading pixels
  lua_register(L,"getbrushpixel",L_GetBrushPixel);
  lua_register(L,"getbrushbackuppixel",L_GetBrushBackupPixel);
  lua_register(L,"getpicturepixel",L_GetPicturePixel);
  lua_register(L,"getlayerpixel",L_GetLayerPixel);
  lua_register(L,"getbackuppixel",L_GetBackupPixel);
  lua_register(L,"getsparelayerpixel",L_GetSpareLayerPixel);
  lua_register(L,"getsparepicturepixel",L_GetSparePicturePixel);

  // Sizes
  lua_register(L,"setbrushsize",L_SetBrushSize);
  lua_register(L,"setpicturesize",L_SetPictureSize);
  lua_register(L,"setsparepicturesize",L_SetSparePictureSize);

  lua_register(L,"getbrushsize",L_GetBrushSize);
  lua_register(L,"getpicturesize",L_GetPictureSize);
  lua_register(L,"getsparepicturesize",L_GetSparePictureSize);

  // color and palette
  lua_register(L,"getforecolor",L_GetForeColor);
  lua_register(L,"getbackcolor",L_GetBackColor);
  lua_register(L,"gettranscolor",L_GetTransColor);

  lua_register(L,"setcolor",L_SetColor_unsaved);
  lua_register(L,"setforecolor",L_SetForeColor);
  lua_register(L,"setbackcolor",L_SetBackColor);

  lua_register(L,"getcolor",L_GetColor);
  lua_register(L,"getbackupcolor",L_GetBackupColor);
  lua_register(L,"getsparecolor",L_GetSpareColor);
  lua_register(L,"getsparetranscolor",L_GetSpareTransColor);
  
  lua_register(L,"matchcolor",L_MatchColor);
  lua_register(L,"matchcolor2",L_MatchColor2);

  // layers
  lua_register(L,"selectlayer",L_SelectLayer);
  lua_register(L,"selectsparelayer",L_SelectSpareLayer);
  lua_register(L,"getlayercount",L_GetLayerCount);
  lua_register(L,"getsparelayercount",L_GetSpareLayerCount);

  // ui
  lua_register(L,"inputbox",L_InputBox);
  lua_register(L,"messagebox",L_MessageBox);
  lua_register(L,"statusmessage",L_StatusMessage);
  lua_register(L,"selectbox",L_SelectBox);

  // misc. stuff
  lua_register(L,"wait",L_Wait);
  lua_register(L,"waitbreak",L_WaitBreak);
  lua_register(L,"waitinput",L_WaitInput);
  lua_register(L,"updatescreen",L_UpdateScreen);
  lua_register(L,"finalizepicture",L_FinalizePicture);
  lua_register(L,"getfilename",L_GetFileName);
  lua_register(L,"run",L_Run);
  
  // dialog
  lua_register(L,"windowopen",L_WindowOpen);
  lua_register(L,"windowclose",L_WindowClose);
  lua_register(L,"windowdodialog",L_WindowDoDialog);
  lua_register(L,"windowbutton",L_WindowButton);
  lua_register(L,"windowrepeatbutton",L_WindowRepeatButton);
  lua_register(L,"windowinput",L_WindowInput);
  lua_register(L,"windowreadline",L_WindowReadline);
  lua_register(L,"windowprint",L_WindowPrint);
  lua_register(L,"windowslider",L_WindowSlider);
  lua_register(L,"windowmoveslider",L_WindowMoveSlider);
  
  // Load all standard libraries
  luaL_openlibs(L);
  
  /*
  luaopen_base(L);
  //luaopen_package(L); // crashes on Windows, for unknown reason
  luaopen_table(L);
  //luaopen_io(L); // crashes on Windows, for unknown reason
  //luaopen_os(L);
  luaopen_string(L);
  luaopen_math(L);
  //luaopen_debug(L);
  */

  // TODO The script may modify the picture, so we do a backup here.
  // If the script is only touching the brush, this isn't needed...
  // The backup also allows the script to read from it to make something
  // like a feedback off effect (convolution matrix comes to mind).
  //Backup();
  Is_backed_up = 0;
  Main_backup_page = Main.backups->Pages;
  Main_backup_screen = Main_screen;
  Backup_the_spare(LAYER_ALL);

  Palette_has_changed=0;
  Brush_was_altered=0;
  Original_back_color=Back_color;
  Original_fore_color=Fore_color;

  // Backup the brush
  Brush_backup=(byte *)GFX2_malloc(((long)Brush_height)*Brush_width);
  Brush_backup_width = Brush_width;
  Brush_backup_height = Brush_height;
  
  if (Brush_backup == NULL)
  {
    Verbose_message("Error!", "Out of memory!");
  }
  else 
  {
    memcpy(Brush_backup, Brush, ((long)Brush_height)*Brush_width);
  
    if (luaL_loadfile(L, Last_run_script) != 0)
    {
      int stack_size;
      stack_size= lua_gettop(L);
      if (stack_size>0 && (message = lua_tostring(L, stack_size))!=NULL)
        Verbose_message("Error!", message);
      else
        Warning_message("Unknown error loading script!");
    }
    else if (lua_pcall(L, 0, 0, 0) != 0)
    {
      int stack_size;
      stack_size= lua_gettop(L);
      
      Update_colors_during_script();
      
      if (stack_size>0 && (message = lua_tostring(L, stack_size))!=NULL)
        Verbose_message("Error running script", message);
      else
        Warning_message("Unknown error running script!");
    }
    // Clean up any remaining dialog windows
    while (Windows_open)
    {
      Close_window();
      Display_cursor();
    }
  }
  // Cleanup
  free(Brush_backup);
  Brush_backup=NULL;
  Update_colors_during_script();
  if (Is_backed_up)
    End_of_modification();
	Print_in_menu("                        ",0);

  lua_close(L);
  
  if (Brush_was_altered)
  {
    // Copy Brush to original
    memcpy(Brush_original_pixels, Brush, (long)Brush_width*Brush_height);
    Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
  }

  Hide_cursor();
  Display_all_screen();
  if (original_current_layer != Main.current_layer)
    Display_layerbar(); // current layer has changed, buttons need to be updated
  
  // Update tilemap if image size has changed
  if (original_image_width!=Main.image_width
   || original_image_height!=Main.image_height)
  {
    Tilemap_update();
  }

  // Update changed pen colors.
  if (Back_color!=Original_back_color || Fore_color!=Original_fore_color)
  {
    // This is done at end of script, in case somebody would use the
    // functions in a custom window.
    if (Back_color!=Original_back_color)
    {
      byte new_color = Back_color;
      Back_color = Original_back_color;
      Set_back_color(new_color);
    }
    if (Fore_color!=Original_fore_color)
    {
      byte new_color = Fore_color;
      Fore_color = Original_fore_color;
      Set_fore_color(new_color);
    }
    
  }
  Cursor_shape=old_cursor_shape;
  Display_cursor();
}

void Run_numbered_script(byte index)
{

  if (index>=10)
    return;
  if (Bound_script[index]==NULL)
    return;
    
  Hide_cursor();
  Run_script(NULL, Bound_script[index]);
}               

void Repeat_script(void)
{

  if (Last_run_script==NULL || Last_run_script[0]=='\0')
  {
    Warning_message("No script to repeat.");
    return;
  }
  
  Hide_cursor();
  Run_script(NULL, Last_run_script);
}

void Set_script_shortcut(T_Fileselector_item * script_item, const char *full_directory)
{
  int i;
  char * full_name;
  
  if (script_item && script_item->Full_name && script_item->Full_name[0] != '\0')
  {
    full_name = Filepath_append_to_dir(full_directory, script_item->Full_name);
    
    // Find if it already has a shortcut
    for (i=0; i<10; i++)
      if (Bound_script[i] != NULL && !strcmp(Bound_script[i], full_name))
        break;
    if (i<10)
    {
      // Existing shortcut
      free(full_name);
    }
    else
    {
      // Try to find a "free" one.
      for (i=0; i<10; i++)
        if (Bound_script[i]==NULL
          || !Has_shortcut(SPECIAL_RUN_SCRIPT_1+i)
          || !File_exists(Bound_script[i]))
          break;
      if (i<10)
      {
        free(Bound_script[i]);
        Bound_script[i] = full_name;
      }
      else
      {
        Warning_message("Already 10 scripts have shortcuts.");
        free(full_name);
        return;
      }
    }
    Window_set_shortcut(SPECIAL_RUN_SCRIPT_1+i);
    if (!Has_shortcut(SPECIAL_RUN_SCRIPT_1+i))
    {
      // User cancelled or deleted all shortcuts
      free(Bound_script[i]);
      Bound_script[i]=NULL;
    }
    // Refresh display
    Hide_cursor();
    Draw_script_information(script_item, full_directory);
    Display_cursor();
  }
}

void Reload_scripts_list(void)
{
  // Reinitialize the list
  Free_fileselector_list(&Scripts_selector);
  if (Config.Scripts_directory == NULL || Config.Scripts_directory[0]=='\0')
  {
    Read_list_of_drives(&Scripts_selector,NAME_WIDTH+1);
  }
  else
  {
    Add_element_to_list(&Scripts_selector, PARENT_DIR, Format_filename(PARENT_DIR, NAME_WIDTH+1, 1), FSOBJECT_DIR, ICON_NONE);
    // Add each found file to the list
    For_each_directory_entry(Config.Scripts_directory, NULL, Add_script);
  }
  // Sort it
  Sort_list_of_files(&Scripts_selector); 
  //
}

void Button_Brush_Factory(void)
{
  static char * selected_file = NULL; // currently selected file in factory file selector
  
  short clicked_button;
  T_List_button* scriptlist;
  T_Scroller_button* scriptscroll;
  T_Special_button* scriptarea;
  T_Fileselector_item *item;
  int last_selected_item=-1;
  char displayed_path[DESC_WIDTH+1];
  int q;
  
  Reload_scripts_list();

  Open_window(33+8*NAME_WIDTH, 180, "Brush Factory");
  
  Window_set_normal_button(85, 161, 67, 14, "Cancel", 0, 1, KEY_ESC); // 1

  Window_display_frame_in(6, FILESEL_Y - 2, NAME_WIDTH*8+4, 84); // File selector
  // Fileselector
  scriptarea=Window_set_special_button(8, FILESEL_Y + 0, NAME_WIDTH*8, 80,0); // 2
  // Scroller for the fileselector
  scriptscroll = Window_set_scroller_button(NAME_WIDTH*8+14, FILESEL_Y - 1, 82,
      Scripts_selector.Nb_elements,10, 0); // 3
  scriptlist = Window_set_list_button(scriptarea,scriptscroll,Draw_script_name, 0); // 4

  Window_set_normal_button(10, 161, 67, 14, "Run", 0, 1, KEY_RETURN); // 5

  Window_display_frame_in(6, FILESEL_Y + 88, DESC_WIDTH*6+4, 4*8+2); // Descr.
  Window_set_special_button(7, FILESEL_Y + 89+24,DESC_WIDTH*6,8,0); // 6
  
  // Box around path (slightly expands up left)
  Window_rectangle(8, FILESEL_Y - 13, DESC_WIDTH*6+2, 9, MC_Black);
  
  while (1)
  {
    // Locate selected file in view
    Highlight_script(&Scripts_selector, scriptlist, selected_file);
    // Update the scroller position
    scriptscroll->Position=scriptlist->List_start;
    Window_draw_slider(scriptscroll);
    
    Window_redraw_list(scriptlist);
    // Display current path:
    q = Config.Scripts_directory == NULL ? 0 : strlen(Config.Scripts_directory);
    if (q<=DESC_WIDTH)
    {
      if (q > 0)
        strcpy(displayed_path, Config.Scripts_directory);
      for (; q < DESC_WIDTH; q++)
        displayed_path[q]=' ';
      displayed_path[q]='\0';
    }
    else
    {
      strcpy(displayed_path, Config.Scripts_directory+q-DESC_WIDTH);
      displayed_path[0] = ELLIPSIS_CHARACTER;
    }
    Print_help(9, FILESEL_Y - 12, displayed_path, 'N', 0, 0);
    
    Draw_script_information(Get_item_by_index(&Scripts_selector,
      scriptlist->List_start + scriptlist->Cursor_position), Config.Scripts_directory);
    
    Update_window_area(0, 0, Window_width, Window_height);
    Display_cursor();
    // Wait for mouse release (needed for example after a double-click
    // that navigates to a subdirectory)
    while (last_selected_item==-1 && Mouse_K)
    {
      Get_input(20);
    }
    
    Reset_quicksearch();
  
    do
    {
      clicked_button = Window_clicked_button();
      if (Key==KEY_BACKSPACE && Config.Scripts_directory != NULL && Config.Scripts_directory[0]!='\0')
      {
        // Make it select first entry (parent directory)
        scriptlist->List_start=0;
        scriptlist->Cursor_position=0;
        clicked_button=5;
      }
      if (Is_shortcut(Key,0x100+BUTTON_HELP))
        Window_help(BUTTON_BRUSH_EFFECTS, "BRUSH FACTORY");
      else if (Is_shortcut(Key,0x200+BUTTON_BRUSH_EFFECTS))
        clicked_button=1; // Cancel
      // Quicksearch
      if (clicked_button==4)
        Reset_quicksearch();
      else if (clicked_button==0 && Key_ANSI)
        clicked_button=Quicksearch_list(scriptlist, &Scripts_selector);
  
      switch (clicked_button)
      {
        case 2: // Double-click an entry in script list
          clicked_button=5;
          break;
        case 4: // Select script
          last_selected_item = scriptlist->List_start + scriptlist->Cursor_position;
          Hide_cursor();
          Draw_script_information(Get_item_by_index(&Scripts_selector,
            scriptlist->List_start + scriptlist->Cursor_position), Config.Scripts_directory);
          Display_cursor();
          break;
        
        case 6:
          Set_script_shortcut(Get_item_by_index(&Scripts_selector,
            scriptlist->List_start + scriptlist->Cursor_position), Config.Scripts_directory);
          break;
          
        default:
          break;
      }
      
    } while (clicked_button != 1 && clicked_button != 5 && !Quit_is_required);

    if (Quit_is_required)
      clicked_button = 1;
    // Cancel
    if (clicked_button==1)
      break;
      
    // OK
    if (Scripts_selector.Nb_elements == 0)
    {
      // No items : same as Cancel
      clicked_button=1;
      break;
    }
  
    // Examine selected file
    item = Get_item_by_index(&Scripts_selector,
      scriptlist->List_start + scriptlist->Cursor_position);
    
    if (item->Type == FSOBJECT_FILE)
    {
      free(selected_file);
      selected_file = strdup(item->Full_name);
      break;
    }
    else if (item->Type == FSOBJECT_DIR || item->Type == FSOBJECT_DRIVE)
    {
      if (item->Type == FSOBJECT_DRIVE)
      {
        // Selecting one drive root
        free(selected_file);
        selected_file = strdup(PARENT_DIR);
        free(Config.Scripts_directory);
        Config.Scripts_directory = strdup(item->Full_name);
      }
      else
      {
        // Going down one or up by one directory
        if (strcmp(item->Full_name, PARENT_DIR) == 0)
        {
          char * separator_pos = Find_last_separator(Config.Scripts_directory);
          if (separator_pos != NULL && separator_pos[1] == '\0')
          {
            // remove trailing separator
            separator_pos[0] = '\0';
            separator_pos = Find_last_separator(Config.Scripts_directory);
          }
          free(selected_file);
          if (separator_pos == NULL)
          {
            selected_file = Config.Scripts_directory; // steal heap buffer
            Config.Scripts_directory = NULL;
          }
          else
          {
            selected_file = strdup(separator_pos + 1);
            separator_pos[0] = '\0';
          }
        }
        else
        {
          char * new_dir = Filepath_append_to_dir(Config.Scripts_directory, item->Full_name);
          if (new_dir != NULL)
          {
            free(Config.Scripts_directory);
            Config.Scripts_directory = new_dir;
          }
        }
      }

      // No break: going back up to beginning of loop
      
      Reload_scripts_list();
      
      scriptlist->Scroller->Nb_elements=Scripts_selector.Nb_elements;
      Compute_slider_cursor_length(scriptlist->Scroller);
      last_selected_item = -1;
      Hide_cursor();
    }
  }
  
  Close_window();
  Unselect_button(BUTTON_BRUSH_EFFECTS);

  if (clicked_button == 5 && selected_file != NULL) // Run the script
  {
    Run_script(Config.Scripts_directory, selected_file);
  }
  else
  {
    Display_cursor();
  }
  Free_fileselector_list(&Scripts_selector);
}

#else // NOLUA

void Button_Brush_Factory(void)
{
    Verbose_message("Error!", "The brush factory is not available in this build of GrafX2.");
}

///
/// Returns a string stating the included Lua engine version,
/// or "Disabled" if Grafx2 is compiled without Lua.
const char * Lua_version(void)
{
  return "Disabled";
}

#endif
