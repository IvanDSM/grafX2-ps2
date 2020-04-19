/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2011 Pawel Góralski
    Copyright 2008 Yves Rizoud
    Copyright 2007-2017 Adrien Destugues (PulkoMandy)
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

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
    #include <proto/dos.h>
    #include <sys/types.h>
#elif defined(__WIN32__) || defined(WIN32)
    #include <windows.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#if !defined(_MSC_VER) && !defined(_EE)
#include <strings.h>
#else
#define strdup _strdup
#define strncasecmp _strnicmp
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif
#include <stdlib.h>
#if !defined(__VBCC__) && !defined(_MSC_VER)
#include <unistd.h>
#endif
#include <ctype.h>
#include <sys/stat.h>

#include "const.h"
#include "struct.h"
#include "global.h"
#include "misc.h"
#include "graph.h"
#include "engine.h"
#include "readline.h"
#include "filesel.h"
#include "fileseltools.h"
#include "loadsave.h"
#include "init.h"
#include "buttons.h"
#include "operatio.h"
#include "pages.h"
#include "palette.h"
#include "errors.h"
#include "readini.h"
#include "saveini.h"
#include "shade.h"
#include "io.h"
#include "help.h"
#include "text.h"
#include "screen.h"
#include "windows.h"
#include "brush.h"
#include "input.h"
#include "special.h"
#include "tiles.h"
#include "setup.h"
#include "unicode.h"
#include "keycodes.h"

#if defined(__amigaos4__) || defined(__AROS__) || defined(__MORPHOS__) || defined(__amigaos__)
    #include <proto/dos.h>
    #include <dirent.h>
#elif defined(__MINT__)
    #include <mint/sysbind.h>
    #include <dirent.h>
#elif defined(WIN32)
#ifndef _MSC_VER
    #include <dirent.h>
#endif
    #include <windows.h>
#else
    #include <dirent.h>
#endif

extern char Program_version[]; // generated in pversion.c

extern short Old_MX;
extern short Old_MY;


//-- MODELE DE BOUTON DE MENU ------------------------------------------------
/*
void Bouton_***(void)
{
  short clicked_button;

  Open_window(310,190,"***");

  Window_set_normal_button(103,137,80,14,"OK",0,1,KEY_RETURN); // 1
  Window_set_scroller_button(18,44,88,16,4,0);             // 2

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
  }
  while (clicked_button!=1);

  Close_window();
  Unselect_button(BOUTON_***);
  Display_cursor();
}
*/

void Message_out_of_memory(void)
{
  short clicked_button;

  Open_window(216,76,"Not enough memory!");

  Print_in_window(8,20,"Please consult the manual",MC_Black,MC_Light);
  Print_in_window(24,28,"to know how to obtain",MC_Black,MC_Light);
  Print_in_window(36,36,"more memory space.",MC_Black,MC_Light);
  Window_set_normal_button(60,53,40,14,"OK",1,1,KEY_RETURN); // 1
  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
    clicked_button=Window_clicked_button();
  while ((clicked_button<=0) && (Key!=KEY_ESC) && (Key!=KEY_o));

  if(clicked_button<=0) Key=0;
  Close_window();
  Display_cursor();
}


void Button_Message_initial(void)
{
  char  str[30];
  int   x_pos,offs_y,x,y;
  int clicked_button=0;
  
  snprintf(str,sizeof(str),"GrafX2 version %s",Program_version);
  Open_window(260,172,str);

  Window_display_frame_in(10,20,239,62);
  Window_rectangle(11,21,237,60,MC_Black);
  for (y=23,offs_y=0; y<79; offs_y+=231,y++)
    for (x=14,x_pos=0; x_pos<231; x_pos++,x++)
      Pixel_in_window(x,y,Gfx->Logo_grafx2[offs_y+x_pos]);

  Print_in_window(130-4*26,88,"Copyright (c) 2007-2020 by",MC_Dark,MC_Light);
  Print_in_window(130-4*23,100,"the Grafx2 project team",MC_Black,MC_Light);
  Print_in_window(130-4*26,112,"Copyright (c) 1996-2001 by",MC_Dark,MC_Light);
  Print_in_window(130-4*13,122,"Sunset Design",MC_Black,MC_Light);
  //Print_in_window( 120-4*13,128,"(placeholder)",MC_Dark,MC_Light);
  Print_in_window(130-4*16,136,"http://grafx2.tk",MC_Dark,MC_Light);

  Window_set_normal_button(56, 151, 71, 14, "Anim", 1, (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION), KEY_a);
  Window_set_normal_button(133, 151, 71, 14, "Layers", 1, (Main.backups->Pages->Image_mode != IMAGE_MODE_LAYERED), KEY_l);

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  while(!Mouse_K && !Key && !Quit_is_required)
    Get_input(20);
  if (Mouse_K)
  {
    clicked_button = Window_get_clicked_button();
    Wait_end_of_click();
  }
  else
    clicked_button = Window_get_button_shortcut();
  GFX2_Log(GFX2_DEBUG, "Button_Message_initial() clicked_button=%d\n", clicked_button);
  Close_window();

  if (clicked_button > 0)
  {
    if (Main.backups->Pages->Image_mode == IMAGE_MODE_LAYERED)
    {
      Switch_layer_mode(IMAGE_MODE_ANIMATION);
      Config.Default_mode_layers = 0;
    }
    else
    {
      Switch_layer_mode(IMAGE_MODE_LAYERED);
      Config.Default_mode_layers = 1;
    }
    if (Check_menu_mode())
    {
      Display_menu();
      Display_all_screen();
    }
    // Modify the mode for the spare too
    Spare.backups->Pages->Image_mode = Main.backups->Pages->Image_mode;
    Update_spare_buffers(Spare.image_width,Spare.image_height);
    Redraw_spare_image();
  }
  Display_cursor();
}



void Change_paintbrush_shape(byte shape)
{
  Paintbrush_shape=shape;
  Display_paintbrush_in_menu();

  switch (Current_operation)
  {
    case OPERATION_FILL :
      Paintbrush_shape_before_fill=shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
      break;
    case OPERATION_COLORPICK :
      Paintbrush_shape_before_colorpicker=shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_NONE;
      break;
    // Note: Il existe un Paintbrush_shape_before_lasso, mais comme le lasso aura
    // été automatiquement désactivé avant d'arriver ici, y'a pas de problème.
  }
}


//-------------------------------- UNDO/REDO ---------------------------------
void Button_Undo(int btn)
{
  Hide_cursor();
  Undo();
  Set_palette(Main.palette);
  Compute_optimal_menu_colors(Main.palette);

  Check_menu_mode();
  Display_all_screen();
  Unselect_button(btn);
  Draw_menu_button(BUTTON_MAGNIFIER,Main.magnifier_mode);
  Display_menu();
  Display_cursor();
}

void Button_Redo(int btn)
{
  Hide_cursor();
  Redo();
  Set_palette(Main.palette);
  Compute_optimal_menu_colors(Main.palette);

  Check_menu_mode();
  Display_all_screen();
  Unselect_button(btn);
  Draw_menu_button(BUTTON_MAGNIFIER,Main.magnifier_mode);
  Display_menu();
  Display_cursor();
}

//---------------------------- SCROLL PALETTE LEFT ---------------------------
void Button_Pal_left(int btn)
{
  short cells;
  cells = (Config.Palette_vertical)?Palette_cells_X():Palette_cells_Y();

  Hide_cursor();
  if (First_color_in_palette)
  {
    if (First_color_in_palette>=cells)
      First_color_in_palette-=cells;
    else
      First_color_in_palette=0;
    Display_menu_palette();
  }
  Unselect_button(btn);
  Display_cursor();
}

void Button_Pal_left_fast(int btn)
{
  short cells_x = Palette_cells_X();
  short cells_y = Palette_cells_Y();

  Hide_cursor();
  if (First_color_in_palette)
  {
    if (First_color_in_palette>=cells_y*cells_x)
      First_color_in_palette-=cells_y*cells_x;
    else
      First_color_in_palette=0;
    Display_menu_palette();
  }
  Unselect_button(btn);
  Display_cursor();
}


//--------------------------- SCROLL PALETTE RIGHT ---------------------------
void Button_Pal_right(int btn)
{
  short cells;
  cells = (Config.Palette_vertical)?Palette_cells_X():Palette_cells_Y();

  Hide_cursor();
  if ((int)First_color_in_palette+Palette_cells_X()*Palette_cells_Y()<256)
  {
    First_color_in_palette+=cells;
    Display_menu_palette();
  }

  Unselect_button(btn);
  Display_cursor();
}

void Button_Pal_right_fast(int btn)
{
  short cells_x = Palette_cells_X();
  short cells_y = Palette_cells_Y();

  Hide_cursor();
  if ((int)First_color_in_palette+cells_y*cells_x<256)
  {
    if ((int)First_color_in_palette+(cells_y)*cells_x*2<256)
      First_color_in_palette+=cells_x*cells_y;
    else
    {
      if (Config.Palette_vertical)
        First_color_in_palette=255/cells_x*cells_x-(cells_y-1)*cells_x;
      else
        First_color_in_palette=255/cells_y*cells_y-(cells_x-1)*cells_y;
    }
    Display_menu_palette();
  }
  Unselect_button(btn);
  Display_cursor();
}

//-------------------- item de la forecolor dans le menu --------------------
void Button_Select_forecolor(int btn)
{
  static long time_click = 0;
  long time_previous;
  int color;

  time_previous = time_click;
  time_click = GFX2_GetTicks();

  color=Pick_color_in_palette();

  if (color == Fore_color)
  {
    // Check if it's a double-click
    if (time_click - time_previous < Config.Double_click_speed)
    {
      // Open palette window
      Button_Palette(btn);
      return;
    }
  }

  do
  {
    if (color != Fore_color && color!=-1)
    {
      Hide_cursor();
      Set_fore_color(color);
      Display_cursor();
    }
    // Wait loop after initial click
    while(Mouse_K)
    {
      Get_input(20);

      if (Button_under_mouse()==btn)
      {
        color=Pick_color_in_palette();
        if (color != Fore_color && color!=-1)
        {
          Hide_cursor();
          Status_print_palette_color(color);
          Set_fore_color(color);
          Display_cursor();
        }
      }
    }
  } while(Mouse_K);
}

//-------------------- item de la backcolor dans le menu --------------------
void Button_Select_backcolor(int btn)
{
  int color;

  do
  {
    color=Pick_color_in_palette();

    if (color!=-1 && color != Back_color)
    {
      Hide_cursor();
      Status_print_palette_color(color);
      Set_back_color(color);
      Display_cursor();
    }
    // Wait loop after initial click
    do
    {
      Get_input(20);

      if (Button_under_mouse()==btn)
        break; // This will repeat this button's action

    } while(Mouse_K);
  } while(Mouse_K);
}

void Button_Hide_menu(int btn)
{
  Hide_cursor();
  if (Menu_is_visible)
  {
    Menu_is_visible=0;
    Menu_Y=Screen_height;

    if (Main.magnifier_mode)
    {
      Compute_magnifier_data();
    }

    //   On repositionne le décalage de l'image pour qu'il n'y ait pas d'in-
    // -cohérences lorsqu'on sortira du mode Loupe.
    if (Main.offset_Y+Screen_height>Main.image_height)
    {
      if (Screen_height>Main.image_height)
        Main.offset_Y=0;
      else
        Main.offset_Y=Main.image_height-Screen_height;
    }
    // On fait pareil pour le brouillon
    if (Spare.offset_Y+Screen_height>Spare.image_height)
    {
      if (Screen_height>Spare.image_height)
        Spare.offset_Y=0;
      else
        Spare.offset_Y=Spare.image_height-Screen_height;
    }

    Compute_magnifier_data();
    if (Main.magnifier_mode)
      Position_screen_according_to_zoom();
    Compute_limits();
    Compute_paintbrush_coordinates();
    Display_all_screen();
  }
  else
  {
    byte current_menu;
    Menu_is_visible=1;
    Menu_Y=Screen_height;
    for (current_menu = 0; current_menu < MENUBAR_COUNT; current_menu++)
      if (Menu_bars[current_menu].Visible)
        Menu_Y -= Menu_bars[current_menu].Height * Menu_factor_Y;

    Compute_magnifier_data();
    if (Main.magnifier_mode)
      Position_screen_according_to_zoom();
    Compute_limits();
    Compute_paintbrush_coordinates();
    Display_menu();
    if (Main.magnifier_mode)
      Display_all_screen();
  }
  Unselect_button(btn);
  Display_cursor();
}

void Button_Toggle_toolbar(int btn)
{
  T_Dropdown_button dropdown;
  T_Dropdown_choice *item;
  static char menu_name_tools[9] = " Tools";
  static char menu_name_layers[9]= " Layers";
  static char menu_name_anim[9]  = " Anim";


  menu_name_tools[0]  = Menu_bars[MENUBAR_TOOLS ].Visible ? 22 : ' ';
  menu_name_layers[0] = Menu_bars[MENUBAR_LAYERS].Visible ? 22 : ' ';
  menu_name_anim[0]   = Menu_bars[MENUBAR_ANIMATION].Visible ? 22 : ' ';

  Hide_cursor();

  dropdown.Pos_X         =Buttons_Pool[BUTTON_HIDE].X_offset;
  dropdown.Pos_Y         =Buttons_Pool[BUTTON_HIDE].Y_offset;
  dropdown.Height        =Buttons_Pool[BUTTON_HIDE].Height;
  dropdown.Dropdown_width=70;
  dropdown.First_item    =NULL;
  dropdown.Bottom_up     =1;

  Window_dropdown_add_item(&dropdown, 0, menu_name_tools);

  if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION ||
      Main.backups->Pages->Nb_layers==1)
    Window_dropdown_add_item(&dropdown, 1, menu_name_layers);

  if (Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION ||
    (Main.backups->Pages->Image_mode == IMAGE_MODE_LAYERED && Main.backups->Pages->Nb_layers==1))
    Window_dropdown_add_item(&dropdown, 2, menu_name_anim);

  item=Dropdown_activate(&dropdown,0,Menu_Y+Menu_bars[MENUBAR_STATUS].Top*Menu_factor_Y);

  if (item)
  {
    switch (item->Number)
    {
      case 0: // tools
        Set_bar_visibility(MENUBAR_TOOLS, !Menu_bars[MENUBAR_TOOLS].Visible, 0);
        break;
      case 1: // layers
        if (Menu_bars[MENUBAR_ANIMATION].Visible)
        {
          Set_bar_visibility(MENUBAR_ANIMATION, 0, 0);
          Config.Default_mode_layers=1;
        }
        Set_bar_visibility(MENUBAR_LAYERS, !Menu_bars[MENUBAR_LAYERS].Visible, 0);

        if (Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
          Switch_layer_mode(IMAGE_MODE_LAYERED);

        break;
      case 2: // anim
        if (Menu_bars[MENUBAR_LAYERS].Visible)
        {
          Set_bar_visibility(MENUBAR_LAYERS, 0, 0);
          Config.Default_mode_layers=0;
        }
        Set_bar_visibility(MENUBAR_ANIMATION, !Menu_bars[MENUBAR_ANIMATION].Visible, 0);

        if (Main.backups->Pages->Image_mode != IMAGE_MODE_ANIMATION)
          Switch_layer_mode(IMAGE_MODE_ANIMATION);

        break;
    }
    // redraw image and menu
    Display_menu();
    Display_all_screen();
  }

  // Closing
  Window_dropdown_clear_items(&dropdown);

  Unselect_button(btn);
  Display_cursor();
}

void Button_Toggle_all_toolbars(int btn)
{
  // This is used to memorize the bars' visibility when temporarily hidden
  static word Last_visibility = 0xFFFF;
  int i;
  word current_visibility;

  Hide_cursor();

  // Check which bars are visible
  current_visibility=0;
  for (i=MENUBAR_STATUS+1;i<MENUBAR_COUNT;i++)
    if (Menu_bars[i].Visible)
      current_visibility |= (1<<i);

  if (current_visibility)
  {
    // At least one is visible: Hide all
    Last_visibility=current_visibility;
    for (i=MENUBAR_STATUS+1;i<MENUBAR_COUNT;i++)
      Set_bar_visibility(i,0, 0);
  }
  else
  {
    // Restore all
    for (i=MENUBAR_STATUS+1;i<MENUBAR_COUNT;i++)
      Set_bar_visibility(i,(Last_visibility & (1<<i)) ? 1 : 0, 0);
  }
  Check_menu_mode();
  Display_menu();
  Display_all_screen();

  Unselect_button(btn);
  Display_cursor();
}

//--------------------------- Quitter le programme ---------------------------
byte Button_Quit_local_function(void)
{
  short clicked_button;
  byte  old_cursor_shape;
  char * filename;
  int exists;

  if (!Main.image_is_modified)
    return 1;

  // On commence par afficher la fenêtre de QUIT
  Open_window(160,84,"Quit ?");
  Window_set_normal_button(20,20,120,14,"Stay",0,1,KEY_ESC);          // 1
  Window_set_normal_button(20,40,120,14,"Save & quit",1,1,KEY_s);   // 2
  Window_set_normal_button(20,60,120,14,"Discard (Quit)",1,1,KEY_d);// 3
  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    Quit_is_required = 0; // ignore other QUIT messages from the system
    clicked_button = Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_QUIT, NULL);
    else if (Is_shortcut(Key,0x100+BUTTON_QUIT))
      clicked_button=1;
  }
  while (clicked_button<=0);

  Close_window();
  Display_cursor();

  switch(clicked_button)
  {
    case 1 : return 0; // Rester
    case 2 : // Sauver et enregistrer
      filename = Filepath_append_to_dir(Main.backups->Pages->File_directory, Main.backups->Pages->Filename);
      exists = File_exists(filename);
      free(filename);
      if ( !exists || Confirmation_box("Erase old file ?") )
      {
        T_IO_Context save_context;

        Hide_cursor();
        old_cursor_shape=Cursor_shape;
        Cursor_shape=CURSOR_SHAPE_HOURGLASS;
        Display_cursor();

        Init_context_layered_image(&save_context, Main.backups->Pages->Filename, Main.backups->Pages->File_directory);
        save_context.File_name_unicode = Main.backups->Pages->Filename_unicode;
        Save_image(&save_context);
        Destroy_context(&save_context);

        Hide_cursor();
        Cursor_shape=old_cursor_shape;
        Display_cursor();

        if (!File_error)
          // L'ayant sauvée avec succès,
          return 1; // On peut quitter
        else
          // Il y a eu une erreur lors de la sauvegarde,
          return 0; // On ne peut donc pas quitter
      }
      else
        // L'utilisateur ne veut pas écraser l'ancien fichier,
        return 0; // On doit donc rester
    case 3 : return 1; // Quitter
  }
  return 0;
}


void Button_Quit(int btn)
{
  //short clicked_button;

  if (Button_Quit_local_function())
  {
    if (Spare.image_is_modified)
    {
      Button_Page(BUTTON_PAGE); // On passe sur le brouillon
      // Si l'utilisateur présente les derniers symptomes de l'abandon
      if (Button_Quit_local_function())
        Quitting=1;
    }
    else
      Quitting=1;
  }

  if ( (Menu_is_visible) && (Mouse_Y+8>Menu_Y) )
    Hide_cursor();

  Unselect_button(btn);

  if ( (Menu_is_visible) && (Mouse_Y+8>Menu_Y) )
    Display_cursor();
}


//---------------------------- Effacer l'écran -------------------------------
void Button_Clear(int btn)
{
  Hide_cursor();
  Backup();
  if (Stencil_mode && Config.Clear_with_stencil)
    Clear_current_image_with_stencil(Main.backups->Pages->Transparent_color,Stencil);
  else
    Clear_current_image(Main.backups->Pages->Transparent_color);
  Redraw_layered_image();
  End_of_modification();
  Display_all_screen();
  Unselect_button(btn);
  Display_cursor();
}

void Button_Clear_with_backcolor(int btn)
{
  Hide_cursor();
  Backup();
  if (Stencil_mode && Config.Clear_with_stencil)
    Clear_current_image_with_stencil(Back_color,Stencil);
  else
    Clear_current_image(Back_color);
  Redraw_layered_image();
  End_of_modification();
  Display_all_screen();
  Unselect_button(btn);
  Display_cursor();
}


//------------------------------- Paramètres ---------------------------------

#define SETTING_PER_PAGE 11
#define SETTING_PAGES     5

#define SETTING_HEIGHT   12

typedef struct {
  const char* Label; // Use NULL label to stop an array
  int Code;
} T_Lookup;

const T_Lookup Lookup_YesNo[] = {
  {"NO",0},
  {"YES",1},
  {NULL,-1},
};

const T_Lookup Lookup_FFF[] = {
  {"All",0},
  {"Files",1},
  {"Dirs.",2},
  {NULL,-1},
};

const T_Lookup Lookup_AutoRes[] = {
  {"Internal",1},
  {"Real",2},
  {NULL,-1},
};

const T_Lookup Lookup_Coords[] = {
  {"Relative",1},
  {"Absolute",2},
  {NULL,-1},
};

const T_Lookup Lookup_MenuRatio[] = {
  {"None",0},
  {"x2",254}, // -2
  {"x3",253}, // -3
  {"x4",252}, // -4
  {"Moderate",2},
  {"Maximum",1},
  {NULL,-1},
};

const T_Lookup Lookup_MouseSpeed[] = {
  {"Normal",1},
  {"/2",2},
  {"/3",3},
  {"/4",4},
  {NULL,-1},
};

const T_Lookup Lookup_SwapButtons[] = {
  {"None",0},
  {"Control",GFX2_MOD_CTRL},
  {"Alt",GFX2_MOD_ALT},
  {NULL,-1},
};

const T_Lookup Lookup_VirtualKeyboard[] = {
  {"Auto",0},
  {"ON",1},
  {"OFF",2},
  {NULL,-1},
};

typedef struct {
  const char* Label;
  byte Type; // 0: label, 1+: setting (size in bytes)
  void * Value;
  int Min_value;
  int Max_value;
  int Digits; // Could be computed from Max_value...but don't bother.
  const T_Lookup * Lookup;
} T_Setting;

long int Get_setting_value(const T_Setting *item)
{
  switch(item->Type)
  {
    case 1:
      return *((byte *)(item->Value));
      break;
    case 2:
      return *((word *)(item->Value));
      break;
    case 4:
    default:
      return *((long int *)(item->Value));
      break;
  }
}

void Set_setting_value(const T_Setting *item, long int value)
{
  switch(item->Type)
  {
    case 1:
      *((byte *)(item->Value)) = value;
      break;
    case 2:
      *((word *)(item->Value)) = value;
      break;
    case 4:
    default:
      *((long int *)(item->Value)) = value;
      break;
  }
}

// Fetch a label in a lookup table. Unknown values get label 0.
const char *Lookup_code(int code, const T_Lookup *lookup)
{
  int i;

  for(i=0; lookup[i].Label!=NULL; i++)
  {
    if (lookup[i].Code == code)
      return lookup[i].Label;
  }
  return lookup[0].Label;
}

/// Increase an enum to next-higher value (wrapping).
int Lookup_next(int code, const T_Lookup *lookup)
{
  int i;

  for(i=0; lookup[i].Label!=NULL; i++)
  {
    if (lookup[i].Code == code)
    {
      if (lookup[i+1].Label==NULL)
        return lookup[0].Code;
      return lookup[i+1].Code;
    }
  }
  return 0;
}

/// Decrease an enum to previous value (wrapping).
int Lookup_previous(int code, const T_Lookup *lookup)
{
  int count;
  int current=-1;

  for(count=0; lookup[count].Label!=NULL; count++)
  {
    if (lookup[count].Code == code)
      current=count;
  }

  return lookup[(current + count - 1) % count].Code;
}

void Settings_display_config(const T_Setting *setting, T_Config * conf, T_Special_button *panel)
{
  int i;

  // A single button
  Print_in_window(155,166,(conf->Auto_save)?"YES":" NO",MC_Black,MC_Light);

  // Clear all
  Window_rectangle(panel->Pos_X, panel->Pos_Y, panel->Width, panel->Height+1, MC_Light);
  for (i=0; i<SETTING_PER_PAGE; i++)
  {
    Print_in_window(panel->Pos_X+3, panel->Pos_Y+i*SETTING_HEIGHT+(SETTING_HEIGHT-6)/2, setting[i].Label, i==0?MC_White:MC_Dark, MC_Light);
    if(setting[i].Value)
    {

      int value = Get_setting_value(&setting[i]);

      if (setting[i].Lookup)
      {
        // Use a lookup table to print a label
        const char *str;
        str = Lookup_code(value,setting[i].Lookup);
        Print_in_window(panel->Pos_X+3+176, panel->Pos_Y+i*SETTING_HEIGHT+(SETTING_HEIGHT-6)/2, str, MC_Black, MC_Light);
      }
      else
      {
        // Print a number
        char str[10];
        Num2str(value,str,setting[i].Digits);
        Print_in_window(panel->Pos_X+3+176, panel->Pos_Y+i*SETTING_HEIGHT+(SETTING_HEIGHT-6)/2, str, MC_Black, MC_Light);
      }
    }
  }
  Update_window_area(panel->Pos_X, panel->Pos_Y, panel->Width, panel->Height+1);
}

void Settings_save_config(T_Config * conf)
{
  if (Save_CFG())
    Error(0);
  else
    if (Save_INI(conf))
      Error(0);
}

void Settings_load_config(T_Config * conf)
{
  if (Load_CFG(0))
    Error(0);
  else
    if (Load_INI(conf))
      Error(0);
}

void Button_Settings(int btn)
{
  short clicked_button;
  T_Config selected_config;
  byte config_is_reloaded=0;
  T_Special_button *panel;
  byte need_redraw=1;
  static byte current_page=0;

  // Definition of settings pages
  //  Label,Type (0 = label, 1+ = setting size in bytes),
  //  Value, min, max, digits, Lookup)

  const T_Setting setting[SETTING_PER_PAGE*SETTING_PAGES] = {

  {"           --- GUI  ---",0,NULL,0,0,0,NULL},
  {"Opening message:",1,&(selected_config.Opening_message),0,1,0,Lookup_YesNo},
  {"Menu ratio adapt:",1,&(selected_config.Ratio),0,1,0,Lookup_MenuRatio},
  {"Draw limits:",1,&(selected_config.Display_image_limits),0,1,0,Lookup_YesNo},
  {"Coordinates:",1,&(selected_config.Coords_rel),0,1,0,Lookup_Coords},
  {"Separate colors:",1,&(selected_config.Separate_colors),0,1,0,Lookup_YesNo},
  {"Safety colors:",1,&(selected_config.Safety_colors),0,1,0,Lookup_YesNo},
  {"Sync views:",1,&(selected_config.Sync_views),0,1,0,Lookup_YesNo},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},

  {"           --- Input  ---",0,NULL,0,0,0,NULL},
  {"Scrollbar speed",0,NULL,0,0,0,NULL},
  {"  on left click:",1,&(selected_config.Delay_left_click_on_slider),1,255,4,NULL},
  {"  on right click:",1,&(selected_config.Delay_right_click_on_slider),1,255,4,NULL},
  {"Merge movement:",1,&(selected_config.Mouse_merge_movement),0,100,4,NULL},
  {"Double click speed:",2,&(selected_config.Double_click_speed),1,1999,4,NULL},
  {"Double key speed:",2,&(selected_config.Double_key_speed),1,1999,4,NULL},
  //{"Mouse speed (fullscreen)",0,NULL,0,0,0,NULL},
  //{"  horizontally:",1,&(selected_config.Mouse_sensitivity_index_x),1,4,0,Lookup_MouseSpeed},
  //{"  vertically:",1,&(selected_config.Mouse_sensitivity_index_y),1,4,0,Lookup_MouseSpeed},
  {"Key to swap buttons:",2,&(selected_config.Swap_buttons),0,0,0,Lookup_SwapButtons},
  {"Virtual keyboard",1,&(selected_config.Use_virtual_keyboard),0,2,0,Lookup_VirtualKeyboard},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},

  {"          --- Editing  ---",0,NULL,0,0,0,NULL},
  {"Adjust brush pick:",1,&(selected_config.Adjust_brush_pick),0,1,0,Lookup_YesNo},
  {"Undo pages:",1,&(selected_config.Max_undo_pages),1,99,5,NULL},
  {"Vertices per polygon:",4,&(selected_config.Nb_max_vertices_per_polygon),2,16384,5,NULL},
  {"Fast zoom:",1,&(selected_config.Fast_zoom),0,1,0,Lookup_YesNo},
  {"Clear with stencil:",1,&(selected_config.Clear_with_stencil),0,1,0,Lookup_YesNo},
  {"Auto discontinuous:",1,&(selected_config.Auto_discontinuous),0,1,0,Lookup_YesNo},
  {"Auto count colors:",1,&(selected_config.Auto_nb_used),0,1,0,Lookup_YesNo},
  {"Right click colorpick:",1,&(selected_config.Right_click_colorpick),0,1,0,Lookup_YesNo},
  {"Multi shortcuts:",1,&(selected_config.Allow_multi_shortcuts),0,1,0,Lookup_YesNo},
  {"",0,NULL,0,0,0,NULL},

  {"      --- File selector  ---",0,NULL,0,0,0,NULL},
  {"Show in fileselector",0,NULL,0,0,0,NULL},
  {"  Hidden files:",4,&(selected_config.Show_hidden_files),0,1,0,Lookup_YesNo},
  {"  Hidden dirs:",4,&(selected_config.Show_hidden_directories),0,1,0,Lookup_YesNo},
  {"Preview delay:",4,&(selected_config.Timer_delay), 1,256,3,NULL},
  {"Maximize preview:",1,&(selected_config.Maximize_preview), 0,1,0,Lookup_YesNo},
  {"Find file fast:",1,&(selected_config.Find_file_fast), 0,2,0,Lookup_FFF},
  {"Auto set resolution:",1,&(selected_config.Auto_set_res), 0,1,0,Lookup_YesNo},
  {"  According to:",1,&(selected_config.Set_resolution_according_to), 1,2,0,Lookup_AutoRes},
  {"Backup:",1,&(selected_config.Backup), 0,1,0,Lookup_YesNo},
  {"",0,NULL,0,0,0,NULL},

  {"      --- Format options  ---",0,NULL,0,0,0,NULL},
  {"Screen size in GIF:",1,&(selected_config.Screen_size_in_GIF),0,1,0,Lookup_YesNo},
  {"Clear palette:",1,&(selected_config.Clear_palette),0,1,0,Lookup_YesNo},
  {"MO6/TO8 palette gamma",1,&(selected_config.MOTO_gamma),10,30,2,NULL},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},
  {"",0,NULL,0,0,0,NULL},


  };

  const char * help_section[SETTING_PAGES] = {
    "GUI",
    "INPUT",
    "EDITING",
    "FILE SELECTOR",
    "FILE FORMAT OPTIONS",
  };

  selected_config=Config;

  Open_window(307,182,"Settings");

    // Button Reload
  Window_set_normal_button(  6,163, 51,14,"Reload"       ,1,1,KEY_r); // 1
    // Button Auto-save
  Window_set_normal_button( 73,163,107,14,"Auto-save:   ",1,1,KEY_a); // 2
    // Button Save
  Window_set_normal_button(183,163, 51,14,"Save"         ,1,1,KEY_s); // 3
    // Button Close
  Window_set_normal_button(250,163, 51,14,"Close"        ,0,1,KEY_ESC); // 4

  panel=Window_set_special_button(10, 21, 272,SETTING_PER_PAGE*SETTING_HEIGHT,0); // 5
  Window_set_scroller_button(285,21,SETTING_PER_PAGE*SETTING_HEIGHT,SETTING_PAGES,1,current_page); // 6

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    if (need_redraw)
    {
      Hide_cursor();
      Settings_display_config(setting+current_page*SETTING_PER_PAGE, &selected_config, panel);
      if (need_redraw & 2)
      {
        // Including slider position
        Window_scroller_button_list->Position=current_page;
        Window_draw_slider(Window_scroller_button_list);
      }

      Display_cursor();

      need_redraw=0;
    }

    clicked_button=Window_clicked_button();

    switch(clicked_button)
    {

      case 1 : // Reload
        Settings_load_config(&selected_config);
        config_is_reloaded=1;
        need_redraw=1;
        break;
      case 2 : // Auto-save
        selected_config.Auto_save=!selected_config.Auto_save;
        need_redraw=1;
        break;
      case 3 : // Save
        Settings_save_config(&selected_config);
        break;
      // case 4: // Close

      case 5: // Panel area
        {

          int num=(((short)Mouse_Y-Window_pos_Y)/Menu_factor_Y - panel->Pos_Y)/SETTING_HEIGHT;
          if (num >= 0 && num < SETTING_PER_PAGE)
          {
            const T_Setting * item = &setting[current_page*SETTING_PER_PAGE+num];
            if (item->Type!=0)
            {
              // Remember which button is clicked
              byte old_mouse_k = Mouse_K;

              if (Window_normal_button_onclick(panel->Pos_X, panel->Pos_Y+num*SETTING_HEIGHT, panel->Width, SETTING_HEIGHT+1, 5))
              {
                int value = Get_setting_value(item);

                if (item->Lookup)
                {
                  // Enum: toggle it
                  if (old_mouse_k & LEFT_SIDE)
                    value = Lookup_next(value, item->Lookup);
                  else
                    value = Lookup_previous(value, item->Lookup);
                  Set_setting_value(item, value);
                }
                else
                {
                  // Numeric: edit it
                  char str[10];
                  str[0]='\0';
                  if (! (old_mouse_k & RIGHT_SIDE))
                    Num2str(value,str,item->Digits+1);
                  if (Readline(panel->Pos_X+3+176, panel->Pos_Y+num*SETTING_HEIGHT+(SETTING_HEIGHT-6)/2,str,item->Digits+1,INPUT_TYPE_INTEGER))
                  {
                    value=atoi(str);
                    if (value<item->Min_value)
                      value = item->Min_value;
                    else if (value>item->Max_value)
                      value = item->Max_value;

                    Set_setting_value(item, value);
                  }
                  Key=0; // Need to discard keys used during editing
                }
              }
            }
          }
        }
        need_redraw=1;
        break;
      case 6: // Scroller
        current_page = Window_attribute2;
        need_redraw=1;
        break;

    }

    if (Key == KEY_MOUSEWHEELDOWN)
    {
      if (current_page < (SETTING_PAGES-1))
      {
        current_page++;
        need_redraw=2;
      }
    }
    else if (Key == KEY_MOUSEWHEELUP)
    {
      if (current_page > 0)
      {
        current_page--;
        need_redraw=2;
      }
    }
    else if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(NB_BUTTONS+0, help_section[current_page]);
    else if (Is_shortcut(Key,0x100+BUTTON_SETTINGS))
      clicked_button=4;
  }
  while ( (clicked_button!=4) && (Key!=KEY_RETURN) && !Quit_is_required);

  // Checks on change
  if (Config.Show_hidden_directories!=selected_config.Show_hidden_directories
    ||Config.Show_hidden_files!=selected_config.Show_hidden_files)
  {
    // Reset fileselector offsets
    // since different files are shown now
    Main.selector.Position=0;
    Main.selector.Offset=0;
    Spare.selector.Position=0;
    Spare.selector.Offset=0;
    Brush_selector.Position=0;
    Brush_selector.Offset=0;
    Palette_selector.Position=0;
    Palette_selector.Offset=0;
  }
  if(Config.Allow_multi_shortcuts && !selected_config.Allow_multi_shortcuts)
  {
    // User just disabled multi shortcuts: make them unique now.
    Remove_duplicate_shortcuts();
  }
  // Copy all
  Config=selected_config;

  if (config_is_reloaded)
    Compute_optimal_menu_colors(Main.palette);

  Close_window();
  Unselect_button(btn);
  // Raffichage du menu pour que les inscriptions qui y figurent soient
  // retracées avec la nouvelle fonte
  Display_menu();
  Display_cursor();

  // On vérifie qu'on peut bien allouer le nombre de pages Undo.
  Set_number_of_backups(Config.Max_undo_pages);
}

// Data for skin selector
T_Fileselector Skin_files_list;


// Data for font selector
T_Fileselector Font_files_list;

//
char * Format_font_filename(const char * fname)
{
  static char result[12];
  int c;
  int length;

  fname+=strlen(FONT_PREFIX); // Omit file prefix
  length=strlen(fname) - 4; // assume .png extension

  for (c=0;c<11 && c<length ;c++)
  {
    result[c]=fname[c];
  }
  result[c]='\0';
  if (length>11)
    result[10] = ELLIPSIS_CHARACTER;

  return result;
}

// Add a skin to the list
static void Add_font_or_skin(const char * full_name, const char * fname)
{
  size_t namelength;
  (void)full_name;

  namelength = strlen(fname);
  if (namelength>=10 && fname[0]!='_' && !strncasecmp(fname, SKIN_PREFIX, strlen(SKIN_PREFIX))
    && (!strcasecmp(fname + namelength - 4,".png")
    || !strcasecmp(fname + namelength - 4,".gif")))
  {
    Add_element_to_list(&Skin_files_list, fname, Format_filename(fname, 19, 0), FSOBJECT_FILE, ICON_NONE);

    if (fname[0]=='\0')
      return;
  }
  else if (namelength>=10 && !strncasecmp(fname, FONT_PREFIX, strlen(FONT_PREFIX))
    && (!strcasecmp(fname + namelength - 4, ".png")))
  {
    Add_element_to_list(&Font_files_list, fname, Format_font_filename(fname), FSOBJECT_FILE, ICON_NONE);

    if (fname[0]=='\0')
      return;
  }

}

// Callback to display a skin name in the list
void Draw_one_skin_name(word x, word y, word index, byte highlighted)
{
  T_Fileselector_item * current_item;

  if (Skin_files_list.Nb_elements)
  {
    current_item = Get_item_by_index(&Skin_files_list, index);
    Print_in_window(x, y, current_item->Short_name, MC_Black,
      (highlighted)?MC_Dark:MC_Light);
  }
}

/// Skin selector window
void Button_Skins(int btn)
{
  short clicked_button;
  short temp;
  char * skinsdir;
  T_Dropdown_button * font_dropdown;
  T_Dropdown_button * cursor_dropdown;
  T_List_button * skin_list;
  T_Scroller_button * file_scroller;
  int selected_font = 0;
  int selected_cursor = Config.Cursor;
  byte separatecolors = Config.Separate_colors;
  byte showlimits = Config.Display_image_limits;
  byte need_load=1;
  int button;

  word x, y, x_pos, offs_y;

  const char * cursors[] = { "Solid", "Transparent", "Thin" };
  T_Gui_skin * gfx = NULL;
  byte * new_font;

  #define FILESEL_Y 34

  // --- Read the contents of skins/ directory ------------------

  // Here we use the same data container as the fileselectors.
  // Reinitialize the list
  Free_fileselector_list(&Skin_files_list);
  Free_fileselector_list(&Font_files_list);
  // Browse the "skins" directory
  skinsdir = Filepath_append_to_dir(Data_directory, SKINS_SUBDIRECTORY);
  // Add each found file to the list
  For_each_file(skinsdir, Add_font_or_skin);
  // Sort it
  Sort_list_of_files(&Skin_files_list);
  Sort_list_of_files(&Font_files_list);

  selected_font = Find_file_in_fileselector(&Font_files_list, Config.Font_file);
  if (selected_font < 0)
    selected_font = 0;

  // --------------------------------------------------------------

  Open_window(290, 140, "Skins");

  // Frames
  Window_display_frame_in(6, FILESEL_Y - 2, 148, 84); // File selector

  // Texts
  Print_in_window( 172, 33,"Font:"            ,MC_Black,MC_Light);
  Print_in_window( 172, 59,"Cursor:"          ,MC_Black,MC_Light);

  // Ok button
  Window_set_normal_button(6, 120, 51, 14, "OK", 0, 1, KEY_RETURN); // 1

  // List of skins
  skin_list = Window_set_list_button(
  // Fileselector
  Window_set_special_button(8, FILESEL_Y + 1, 144, 80,0), // 2
    // Scroller for the fileselector
    (file_scroller = Window_set_scroller_button(155, FILESEL_Y - 1, 82,
    Skin_files_list.Nb_elements, 10, 0)), // 3
    Draw_one_skin_name, 2); // 4

  skin_list->Cursor_position = Find_file_in_fileselector(&Skin_files_list, Config.Skin_file);
  if (skin_list->Cursor_position < 0)
    skin_list->Cursor_position = 0;

  // Buttons to choose a font
  font_dropdown = Window_set_dropdown_button(172, 43, 104, 11, 0, Get_item_by_index(&Font_files_list,selected_font)->Short_name,1,0,1,RIGHT_SIDE|LEFT_SIDE,0); // 5
  for (temp=0; temp<Font_files_list.Nb_files; temp++)
    Window_dropdown_add_item(font_dropdown,temp,Get_item_by_index(&Font_files_list,temp)->Short_name);

  // Cancel
  Window_set_normal_button(61, 120, 51,14,"Cancel",0,1,KEY_ESCAPE); // 6

  // Dropdown list to choose cursor type
  cursor_dropdown = Window_set_dropdown_button(172, 69, 104, 11, 0,
    cursors[selected_cursor], 1, 0, 1, RIGHT_SIDE|LEFT_SIDE,0); // 7
  for (temp = 0; temp<3; temp++)
    Window_dropdown_add_item(cursor_dropdown, temp, cursors[temp]);

  Window_set_normal_button(172, 87, 14, 14,
    (Config.Display_image_limits)?"X":" ", -1, 1, KEY_NONE); // 8
  Print_in_window( 190, 85,"Draw picture", MC_Dark, MC_Light);
  Print_in_window( 190, 94,"limits", MC_Dark, MC_Light);

  Window_set_normal_button(172, 111, 14, 14,
    (Config.Separate_colors)?"X":" ", -1, 1, KEY_NONE); // 9
  Print_in_window( 190, 109,"Separate", MC_Dark, MC_Light);
  Print_in_window( 190, 118,"colors", MC_Dark, MC_Light);

  Window_redraw_list(skin_list);

  for (y = 14, offs_y = 0; offs_y < 16; offs_y++, y++)
    for (x = 6, x_pos = 0; x_pos<173; x_pos++, x++)
      Pixel_in_window(x, y, Gfx->Preview[offs_y][x_pos]);

  Update_window_area(0, 0, Window_width, Window_height);

  Display_cursor();

  do
  {
    if (need_load)
    {
      need_load=0;

      Hide_cursor();
      // (Re-)load GUI graphics from selected skins
      free(skinsdir);
      skinsdir = strdup(Get_item_by_index(&Skin_files_list,
        skin_list->List_start + skin_list->Cursor_position)->Full_name);

      gfx = Load_graphics(skinsdir, NULL);
      if (gfx == NULL) // Error
      {
        Display_cursor();
        Verbose_message("Error!", Gui_loading_error_message);
        Hide_cursor();
        // Update preview
        Window_rectangle(6, 14, 173, 16, MC_Light);
      }
      else
      {
        // Update preview

        // Display the bitmap according to its own color indices
        for (y = 14, offs_y = 0; offs_y < 16; offs_y++, y++)
        for (x = 6, x_pos = 0; x_pos<173; x_pos++, x++)
        {
          if (gfx->Preview[offs_y][x_pos] == gfx->Color[0])
            Pixel_in_window(x, y, MC_Black);
          else if (gfx->Preview[offs_y][x_pos] == gfx->Color[1])
            Pixel_in_window(x, y,  MC_Dark);
          else if (gfx->Preview[offs_y][x_pos] == gfx->Color[3])
            Pixel_in_window(x, y, MC_White);
          else if (gfx->Preview[offs_y][x_pos] == gfx->Color[2])
            Pixel_in_window(x, y, MC_Light);
        }
        // Actualize current screen according to preferred GUI colors
        // Note this only updates onscreen colors
        Set_color(
          MC_Black,
          gfx->Default_palette[gfx->Color[0]].R,
          gfx->Default_palette[gfx->Color[0]].G,
          gfx->Default_palette[gfx->Color[0]].B);
        Set_color(
          MC_Dark,
          gfx->Default_palette[gfx->Color[1]].R,
          gfx->Default_palette[gfx->Color[1]].G,
          gfx->Default_palette[gfx->Color[1]].B);
        Set_color(
          MC_Light,
          gfx->Default_palette[gfx->Color[2]].R,
          gfx->Default_palette[gfx->Color[2]].G,
          gfx->Default_palette[gfx->Color[2]].B);
        Set_color(
          MC_White,
          gfx->Default_palette[gfx->Color[3]].R,
          gfx->Default_palette[gfx->Color[3]].G,
          gfx->Default_palette[gfx->Color[3]].B);
      }
      Update_window_area(6, 14, 173, 16);
      Display_cursor();
    }

    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_SETTINGS, "SKINS");

    switch(clicked_button)
    {
      case 1 : // OK
        break;
      case 2 : // double-click file: do nothing
        break;
      case 3 : // doesn't happen
        break;
      case 4 : // a file is selected
          need_load=1;
        break;
      case 5 : // Font dropdown
      {
        T_Fileselector_item* fontName;
        selected_font = Window_attribute2; // Get the index of the chosen font.
        fontName = Get_item_by_index(&Font_files_list,selected_font);
        new_font = Load_font(fontName->Full_name, 1);
        if (new_font)
        {
          free(Menu_font);
          Menu_font = new_font;
          Print_in_window( 172, 33,"Font:"            ,MC_Black,MC_Light);
          Print_in_window_limited(font_dropdown->Pos_X+2,font_dropdown->Pos_Y+(font_dropdown->Height-7)/2,
            fontName->Short_name,(byte)strlen(fontName->Short_name) ,MC_Black,MC_Light);
          Update_window_area(172, 33, 8 * 5, 8);
        }
        break;
      }
      // 6: Cancel
      case 7 : // Cursor
        selected_cursor = Window_attribute2;
        break;
      case 8: // Display limits
        showlimits = !showlimits;
        Hide_cursor();
        Print_in_window(175, 90, (showlimits)?"X":" ", MC_Black, MC_Light);
        Display_cursor();
        break;
      case 9: // Separate colors
        separatecolors = !separatecolors;
        Hide_cursor();
        Print_in_window(175, 114, (separatecolors)?"X":" ", MC_Black, MC_Light);
        Display_cursor();
        break;
    }
  }
  while ( (clicked_button!=1) && (clicked_button !=6) && (Key != KEY_ESCAPE) && !Quit_is_required);

  if(clicked_button == 1)
  {
    if (gfx != NULL)
    {
      Set_current_skin(skinsdir, gfx);
    }
    // (Re-)load the selected font
    new_font = Load_font(Get_item_by_index(&Font_files_list,selected_font)->Full_name, 1);
    if (new_font)
    {
      const char * fname;

      free(Menu_font);
      Menu_font = new_font;
      fname = Get_item_by_index(&Font_files_list,selected_font)->Full_name;
      free(Config.Font_file);
      Config.Font_file = (char *)strdup(fname);
    }
    // Confirm the change of cursor shape
    Config.Cursor = selected_cursor;
    Config.Display_image_limits = showlimits;
    Config.Separate_colors = separatecolors;

    // Now find the best colors for the new skin in the current palette
    // and remap the skin
    Compute_optimal_menu_colors(Main.palette);

  }

  // We don't want to keep the skin's palette, as this would corrupt the current picture's one.
  Set_palette(Main.palette);

  Close_window();
  Unselect_button(btn);

  // Raffichage du menu pour que les inscriptions qui y figurent soient retracées avec la nouvelle fonte
  Display_menu();
  // Redraw all buttons, to ensure all specific sprites are in place.
  // This is necessary for multi-state buttons, for example Freehand.
  for (button=0; button<NB_BUTTONS; button++)
  {
    byte state=Buttons_Pool[button].Pressed;
    switch(button)
    {
      case BUTTON_MAGNIFIER:
        state|=Main.magnifier_mode;
        break;
      case BUTTON_EFFECTS:
        state|=Any_effect_active();
        break;
    }
    Draw_menu_button(button,state);
  }
  Display_cursor();
  free(skinsdir);
}


//---------------------------- Changement de page ----------------------------
void Button_Page(int btn)
{
  byte   factor_index;
  T_Document temp_doc;

  Hide_cursor();

  if (Config.Sync_views)
    Copy_view_to_spare();

  // First update the page descriptors before swapping them
  Upload_infos_page(&Main);
  Upload_infos_page(&Spare);

  // SWAP
  memcpy(&temp_doc, &Main, sizeof(T_Document));
  memcpy(&Main, &Spare, sizeof(T_Document));
  memcpy(&Spare, &temp_doc, sizeof(T_Document));

  Pixel_preview=(Main.magnifier_mode)?Pixel_preview_magnifier:Pixel_preview_normal;

  //Redraw_layered_image();
  // replaced by
  Update_buffers(Main.image_width, Main.image_height);
  Update_depth_buffer();
  Update_screen_targets();
  End_of_modification();
  // --

  // A la fin, on affiche l'écran
  for (factor_index=0; ZOOM_FACTOR[factor_index]!=Main.magnifier_factor; factor_index++);
  //Change.magnifier_factor(factor_index,0);
  Compute_magnifier_data();
  if (Main.magnifier_mode)
    Pixel_preview=Pixel_preview_magnifier;
  else
    Pixel_preview=Pixel_preview_normal;
  Compute_limits();
  Compute_paintbrush_coordinates();

  Set_palette(Main.palette);
  Compute_optimal_menu_colors(Main.palette);
  Check_menu_mode();
  Display_all_screen();
  Unselect_button(btn);
  Draw_menu_button(BUTTON_MAGNIFIER,Main.magnifier_mode);
  // Tilemap mode might be different
  Draw_menu_button(BUTTON_EFFECTS,Any_effect_active());
  Display_menu();

  Display_cursor();
}


// -- Copie de page ---------------------------------------------------------

void Copy_image_only(void)
{
  word old_width=Spare.image_width;
  word old_height=Spare.image_height;

  if (Backup_and_resize_the_spare(Main.image_width,Main.image_height))
  {
    byte i;

    for (i=0; i<Spare.backups->Pages->Nb_layers; i++)
    {
      if (i == Spare.current_layer)
      {
        // Copy the current layer
        memcpy(Spare.backups->Pages->Image[i].Pixels,Main.backups->Pages->Image[Main.current_layer].Pixels,Main.image_width*Main.image_height);
      }
      else
      {
        // Resize the original layer
        Copy_part_of_image_to_another(
        Spare.backups->Pages->Next->Image[i].Pixels,0,0,Min(old_width,Spare.image_width),
        Min(old_height,Spare.image_height),old_width,
        Spare.backups->Pages->Image[i].Pixels,0,0,Spare.image_width);
      }
    }

    // Copie des dimensions de l'image
    /*
      C'est inutile, le "Backuper et redimensionner brouillon" a déjà modifié
      ces valeurs pour qu'elles soient correctes.
    */
    /*
    Spare.image_width=Main.image_width;
    Spare.image_height=Main.image_height;
    */

    Copy_view_to_spare();

    // Update the visible buffer of the spare.
    // It's a bit complex because at the moment, to save memory,
    // the spare doesn't have a full visible_buffer + depth_buffer,
    // so I can't use exactly the same technique as for Main page.
    // (It's the same reason that the "Page" function gets complex,
    // it needs to rebuild a depth buffer only, trusting the
    // depth buffer that was already available in Spare_.)
    Update_spare_buffers(Spare.image_width,Spare.image_height);
    Redraw_spare_image();

  }
  else
    Message_out_of_memory();
}


void Copy_some_colors(void)
{
  short index;
  byte confirmation=0;
  static byte mask_color_to_copy[256]; // static to use less stack

  memset(mask_color_to_copy,1,256);
  Menu_tag_colors("Tag colors to copy",mask_color_to_copy,&confirmation,0, NULL, 0xFFFF);

  if (confirmation)
  {
    // Make a backup with the same pixel data as previous history steps
    Backup_the_spare(LAYER_NONE);
    for (index=0; index<256; index++)
    {
      if (mask_color_to_copy[index])
        memcpy(Spare.palette+index,Main.palette+index,
               sizeof(T_Components));
    }
  }
}


void Button_Copy_page(int btn)
{
  short clicked_button;


  Open_window(168,137,"Copy to spare page");

  Window_set_normal_button(10, 20,148,14,"Pixels + palette" , 0,1,KEY_RETURN); // 1
  Window_set_normal_button(10, 37,148,14,"Pixels only"      , 3,1,KEY_x); // 2
  Window_set_normal_button(10, 54,148,14,"Palette only"     , 1,1,KEY_p); // 3
  Window_set_normal_button(10, 71,148,14,"Some colors only" , 6,1,KEY_c); // 4
  Window_set_normal_button(10, 88,148,14,"Palette and remap",13,1,KEY_r); // 5
  Window_set_normal_button(44,114, 80,14,"Cancel"           , 0,1,KEY_ESC); // 6
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_PAGE, NULL);
    else if (Is_shortcut(Key,0x200+BUTTON_PAGE))
      clicked_button=6;
  }
  while (clicked_button<=0 && !Quit_is_required);

  Close_window();
  Display_cursor();

  switch (clicked_button)
  {
    case 1: // Pixels+palette
      // backup is done by the following function
      Copy_image_only();
      // copie de la palette
      memcpy(Spare.palette,Main.palette,sizeof(T_Palette));
      // Equivalent of 'end_of_modifications' for spare.
      Update_spare_buffers(Spare.image_width,Spare.image_height);
      Redraw_spare_image();
      Spare.image_is_modified=1;
      if (Spare.tilemap_mode)
        Disable_tilemap(&Spare);
      break;

    case 2: // Pixels only
      // backup is done by the following function
      Copy_image_only();
      // Equivalent of 'end_of_modifications' for spare.
      Update_spare_buffers(Spare.image_width,Spare.image_height);
      Redraw_spare_image();
      Spare.image_is_modified=1;
      if (Spare.tilemap_mode)
        Disable_tilemap(&Spare);
      break;

    case 3: // Palette only
      Backup_the_spare(LAYER_NONE);
      // Copy palette
      memcpy(Spare.palette,Main.palette,sizeof(T_Palette));
      // Equivalent of 'end_of_modifications' for spare.
      Update_spare_buffers(Spare.image_width,Spare.image_height);
      Redraw_spare_image();
      Spare.image_is_modified=1;
      break;

    case 4: // Some colors
      // Will backup if needed
      Copy_some_colors();
      break;

    case 5: // Palette and remap
      Backup_the_spare(LAYER_ALL);
      Remap_spare();
      // Copy palette
      memcpy(Spare.palette,Main.palette,sizeof(T_Palette));
      // Equivalent of 'end_of_modifications' for spare.
      Update_spare_buffers(Spare.image_width,Spare.image_height);
      Redraw_spare_image();
      Spare.image_is_modified=1;
      break;
  }

  Hide_cursor();
  Unselect_button(btn);
  Display_cursor();
}


// -- Suppression d'une page -------------------------------------------------
void Button_Kill(int btn)
{
  if ( (Main.backups->List_size==1)
    || (!Confirmation_box("Delete the current page?")) )
  {
    if (Main.backups->List_size==1)
      Warning_message("You can't delete the last page.");
    Hide_cursor();
    Unselect_button(btn);
    Display_cursor();
  }
  else
  {
    Hide_cursor();
    Free_current_page();

    Set_palette(Main.palette);
    Compute_optimal_menu_colors(Main.palette);

    Display_all_screen();
    Unselect_button(btn);
    Draw_menu_button(BUTTON_MAGNIFIER,Main.magnifier_mode);
    Display_menu();
    Display_cursor();
  }
}


//------------------------- Dimensions Image/Screen ---------------------------

void Check_mode_button(short x_pos, short y_pos, byte state)
{
  byte color;

  switch (state & 0x7F)
  {
    case 0 : color=MC_White; break;
    case 1 : color=MC_Light; break;
    case 2 : color=MC_Dark; break;
    default: color=MC_Black;
  }
  Window_rectangle(x_pos, y_pos, 9, 3, color);
  Update_window_area(x_pos, y_pos,9,3);
}

/// Number of video modes to display in the resolution menu
#define MODELIST_LINES 10

void Display_modes_list(short list_start, short cursor_position)
{
  short index,current_mode;
  short y_pos;
  byte  text_color,background_color;
  char str[29];
  char *ratio;

  for (current_mode=list_start,index=0; index<MODELIST_LINES && current_mode < Nb_video_modes ; index++,current_mode++)
  {
    y_pos=86+(index<<3);
    Check_mode_button(19,y_pos+2,Video_mode[current_mode].State);

    if (cursor_position!=index)
    {
      background_color =MC_Black;
      if ((Video_mode[current_mode].State & 3) == 3)
        text_color=MC_Dark;
      else
        text_color=MC_Light;
    }
    else
    {
      background_color =MC_Dark;
      if ((Video_mode[current_mode].State & 3) == 3)
        text_color=MC_Light;
      else
        text_color=MC_White;
    }
    snprintf(str, sizeof(str), "%4hu %4hu", Video_mode[current_mode].Width, Video_mode[current_mode].Height);

    if(Video_mode[current_mode].Fullscreen == 0)
      memcpy(str+9,"   Window          ",20);
    else
    {
      memcpy(str+9," Fullscreen ",13);

      if (Video_mode[current_mode].Width*3 == Video_mode[current_mode].Height*4)
        ratio="    4:3";
      else if (Video_mode[current_mode].Width*9 == Video_mode[current_mode].Height*16)
        ratio="   16:9";
      else if (Video_mode[current_mode].Width*10 == Video_mode[current_mode].Height*16)
        ratio="  16:10";
      else if (Video_mode[current_mode].Width*145 == Video_mode[current_mode].Height*192)
        ratio="192:145";
      else if (Video_mode[current_mode].Width*2 == Video_mode[current_mode].Height*3)
        ratio="    3:2";
      else if (Video_mode[current_mode].Width*3 == Video_mode[current_mode].Height*5)
        ratio="    5:3";
      else if (Video_mode[current_mode].Width*4 == Video_mode[current_mode].Height*5)
        ratio="    5:4";
      else if (Video_mode[current_mode].Width*16 == Video_mode[current_mode].Height*25)
        ratio="  25:16";
      else
        ratio="       ";

      strcpy(str+21,ratio);
    }

    Print_in_window(38,y_pos,str,text_color,background_color);
  }
}


void Scroll_list_of_modes(short list_start, short cursor_position, int * selected_mode)
{
  Hide_cursor();
  *selected_mode=list_start+cursor_position;
  if (Window_scroller_button_list->Position!=list_start)
  {
    Window_scroller_button_list->Position=list_start;
    Window_draw_slider(Window_scroller_button_list);
  }
  Display_modes_list(list_start,cursor_position);
  Display_cursor();
}

void Button_Resolution(int btn)
{
  short clicked_button;
  int   selected_mode;
  word  chosen_width;
  word  chosen_height;
  byte  chosen_pixel;
  short list_start;
  short cursor_position;
  short temp;
  char  str[8];
  T_Special_button * input_width_button, * input_button_height;
  T_Dropdown_button * pixel_button;
  static const char *pixel_ratio_labels[PIXEL_MAX] ={
    "Normal    (1x1)",
    "Wide      (2x1)",
    "Tall      (1x2)",
    "Double    (2x2)",
    "Triple    (3x3)",
    "Wide2     (4x2)",
    "Tall2     (2x4)",
    "Tall3     (3x4)",
    "Quadruple (4x4)"};

  Open_window(299,190,"Picture & screen sizes");

  Print_in_window( 12, 21,"Picture size:"   ,MC_Dark,MC_Light);
  Window_display_frame      ( 8,17,195, 33);

  Window_set_normal_button(223, 18,67,14,"OK"      ,0,1,KEY_RETURN); // 1
  Window_set_normal_button(223, 35,67,14,"Cancel"  ,0,1,KEY_ESC);  // 2

  Print_in_window_underscore( 12, 37,"Width:",MC_Dark,MC_Light,1);
  input_width_button=Window_set_input_button_s( 60, 35,4,KEY_w);     // 3

  Print_in_window_underscore(108, 37,"Height:",MC_Dark,MC_Light,1);
  input_button_height=Window_set_input_button_s(164, 35,4,KEY_h); // 4

  Window_display_frame      ( 8,72,283,110);
  Window_display_frame_in   (37,84,228,84);
  Window_rectangle          (38,85,226,82,MC_Black);
  Print_in_window( 16, 76,"OK"              ,MC_Dark,MC_Light);
  Print_in_window( 55, 76,"X    Y"          ,MC_Dark,MC_Light);
  Print_in_window(120, 76,"Win / Full"      ,MC_Dark,MC_Light);
  Print_in_window(219, 76,"Ratio"           ,MC_Dark,MC_Light);
  Print_in_window( 30,170,"\03"             ,MC_Dark,MC_Light);
  Print_in_window( 62,170,"OK"              ,MC_Dark,MC_Light);
  Print_in_window(102,170,"Imperfect"       ,MC_Dark,MC_Light);
  Print_in_window(196,170,"Unsupported"     ,MC_Dark,MC_Light);
  Window_set_special_button(38,86,225,80,0);                       // 5

  selected_mode=Current_resolution;
  if (selected_mode>=MODELIST_LINES/2 && Nb_video_modes > MODELIST_LINES)
  {
    if (selected_mode<Nb_video_modes-MODELIST_LINES/2)
    {
      list_start=selected_mode-(MODELIST_LINES/2-1);
      cursor_position=(MODELIST_LINES/2-1);
    }
    else
    {
      list_start=Nb_video_modes-MODELIST_LINES;
      cursor_position=selected_mode-list_start;
    }
  }
  else
  {
    list_start=0;
    cursor_position=selected_mode;
  }
  Window_set_scroller_button(271,85,81,Nb_video_modes,MODELIST_LINES,list_start); // 6

  chosen_pixel=Pixel_ratio;
  Print_in_window( 12, 57,"Pixel size:"    ,MC_Dark,MC_Light);
  pixel_button=Window_set_dropdown_button(108,55,17*8,11,17*8,pixel_ratio_labels[Pixel_ratio],1,0,1,LEFT_SIDE|RIGHT_SIDE,0);    // 7
  for (temp=0;temp<PIXEL_MAX;temp++)
    Window_dropdown_add_item(pixel_button,temp,pixel_ratio_labels[temp]);

  // 10 little buttons for the state of each visible mode
  for (temp=0; temp<MODELIST_LINES && temp < Nb_video_modes; temp++)
    Window_set_normal_button(17,86+(temp<<3),13,7,"",0,1,KEY_NONE);// 8..17

  // Dummy buttons as explainations of colors
  Window_draw_normal_bouton( 16,170,13,7,"",0,0);
  Check_mode_button( 18,172,0);
  Window_draw_normal_bouton( 48,170,13,7,"",0,0);
  Check_mode_button( 50,172,1);
  Window_draw_normal_bouton( 88,170,13,7,"",0,0);
  Check_mode_button( 90,172,2);
  Window_draw_normal_bouton(182,170,13,7,"",0,0);
  Check_mode_button(184,172,3);


  chosen_width=Main.image_width;
  Num2str(chosen_width,str,4);
  Window_input_content(input_width_button,str);

  chosen_height=Main.image_height;
  Num2str(chosen_height,str,4);
  Window_input_content(input_button_height,str);

  Display_modes_list(list_start,cursor_position);

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();

    switch (clicked_button)
    { case -1: case 0: case 1: case 2:
        break;

      case 3 : // Largeur
        Num2str(chosen_width,str,4);
        Readline(62,37,str,4,INPUT_TYPE_INTEGER);
        chosen_width=atoi(str);
        // On corrige les dimensions
        if (chosen_width==0)
        {
          chosen_width=1;
          Num2str(chosen_width,str,4);
          Window_input_content(input_width_button,str);
        }
        Display_cursor();
        break;

      case 4 : // Height
        Num2str(chosen_height,str,4);
        Readline(166,37,str,4,INPUT_TYPE_INTEGER);
        chosen_height=atoi(str);
        // On corrige les dimensions
        if (chosen_height==0)
        {
          chosen_height=1;
          Num2str(chosen_height,str,4);
          Window_input_content(input_button_height,str);
        }
        Display_cursor();
        break;

      case 5: // Liste des modes
        temp=(((Mouse_Y-Window_pos_Y)/Menu_factor_Y)-86)>>3;
        if (temp<Nb_video_modes && ((Mouse_K==2) || (temp!=cursor_position)))
        {
          Hide_cursor();
          if (temp!=cursor_position)
          {
            cursor_position=temp;
            Display_modes_list(list_start,cursor_position);
          }
          selected_mode=list_start+cursor_position;
          // Si l'utilisateur s'est servi du bouton droit de la souris:
          if (Mouse_K==2)
          {
            // On affecte également les dimensions de l'image:
            chosen_width=Video_mode[selected_mode].Width/Pixel_width;
            Num2str(chosen_width,str,4);
            Window_input_content(input_width_button,str);

            chosen_height=Video_mode[selected_mode].Height/Pixel_height;
            Num2str(chosen_height,str,4);
            Window_input_content(input_button_height,str);
          }
          Display_cursor();
        }
        Wait_end_of_click();
        break;

      case 6: // Scroller
        list_start=Window_attribute2;
        selected_mode=list_start+cursor_position;
        Display_modes_list(list_start,cursor_position);
        break;

      case 7: // Pixel size
        chosen_pixel=Window_attribute2;
        break;

      default: // Boutons de tag des états des modes
        temp=list_start+clicked_button-8;
        if (Video_mode[temp].Fullscreen==1 && // On n'a pas le droit de cocher le mode fenêtré
            !(Video_mode[temp].State & 128)) // Ni ceux non détectés par SDL
        {
          if (Window_attribute1==LEFT_SIDE)
            Video_mode[temp].State=((Video_mode[temp].State&0x7F)+1)&3;
          else
            Video_mode[temp].State=((Video_mode[temp].State&0x7F)+3)&3;

          Hide_cursor();
          //Check_mode_button(19,16+(clicked_button<<3),Video_mode[temp].State);
          Display_modes_list(list_start,cursor_position);
          Display_cursor();
        }
    }

    // Gestion des touches de déplacement dans la liste
    switch (Key)
    {
      case KEY_UP : // Haut
        if (cursor_position>0)
          cursor_position--;
        else
          if (list_start>0)
            list_start--;
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case KEY_DOWN : // Bas
        if (cursor_position<(MODELIST_LINES-1) && cursor_position<(Nb_video_modes-1))
          cursor_position++;
        else
          if (list_start<Nb_video_modes-MODELIST_LINES)
            list_start++;
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case KEY_PAGEUP : // PageUp
        if (cursor_position>0)
          cursor_position=0;
        else
        {
          if (list_start>(MODELIST_LINES-1))
            list_start-=(MODELIST_LINES-1);
          else
            list_start=0;
        }
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case KEY_PAGEDOWN : // PageDown
        if (Nb_video_modes<MODELIST_LINES)
          cursor_position=Nb_video_modes-1;
        else if (cursor_position<(MODELIST_LINES-1))
          cursor_position=(MODELIST_LINES-1);
        else
        {
          if (list_start<Nb_video_modes-(MODELIST_LINES*2-1))
            list_start+=(MODELIST_LINES-1);
          else
            list_start=Nb_video_modes-MODELIST_LINES;
        }
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case KEY_HOME : // Home
        list_start=0;
        cursor_position=0;
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      case KEY_END : // End
        if (Nb_video_modes<MODELIST_LINES)
          cursor_position=Nb_video_modes-1;
        else
        {
          list_start=Nb_video_modes-MODELIST_LINES;
          cursor_position=(MODELIST_LINES-1);
        }
        Scroll_list_of_modes(list_start,cursor_position,&selected_mode);
        Key=0;
        break;
      default:
        if (Is_shortcut(Key,0x100+BUTTON_HELP))
        {
          Window_help(BUTTON_RESOL, NULL);
          Key=0;
          break;
        }
    }

  }
  while ((clicked_button!=1) && (clicked_button!=2) && !Quit_is_required);

  Close_window();

  if (clicked_button==1) // OK
  {
    if (Main.magnifier_mode)
      Unselect_button(BUTTON_MAGNIFIER);

    if ( (chosen_width!=Main.image_width)
      || (chosen_height!=Main.image_height) )
    {
      Resize_image(chosen_width,chosen_height);
      End_of_modification();
      Tilemap_update();
    }

    if ((Video_mode[selected_mode].State & 3) == 3 ||
      Init_mode_video(
        Video_mode[selected_mode].Width,
        Video_mode[selected_mode].Height,
        Video_mode[selected_mode].Fullscreen,
        chosen_pixel))
    {
      Error(0); // Tell user it is an invalid mode
      Pixel_ratio=PIXEL_SIMPLE;
      Init_mode_video(
        Video_mode[Current_resolution].Width,
        Video_mode[Current_resolution].Height,
        Video_mode[Current_resolution].Fullscreen,
        Pixel_ratio);
    }

    Display_menu();
    Reposition_palette();
    Display_all_screen();
  }
  Paintbrush_X = Mouse_X;
  Paintbrush_Y = Mouse_Y;


  Unselect_button(btn);
  Display_cursor();
}


void Button_Safety_resolution(int btn)
{
  // In windowed mode, do nothing
  if (Current_resolution==0)
  {
    Hide_cursor();
    Unselect_button(btn);
    Display_cursor();
    return;
  }

  Hide_cursor();

  Unselect_button(BUTTON_MAGNIFIER);

  Init_mode_video(
    Video_mode[0].Width,
    Video_mode[0].Height,
    Video_mode[0].Fullscreen,
    PIXEL_SIMPLE);
  Current_resolution=0;
  Display_menu();
  Reposition_palette();
  Display_all_screen();

  Unselect_button(btn);
  // Le pinceau est affiché à la position du clic et pas
  Display_cursor();
}


//------------------ Gestion des boutons de dessin à la main -----------------

void Button_Draw(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(Selected_freehand_mode);
  Display_cursor();
}


void Button_Draw_switch_mode(int btn)
{
  char icon;

  T_Dropdown_button dropdown;
  T_Dropdown_choice *item = NULL;
  int i;
  static const char text[4][14] =
    {"Continuous", "Discontinuous", "Single", "Contour fill"};

  dropdown.Pos_X         =Buttons_Pool[btn].X_offset;
  dropdown.Pos_Y         =Buttons_Pool[btn].Y_offset;
  dropdown.Height        =Buttons_Pool[btn].Height;
  dropdown.Dropdown_width=14*8;
  dropdown.First_item    =NULL;
  dropdown.Bottom_up     =1;

  Hide_cursor();

  // If we get here from a keyboard shortcut, don't show the menu and directly
  // switch to the next drawing mode.
  if (Mouse_K != 0) {

    for(i = 0; i < 4; i++) {
      Window_dropdown_add_item(&dropdown, i, text[i]);
    }

    item=Dropdown_activate(&dropdown,0,Menu_Y);
  }

  if (item)
  {
    Selected_freehand_mode = item->Number;
  } else {
    Selected_freehand_mode++;
    if (Selected_freehand_mode>OPERATION_FILLED_CONTOUR)
      Selected_freehand_mode=OPERATION_CONTINUOUS_DRAW;
  }

  switch(Selected_freehand_mode)
  {
    default:
    case OPERATION_CONTINUOUS_DRAW:
      icon=-1;
      break;
    case OPERATION_DISCONTINUOUS_DRAW:
      icon=MENU_SPRITE_DISCONTINUOUS_DRAW;
      break;
    case OPERATION_POINT_DRAW:
      icon=MENU_SPRITE_POINT_DRAW;
      break;
    case OPERATION_FILLED_CONTOUR:
      icon=MENU_SPRITE_CONTOUR_DRAW;
      break;
  }
  Display_sprite_in_menu(btn,icon);
  Draw_menu_button(btn,BUTTON_PRESSED);
  Start_operation_stack(Selected_freehand_mode);

  Display_cursor();
  Window_dropdown_clear_items(&dropdown);
}


// -- Gestion des boutons de rectangle vide et plein ------------------------

void Button_Empty_rectangle(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_EMPTY_RECTANGLE);
  Display_cursor();
}


void Button_Filled_rectangle(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_FILLED_RECTANGLE);
  Display_cursor();
}


// -- Gestion des boutons de cercle (ellipse) vide et plein(e) --------------

void Button_circle_ellipse(int btn)
{
  word operation;

  switch (btn)
  {
    default:
    case BUTTON_CIRCLES:
      operation = OPERATION_EMPTY_CIRCLE_CTR;
      break;
    case BUTTON_FILLCIRC:
      operation = OPERATION_FILLED_CIRCLE_CTR;
      break;
    case BUTTON_SPHERES:
      operation = OPERATION_GRAD_CIRCLE_CTR;
      break;
  }
  operation += Selected_circle_ellipse_mode;  // CIRCLE_CTR/CIRCLE_CRN/ELLIPSE_CTR/ELLIPSE_CRN;
  Hide_cursor();
  Start_operation_stack(operation);
  Display_cursor();
}

void Button_Circle_switch_mode(int btn)
{
  T_Dropdown_button dropdown;
  T_Dropdown_choice *item = NULL;
  int i;
  static const char * text[4] =
    { "Circle (center/radius)", "Circle (corners)",
      "Ellipse (center/radiuses)", "Ellipse (corners)" };

  dropdown.Pos_X         =Buttons_Pool[btn].X_offset;
  dropdown.Pos_Y         =Buttons_Pool[btn].Y_offset;
  dropdown.Height        =Buttons_Pool[btn].Height;
  dropdown.Dropdown_width=26*8;
  dropdown.First_item    =NULL;
  dropdown.Bottom_up     =1;

  Hide_cursor();

  // If we get here from a keyboard shortcut, don't show the menu and directly
  // switch to the next drawing mode.
  if (Mouse_K != 0) {
    for(i = 0; i < 4; i++) {
      Window_dropdown_add_item(&dropdown, i, text[i]);
    }
    item=Dropdown_activate(&dropdown,0,Menu_Y);
  }

  if (item)
  {
    Selected_circle_ellipse_mode = item->Number;
  } else {
    Selected_circle_ellipse_mode = (Selected_circle_ellipse_mode + 1) & 3;
  }

  Display_sprite_in_menu(BUTTON_CIRCLES,
    (Selected_circle_ellipse_mode >= 2) ? MENU_SPRITE_ELLIPSES : -1);
  Draw_menu_button(BUTTON_CIRCLES,BUTTON_RELEASED);
  Display_sprite_in_menu(BUTTON_FILLCIRC,
    (Selected_circle_ellipse_mode >= 2) ? MENU_SPRITE_ELLIPSES : -1);
  Draw_menu_button(BUTTON_FILLCIRC,BUTTON_RELEASED);
  Display_sprite_in_menu(BUTTON_SPHERES,
    (Selected_circle_ellipse_mode >= 2) ? MENU_SPRITE_GRAD_ELLIPSE : -1);
  Draw_menu_button(BUTTON_SPHERES,BUTTON_RELEASED);
  Draw_menu_button(btn,BUTTON_PRESSED);

  Display_cursor();
  Button_circle_ellipse(btn);
  Window_dropdown_clear_items(&dropdown);
}

// -- Gestion du menu des dégradés ------------------------------------------
void Draw_button_gradient_style(short x_pos,short y_pos,int technique)
{
  short line;

  // On commence par afficher les 2 côtés qui constituent le dégradé de base:
    // Côté gauche (noir)
  Window_rectangle(x_pos+2, y_pos+2, 6, 10, MC_Black);
    // Côté droit (blanc)
  Window_rectangle(x_pos+8, y_pos+2, 5, 10, MC_White);

  switch(technique)
  {
    case 1 : // Dégradé de trames simples
      // Au centre, on place 10 lignes tramées simplement
      for (line=2;line<2+10;line++)
        if (line&1)
        {
          // Lignes impaires
          Pixel_in_window(x_pos+ 5,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 7,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 8,y_pos+line,MC_Black);
        }
        else
        {
          // Lignes paires
          Pixel_in_window(x_pos+ 6,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 9,y_pos+line,MC_Black);
        }
      break;
    case 2 : // Dégradé de trames étendues
      // Au centre, on place 10 lignes tramées de façon compliquée
      for (line=2;line<2+10;line++)
        if (line&1)
        {
          // Lignes impaires
          Pixel_in_window(x_pos+ 7,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 8,y_pos+line,MC_Black);
          Pixel_in_window(x_pos+10,y_pos+line,MC_Black);
        }
        else
        {
          // Lignes paires
          Pixel_in_window(x_pos+ 4,y_pos+line,MC_White);
          Pixel_in_window(x_pos+ 6,y_pos+line,MC_White);
        }
  }

  Update_window_area(x_pos+2,y_pos+2,10,10);
}

void Load_gradient_data(int index)
{
  if (Main.backups->Pages->Gradients->Range[index].Start>Main.backups->Pages->Gradients->Range[index].End)
    Error(0);
  Gradient_lower_bound =Main.backups->Pages->Gradients->Range[index].Start;
  Gradient_upper_bound =Main.backups->Pages->Gradients->Range[index].End;
  Gradient_is_inverted          =Main.backups->Pages->Gradients->Range[index].Inverse;
  Gradient_random_factor=Main.backups->Pages->Gradients->Range[index].Mix+1;

  Gradient_bounds_range=(Gradient_lower_bound<Gradient_upper_bound)?
                            Gradient_upper_bound-Gradient_lower_bound:
                            Gradient_lower_bound-Gradient_upper_bound;
  Gradient_bounds_range++;

  switch(Main.backups->Pages->Gradients->Range[index].Technique)
  {
    case 0 : // Degradé de base
      Gradient_function=Gradient_basic;
      break;
    case 1 : // Dégradé de trames simples
      Gradient_function=Gradient_dithered;
      break;
    case 2 : // Dégradé de trames étendues
      Gradient_function=Gradient_extra_dithered;
  }
}

/// draw the gradient preview for the Gradation menu
static void Draw_gradient_preview(short start_x,short start_y,short width,short height,int index)
{
  short x_pos; // Variables de balayage du block en bas de l'écran.
  short y_pos;
  short end_x;
  short end_y;

  Load_gradient_data(index);

  start_x=Window_pos_X+(start_x*Menu_factor_X);
  start_y=Window_pos_Y+(start_y*Menu_factor_Y);

  Gradient_total_range=width*Menu_factor_X;

  end_x=start_x+Gradient_total_range;
  end_y=start_y+(height*Menu_factor_Y);

  for (y_pos=start_y;y_pos<end_y;y_pos++)
    for (x_pos=start_x;x_pos<end_x;x_pos++)
      Gradient_function(x_pos-start_x,x_pos,y_pos);
  Update_rect(start_x,start_y,width*Menu_factor_X,height*Menu_factor_Y);
}

/// Tag colors from a gradient range
static void Tag_color_gradient_range(const T_Gradient_range * range)
{
  Tag_color_range(range->Start, range->End);
}

/// Print the Color cycling speed in Hz, ms and in DP2E unit
static void Print_color_cycling_details(short y_pos)
{
  char str[24];

  Num2str(Main.backups->Pages->Gradients->Range[Current_gradient].Speed, str, 3);
  Print_in_window(73+134, y_pos, str, MC_Black, MC_Light);
  if (Main.backups->Pages->Gradients->Range[Current_gradient].Speed > 0)
  {
    snprintf(str, sizeof(str), "Speed %7.4fHz %4ums",
             Main.backups->Pages->Gradients->Range[Current_gradient].Speed * 0.2856,
             (unsigned)(1000.0 / (Main.backups->Pages->Gradients->Range[Current_gradient].Speed * 0.2856)));
    Print_in_window(8, y_pos, str, MC_Black, MC_Light);
  }
  else
    Print_in_window(8, y_pos, "               STOPPED", MC_Black, MC_Light);
}

void Button_Gradients(int btn)
{
  short clicked_button;
  char  str[4];
  T_Gradient_array backup_gradients;
  int   old_current_gradient;
  T_Scroller_button * mix_scroller;
  T_Scroller_button * speed_scroller;
  T_Scroller_button * gradient_scroller;
  short old_mouse_x;
  short old_mouse_y;
  byte  old_mouse_k;
  byte  temp_color;
  byte  first_color;
  byte  last_color;
  byte  color;
  byte  click;
  int  changed_gradient_index;
  byte cycling_mode=Cycling_mode;

  (void)btn;
  // Enable cycling while this window is open
  Cycling_mode=1;

  Gradient_pixel=Pixel;
  old_current_gradient=Current_gradient;
  changed_gradient_index=0;
  memcpy(&backup_gradients,Main.backups->Pages->Gradients,sizeof(T_Gradient_array));

  Open_window(235,146+12,"Gradation menu");

  Window_set_palette_button(48,19);                            // 1
  // Slider for gradient selection
  gradient_scroller=Window_set_scroller_button(218,20,75,16,1,Current_gradient);  // 2
  // Slider for mix
  mix_scroller = Window_set_scroller_button(31,20,84,256,1,
    Main.backups->Pages->Gradients->Range[Current_gradient].Mix);                      // 3
  // Direction
  Window_set_normal_button(8,20,15,14,
    (Main.backups->Pages->Gradients->Range[Current_gradient].Inverse)?"\033":"\032",0,1,KEY_TAB); // 4
  // Technique
  Window_set_normal_button(8,90,15,14,"",0,1,KEY_TAB|GFX2_MOD_SHIFT); // 5
  Draw_button_gradient_style(8,90,Main.backups->Pages->Gradients->Range[Current_gradient].Technique);

  Window_set_normal_button(178,128+12,51,14,"OK",0,1,KEY_RETURN);     // 6
  Window_set_normal_button(123,128+12,51,14,"Cancel",0,1,KEY_ESC);  // 7
  // Scrolling speed
  //((250/2 = 125)+1 = 126) +24 = 150
  speed_scroller = Window_set_horizontal_scroller_button(79, 111, 150, COLOR_CYCLING_SPEED_MAX + 1, 1,
                                              Main.backups->Pages->Gradients->Range[Current_gradient].Speed);  // 8
  Print_color_cycling_details(113 + 14);

  Print_in_window(5,58,"MIX",MC_Dark,MC_Light);

  // Cycling mode on/off
  Window_set_normal_button(8,109,62,14,"",0,1,KEY_NONE); // 9
  Print_in_window(11,112,"Cycling",cycling_mode?MC_Black:MC_Dark,MC_Light);

  // On tagge les couleurs qui vont avec
  Tag_color_gradient_range(&Main.backups->Pages->Gradients->Range[Current_gradient]);

  Num2str(Current_gradient+1,str,2);
  Print_in_window(215,100,str,MC_Black,MC_Light);

  // On affiche le cadre autour de la préview
  Window_display_frame_in(7,127+12,110,16);
  // On affiche la preview
  Draw_gradient_preview(8,128+12,108,14,Current_gradient);

  first_color=last_color=(Main.backups->Pages->Gradients->Range[Current_gradient].Inverse)?Main.backups->Pages->Gradients->Range[Current_gradient].End:Main.backups->Pages->Gradients->Range[Current_gradient].Start;
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    old_mouse_x=Mouse_X;
    old_mouse_y=Mouse_Y;
    old_mouse_k=Mouse_K;
    if (changed_gradient_index)
    {
      // User has changed which gradient (0-15) he's watching
      changed_gradient_index=0;

      Hide_cursor();

      // On affiche la valeur sous la jauge
      Num2str(Current_gradient+1,str,2);
      Print_in_window(215,100,str,MC_Black,MC_Light);

      // On tagge les couleurs qui vont avec
      Tag_color_gradient_range(&Main.backups->Pages->Gradients->Range[Current_gradient]);

      // On affiche le sens qui va avec
      Print_in_window(12,23,(Main.backups->Pages->Gradients->Range[Current_gradient].Inverse)?"\033":"\032",MC_Black,MC_Light);

      // On raffiche le mélange (jauge) qui va avec
      mix_scroller->Position=Main.backups->Pages->Gradients->Range[Current_gradient].Mix;
      Window_draw_slider(mix_scroller);

      // Update speed
      speed_scroller->Position=Main.backups->Pages->Gradients->Range[Current_gradient].Speed;
      Window_draw_slider(speed_scroller);
      Print_color_cycling_details(113 + 14);

      // Gradient #
      gradient_scroller->Position=Current_gradient;
      Window_draw_slider(gradient_scroller);

      // Technique (flat, dithered, very dithered)
      Draw_button_gradient_style(8,90,Main.backups->Pages->Gradients->Range[Current_gradient].Technique);

      // Rectangular gradient preview
      Draw_gradient_preview(8,128+12,108,14,Current_gradient);

      Display_cursor();
    }

    clicked_button=Window_clicked_button();
    if (Input_sticky_control!=8 || !Mouse_K)
    {
      Allow_colorcycling=0;
      // Restore palette
      Set_palette(Main.palette);
    }

    switch(clicked_button)
    {
      case -1 :
      case  1 : // Palette
        if ( (Mouse_X!=old_mouse_x) || (Mouse_Y!=old_mouse_y) || (Mouse_K!=old_mouse_k) )
        {
          Hide_cursor();
          temp_color=(clicked_button==1) ? Window_attribute2 : Read_pixel(Mouse_X,Mouse_Y);

          if (!old_mouse_k)
          {
            // On vient de clicker

            // On met à jour l'intervalle du dégradé
            first_color=last_color=Main.backups->Pages->Gradients->Range[Current_gradient].Start=Main.backups->Pages->Gradients->Range[Current_gradient].End=temp_color;
            // On tagge le bloc
            Tag_color_gradient_range(&Main.backups->Pages->Gradients->Range[Current_gradient]);
            // Tracé de la preview:
            Draw_gradient_preview(8,128+12,108,14,Current_gradient);
          }
          else
          {
            // On maintient le click, on va donc tester si le curseur bouge
            if (temp_color!=last_color)
            {
              // On commence par ordonner la 1ère et dernière couleur du bloc
              if (first_color<temp_color)
              {
                Main.backups->Pages->Gradients->Range[Current_gradient].Start=first_color;
                Main.backups->Pages->Gradients->Range[Current_gradient].End  =temp_color;
              }
              else if (first_color>temp_color)
              {
                Main.backups->Pages->Gradients->Range[Current_gradient].Start=temp_color;
                Main.backups->Pages->Gradients->Range[Current_gradient].End  =first_color;
              }
              else
                Main.backups->Pages->Gradients->Range[Current_gradient].Start=Main.backups->Pages->Gradients->Range[Current_gradient].End=first_color;
              // On tagge le bloc
              Tag_color_gradient_range(&Main.backups->Pages->Gradients->Range[Current_gradient]);
              // Tracé de la preview:
              Draw_gradient_preview(8,128+12,108,14,Current_gradient);
              last_color=temp_color;
            }
          }
          Display_cursor();
        }
        break;
      case  2 : // Nouvel indice de dégradé
        // Nouvel indice dans Window_attribute2
        Current_gradient=Window_attribute2;
        changed_gradient_index=1;
        break;
      case  3 : // Nouveau mélange de dégradé
        Hide_cursor();
        // Nouvel mélange dans Window_attribute2
        Main.backups->Pages->Gradients->Range[Current_gradient].Mix=Window_attribute2;
        // On affiche la nouvelle preview
        Draw_gradient_preview(8,128+12,108,14,Current_gradient);
        Display_cursor();
        break;
      case  4 : // Changement de sens
        Hide_cursor();
        // On inverse le sens (par un XOR de 1)
        Main.backups->Pages->Gradients->Range[Current_gradient].Inverse^=1;
        Print_in_window(12,23,(Main.backups->Pages->Gradients->Range[Current_gradient].Inverse)?"\033":"\032",MC_Black,MC_Light);
        // On affiche la nouvelle preview
        Draw_gradient_preview(8,128+12,108,14,Current_gradient);
        Display_cursor();
        break;
      case  5 : // Changement de technique
        Hide_cursor();
        // On change la technique par (+1)%3
        Main.backups->Pages->Gradients->Range[Current_gradient].Technique=(Main.backups->Pages->Gradients->Range[Current_gradient].Technique+1)%3;
        Draw_button_gradient_style(8,90,Main.backups->Pages->Gradients->Range[Current_gradient].Technique);
        // On affiche la nouvelle preview
        Draw_gradient_preview(8,128+12,108,14,Current_gradient);
        Display_cursor();
        break;
      case  8 : // Speed
        Main.backups->Pages->Gradients->Range[Current_gradient].Speed=Window_attribute2;
        Hide_cursor();
        Print_color_cycling_details(113 + 14);
        Display_cursor();
        Allow_colorcycling=1;
        break;
      case 9: // Cycling on/off
        cycling_mode = !cycling_mode;
        Hide_cursor();
        Print_in_window(11,112,"Cycling",cycling_mode?MC_Black:MC_Dark,MC_Light);
        Display_cursor();
        break;
    }

    if (!Mouse_K)
    switch (Key)
    {
      case KEY_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case KEY_COMMA :
        Get_color_behind_window(&color,&click);
        if (click)
        {
          Hide_cursor();
          temp_color=color;

          // On met à jour l'intervalle du dégradé
          first_color=last_color=Main.backups->Pages->Gradients->Range[Current_gradient].Start=Main.backups->Pages->Gradients->Range[Current_gradient].End=temp_color;
          // On tagge le bloc
          Tag_color_gradient_range(&Main.backups->Pages->Gradients->Range[Current_gradient]);
          // Tracé de la preview:
          Draw_gradient_preview(8,128+12,108,14,Current_gradient);
          Display_cursor();
          Wait_end_of_click();
        }
        Key=0;
        break;
      case KEY_MOUSEWHEELUP:
        if (Current_gradient>0)
        {
          Current_gradient--;
          changed_gradient_index=1;
        }
        break;
      case KEY_MOUSEWHEELDOWN:
        if (Current_gradient<15)
        {
          Current_gradient++;
          changed_gradient_index=1;
        }
        break;

      default:
        if (Is_shortcut(Key,0x100+BUTTON_HELP))
        {
          Window_help(BUTTON_GRADRECT, NULL);
          Key=0;
          break;
        }
        else if (Is_shortcut(Key,0x200+BUTTON_GRADRECT))
          clicked_button=6;
        else if (Is_shortcut(Key,SPECIAL_CYCLE_MODE))
        {
          // Cycling on/off
          cycling_mode = !cycling_mode;
          Hide_cursor();
          Print_in_window(11,112,"Cycling",cycling_mode?MC_Black:MC_Dark,MC_Light);
          Display_cursor();
        }
    }
  }
  while (clicked_button!=6 && clicked_button!=7 && !Quit_is_required);

  Close_window();
  // The Grad rect operation uses the same button as Grad menu.
  if (Current_operation != OPERATION_GRAD_RECTANGLE)
    Unselect_button(BUTTON_GRADRECT);

  Display_cursor();

  Gradient_pixel=Display_pixel;
  Cycling_mode=cycling_mode;
  if (clicked_button==7) // Cancel
  {
    Current_gradient=old_current_gradient;
    memcpy(Main.backups->Pages->Gradients,&backup_gradients,sizeof(T_Gradient_array));
  }
}


// -- Gestion des boutons de cercle / ellipse / rectangle dégradés --------------------

void Button_Grad_rectangle(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_GRAD_RECTANGLE);
  Display_cursor();
}


// -- Gestion du bouton de remplissage ---------------------------------------

void Button_Fill(int btn)
{
  (void)btn;
  if (Current_operation!=OPERATION_FILL)
  {
    Hide_cursor();

    if (Current_operation!=OPERATION_REPLACE)
    {
      Paintbrush_shape_before_fill=Paintbrush_shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
    }
    else
      if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
           ( (!Main.magnifier_mode) || (Mouse_X<Main.separator_position) || (Mouse_X>=Main.X_zoom) ) )
        Print_in_menu("X:       Y:             ",0);
    Start_operation_stack(OPERATION_FILL);
    Display_cursor();
  }
}


void Button_Replace(int btn)
{
  (void)btn;
  if (Current_operation!=OPERATION_REPLACE)
  {
    Hide_cursor();
    if (Current_operation!=OPERATION_FILL)
    {
      Paintbrush_shape_before_fill=Paintbrush_shape;
      Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
    }
    if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
         ( (!Main.magnifier_mode) || (Mouse_X<Main.separator_position) || (Mouse_X>=Main.X_zoom) ) )
      Print_in_menu("X:       Y:       (    )",0);
    Start_operation_stack(OPERATION_REPLACE);
    Display_cursor();
  }
}


void Button_Unselect_fill(int btn)
{
  (void)btn;
  Paintbrush_shape=Paintbrush_shape_before_fill;

  if (Current_operation==OPERATION_REPLACE)
    if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
         ( (!Main.magnifier_mode) || (Mouse_X<Main.separator_position) || (Mouse_X>=Main.X_zoom) ) )
      Print_in_menu("X:       Y:             ",0);
}


//---------------------------- Menu des pinceaux -----------------------------

/// Checks if the current brush is identical to a preset one.
byte Same_paintbrush(byte index)
{
  if (Paintbrush_shape!=Paintbrush[index].Shape ||
      Paintbrush_width!=Paintbrush[index].Width ||
      Paintbrush_height!=Paintbrush[index].Height)
  return 0;

  if (Paintbrush_shape==PAINTBRUSH_SHAPE_MISC)
  {
    // Check all pixels
    int x,y;
    for(y=0;y<Paintbrush_height;y++)
      for(x=0;x<Paintbrush_width;x++)
        if(Paintbrush_sprite[(y*MAX_PAINTBRUSH_SIZE)+x]!=Paintbrush[index].Sprite[y][x])
          return 0;
  }
  return 1;
}

void Button_Paintbrush_menu(int btn)
{
  short clicked_button;
  short x_pos,y_pos;
  byte index;

  Open_window(310,180,"Paintbrush menu");

  Window_display_frame(8,21,294,132);

  Window_set_normal_button(10,158,67,14,"Cancel",0,1,KEY_ESC); // 1

  Window_set_dropdown_button(216, 158, 84,14,84,"Preset...", 0,0,1,RIGHT_SIDE|LEFT_SIDE,1);
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_ROUND,         "Round");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_SQUARE,        "Square");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_HORIZONTAL_BAR,"Horizontal");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_VERTICAL_BAR,  "Vertical");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_SLASH,         "Slash");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_ANTISLASH,     "Antislash");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_RANDOM,        "Random");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_CROSS,         "Cross");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_PLUS,          "Plus");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_DIAMOND,       "Diamond");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_SIEVE_ROUND,   "Sieve Rnd");
  Window_dropdown_add_item(Window_dropdown_button_list,PAINTBRUSH_SHAPE_SIEVE_SQUARE,  "Sieve Sqr");

  for (index=0; index<NB_PAINTBRUSH_SPRITES; index++)
  {
    x_pos=13+(index%12)*24;
    y_pos=27+(index/12)*25;
    //Window_set_normal_button(x_pos  ,y_pos  ,20,20,"",0,1,KEY_NONE);
    Window_set_dropdown_button(x_pos  ,y_pos  ,20,20,28,NULL, 0,0,0,RIGHT_SIDE,0);
    Window_dropdown_add_item(Window_dropdown_button_list, 1, "Set");
    // Highlight selected brush
    if (Same_paintbrush(index))
      Window_rectangle(x_pos,y_pos,20,20,MC_White);

    Display_paintbrush_in_window(x_pos+2,y_pos+2,index);
  }
  for (index=0; index<BRUSH_CONTAINER_COLUMNS*BRUSH_CONTAINER_ROWS; index++)
  {
    x_pos=13+((index+NB_PAINTBRUSH_SPRITES)%12)*24;
    y_pos=27+((index+NB_PAINTBRUSH_SPRITES)/12)*25;
    Window_set_dropdown_button(x_pos  ,y_pos  ,20,20,28,NULL, 0,0,0,RIGHT_SIDE,0);
    Window_dropdown_add_item(Window_dropdown_button_list, 1, "Set");
    Display_stored_brush_in_window(x_pos+2, y_pos+2, index);
  }

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
      Window_help(BUTTON_PAINTBRUSHES, NULL);
    // Brush container
    if (clicked_button>=(NB_PAINTBRUSH_SPRITES+3))
    {
      index = clicked_button-NB_PAINTBRUSH_SPRITES-3;

      if (Window_attribute2==1) // Set
      {
        // Store

        x_pos=13+((index+NB_PAINTBRUSH_SPRITES)%12)*24;
        y_pos=27+((index+NB_PAINTBRUSH_SPRITES)/12)*25;

        Store_brush(index);
        Hide_cursor();
        Display_stored_brush_in_window(x_pos+2, y_pos+2, index);
        Display_cursor();
      }
      else
      {
        // Restore and exit

        if (Restore_brush(index))
        {
          Close_window();
          break;
        }
      }

    }
    else if (clicked_button>=3)
    // Standard paintbrushes
    {
      if (Window_attribute2!=1)
      {
        // Select paintbrush
        Close_window();
        Select_paintbrush(clicked_button-3);
        break;
      }
      else if (Window_attribute2==1)
      {
        // Store current
        index=clicked_button-3;
        if (!Store_paintbrush(index))
        {
          // Redraw
          Hide_cursor();
          x_pos=13+(index%12)*24;
          y_pos=27+(index/12)*25;
          Window_rectangle(x_pos,y_pos,20,20,MC_White);
          Display_paintbrush_in_window(x_pos+2,y_pos+2,index);
          Display_cursor();
        }
      }
    }
    else if (clicked_button==1 || Is_shortcut(Key,0x100+BUTTON_PAINTBRUSHES))
    {
      Close_window();
      break;
    }
    else if (clicked_button==2)
    {
      int size;
      // Pick a standard shape
      Paintbrush_shape=Window_attribute2;
      // Assign a reasonable size
      size=Max(Paintbrush_width,Paintbrush_height);
      if (size==1)
        size=3;

      switch (Paintbrush_shape)
      {
        case PAINTBRUSH_SHAPE_HORIZONTAL_BAR:
          Set_paintbrush_size(size, 1);
          break;
        case PAINTBRUSH_SHAPE_VERTICAL_BAR:
            Set_paintbrush_size(1, size);
          break;
        case PAINTBRUSH_SHAPE_CROSS:
        case PAINTBRUSH_SHAPE_PLUS:
        case PAINTBRUSH_SHAPE_DIAMOND:
          Set_paintbrush_size(size|1,size|1);
          break;
        default:
          Set_paintbrush_size(size,size);
          break;

      }
      Close_window();
      Change_paintbrush_shape(Paintbrush_shape);
      break;
    }
  }
  while (1);

  Unselect_button(btn);
  Display_cursor();
}


void Button_Brush_monochrome(int btn)
{
  Hide_cursor();
  // On passe en brosse monochrome:
  Change_paintbrush_shape(PAINTBRUSH_SHAPE_MONO_BRUSH);

  Unselect_button(btn);

  Display_cursor();
}

// -- Fonction renvoyant le mode vidéo le plus adapté à l'image chargée -----
#define TOLERANCE_X 8
#define TOLERANCE_Y 4
static int Best_video_mode(void)
{
  short best_width,best_height;
  int best_mode;
  short temp_x,temp_y;
  int mode;

  // Si mode fenêtre, on reste dans ce mode.
  if (Current_resolution == 0)
    return 0;

  // On commence par borner les dimensions, ou du moins les rendre cohérentes
  if ((Original_screen_X<=0) || (Config.Set_resolution_according_to==2))
    Original_screen_X=Main.image_width;
  else
    if (Original_screen_X<320)
      Original_screen_X=320;

  if ((Original_screen_Y<=0) || (Config.Set_resolution_according_to==2))
    Original_screen_Y=Main.image_height;
  else
    if (Original_screen_Y<200)
      Original_screen_Y=200;

  GFX2_Log(GFX2_DEBUG, "Best_video_mode() looking for %dx%d\n", (int)Original_screen_X, (int)Original_screen_Y);

  if ((Original_screen_X > Video_mode[Nb_video_modes-1].Width) ||
      (Original_screen_Y > Video_mode[Nb_video_modes-1].Height))
  {
    // return the "biggest" video mode
    return Nb_video_modes-1;
  }

  // Maintenant on peut chercher le mode qui correspond le mieux
  best_mode=Current_resolution;
  best_width=0;
  best_height=0;


  for (mode=1; mode<Nb_video_modes; mode++)
  {
    if (Video_mode[mode].Fullscreen && (Video_mode[mode].State&3)<2)
    {
      temp_x=Video_mode[mode].Width;
      temp_y=Video_mode[mode].Height;

      if ( (Original_screen_X-TOLERANCE_X<=temp_x)
        && (Original_screen_Y-TOLERANCE_Y<=temp_y) )
        return mode;
      else
      {
        if ( (best_width<=temp_x)
          && (best_height<=temp_y)
          && (temp_x-TOLERANCE_X<=Original_screen_X)
          && (temp_y-TOLERANCE_Y<=Original_screen_Y) )
        {
          best_width=temp_x;
          best_height=temp_y;
          best_mode=mode;
        }
      }
    }
  }

  return best_mode;
}

void Load_picture(enum CONTEXT_TYPE type)
{
  byte  confirm;
  byte  old_cursor_shape;
  int   new_mode;
  T_IO_Context context;
  word * filename_unicode = NULL;
  T_Selector_settings * selector;

  switch (type)
  {
  case CONTEXT_MAIN_IMAGE:
    filename_unicode = Main.backups->Pages->Filename_unicode;
    Init_context_layered_image(&context, Main.backups->Pages->Filename, Main.backups->Pages->File_directory);
    selector = &Main.selector;
    break;
  case CONTEXT_BRUSH:
    filename_unicode = Brush_filename_unicode;
    Init_context_brush(&context, Brush_filename, Brush_file_directory);
    selector = &Brush_selector;
    break;
  case CONTEXT_PALETTE:
    Init_context_layered_image(&context, "", Main.backups->Pages->File_directory);
    context.Type = CONTEXT_PALETTE;
    context.Format = FORMAT_PAL;
    selector = &Palette_selector;
    break;
  default:
    return; // DO NOTHING
  }
  context.File_name_unicode = Unicode_strdup(filename_unicode);
  confirm = Button_Load_or_Save(selector, 1, &context);

  if (confirm)
  {
    if (type==CONTEXT_MAIN_IMAGE)
    {
      if (Main.image_is_modified)
        confirm=Confirmation_box("Discard unsaved changes?");
    }
  }

  // confirm is modified inside the first if, that's why we check it
  // again here
  if (confirm)
  {
    short old_image_width, old_image_height;

    old_image_width = Main.image_width;
    old_image_height = Main.image_height;
    old_cursor_shape=Cursor_shape;
    Hide_cursor();
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    if (type==CONTEXT_MAIN_IMAGE)
    {
      Original_screen_X=0;
      Original_screen_Y=0;
    }

    Load_image(&context);

    if (type==CONTEXT_BRUSH)
    {
      free(Brush_filename);
      Brush_filename = context.File_name; // "steal" heap string
      context.File_name = NULL;
      free(Brush_filename_unicode);
      Brush_filename_unicode = context.File_name_unicode; // "steal" heap string
      context.File_name_unicode = NULL;
      free(Brush_file_directory);
      Brush_file_directory = context.File_directory;  // "steal" heap string
      context.File_directory = NULL;
      Brush_fileformat = context.Format;

      Tiling_offset_X=0;
      Tiling_offset_Y=0;

      Brush_offset_X=(Brush_width>>1);
      Brush_offset_Y=(Brush_height>>1);

      Select_button(BUTTON_DRAW,LEFT_SIDE);
      if (Config.Auto_discontinuous)
      {
        // On se place en mode Dessin discontinu à la main
        while (Current_operation!=OPERATION_DISCONTINUOUS_DRAW)
          Select_button(BUTTON_DRAW,RIGHT_SIDE);
      }
      Hide_cursor();
      // On passe en brosse couleur:
      Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
    }
    else
    {
      Hide_cursor();
      Cursor_shape=old_cursor_shape;
    }


    if ( (File_error==1) || (Get_fileformat(Main.fileformat)->Palette_only) )
    {
      if (File_error!=1)
        Compute_optimal_menu_colors(Main.palette);
    }
    else
    {
      if (type==CONTEXT_MAIN_IMAGE)
      {
        if (Main.magnifier_mode &&
            (Main.image_width > old_image_width || Main.image_height > old_image_height))
        {
          // disable magnifier
          Pixel_preview = Pixel_preview_normal;
          Main.magnifier_mode = 0;
          Draw_menu_button(BUTTON_MAGNIFIER,Main.magnifier_mode);
        }

        new_mode=Best_video_mode();
        if ((Config.Auto_set_res) && (new_mode!=Current_resolution))
        {
          Init_mode_video(
            Video_mode[new_mode].Width,
            Video_mode[new_mode].Height,
            Video_mode[new_mode].Fullscreen,
            Pixel_ratio);
          Display_menu();
        }
        // In window mode, activate wide or tall pixels if the image says so.
        else if (!Video_mode[Current_resolution].Fullscreen &&
          ((context.Ratio == PIXEL_WIDE &&
            Pixel_ratio != PIXEL_WIDE && Pixel_ratio != PIXEL_WIDE2) ||
            (context.Ratio == PIXEL_TALL &&
            Pixel_ratio != PIXEL_TALL && Pixel_ratio != PIXEL_TALL2) ||
            (context.Ratio == PIXEL_TALL3 &&
            Pixel_ratio != PIXEL_TALL3) ))
        {
          Init_mode_video(
            Video_mode[Current_resolution].Width,
            Video_mode[Current_resolution].Height,
            Video_mode[Current_resolution].Fullscreen,
            context.Ratio);
            Display_menu();
        }
        else
        {
          if (Main.image_width > old_image_width || Main.image_height > old_image_height)
          {
            Main.offset_X = 0;
            Main.offset_Y = 0;
          }
          Compute_limits();
          Compute_paintbrush_coordinates();
        }

        Compute_optimal_menu_colors(Main.palette);
        Redraw_layered_image();
        End_of_modification();
        Check_menu_mode();
        Display_all_screen();
        Main.image_is_modified=0;
      }
    }

    Display_menu();
    Display_cursor();
  }

  Destroy_context(&context);

  Hide_cursor();
  Print_filename();
  Display_cursor();
  Set_palette(Main.palette);
}


void Button_Load(int btn)
{
  (void)btn;
  // On sauve l'état actuel des paramètres de l'image pour pouvoir les
  // restituer en cas d'erreur n'affectant pas l'image
  Upload_infos_page(&Main);

  Load_picture(CONTEXT_MAIN_IMAGE);
  Tilemap_update();
}


void Button_Reload(int btn)
{
  byte old_cursor_shape;
  int  new_mode;

  (void)btn;
  // On sauve l'état actuel des paramètres de l'image pour pouvoir les
  // restituer en cas d'erreur n'affectant pas l'image
  Upload_infos_page(&Main);

  if ( (!Main.image_is_modified) || Confirmation_box("Discard unsaved changes ?") )
  {
    T_IO_Context context;

    Hide_cursor();
    old_cursor_shape=Cursor_shape;
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    Original_screen_X=0;
    Original_screen_Y=0;

    Init_context_layered_image(&context, Main.backups->Pages->Filename, Main.backups->Pages->File_directory);
    Load_image(&context);

    Hide_cursor();
    Cursor_shape=old_cursor_shape;

    if (File_error!=1)
    {
      if (Main.magnifier_mode)
      {
        Pixel_preview=Pixel_preview_normal;
        Main.magnifier_mode=0;
        Draw_menu_button(BUTTON_MAGNIFIER,Main.magnifier_mode);
      }

      new_mode=Best_video_mode();
      if ( ((Config.Auto_set_res) && (new_mode!=Current_resolution)) &&
           (!Resolution_in_command_line) )
      {
        Init_mode_video(
        Video_mode[new_mode].Width,
        Video_mode[new_mode].Height,
        Video_mode[new_mode].Fullscreen,
        Pixel_ratio);
        Display_menu();
      }
      // In window mode, activate wide or tall pixels if the image says so.
      else if (!Video_mode[Current_resolution].Fullscreen &&
        ((context.Ratio == PIXEL_WIDE &&
          Pixel_ratio != PIXEL_WIDE && Pixel_ratio != PIXEL_WIDE2) ||
          (context.Ratio == PIXEL_TALL &&
          Pixel_ratio != PIXEL_TALL && Pixel_ratio != PIXEL_TALL2 &&
          Pixel_ratio != PIXEL_TALL3)))
      {
        Init_mode_video(
          Video_mode[Current_resolution].Width,
          Video_mode[Current_resolution].Height,
          Video_mode[Current_resolution].Fullscreen,
          context.Ratio);
          Display_menu();
      }
      else
      {
        Main.offset_X=0;
        Main.offset_Y=0;
        Compute_limits();
        Compute_paintbrush_coordinates();
      }
      Tilemap_update();
      Redraw_layered_image();
      End_of_modification();
      Check_menu_mode();
      Display_all_screen();

      Main.image_is_modified=0;
    }
    Destroy_context(&context);
  }
  else
    Hide_cursor();

  Compute_optimal_menu_colors(Main.palette);
  Display_menu();
  if (Config.Display_image_limits)
    Display_image_limits();

  Unselect_button(BUTTON_LOAD);

  Display_cursor();
}


static void Backup_existing_file(const char * filename)
{
  char * new_filename; // full filename of the backup file
  char * p;
  int i;
  size_t len;

  if (filename == NULL)
    return;
  len = strlen(filename);
  new_filename = malloc(len + 1 + 4); // spare bytes to concat ".BAK"
  memcpy(new_filename, filename, len + 1);
  p = Find_last_separator(new_filename);  // pointer to the filename part (after directory)
  if (p == NULL)
    p = new_filename;
  else
    p++;
  i = Position_last_dot(p);
  if (i >= 0)
    memcpy(p + i + 1, "BAK", 4);
  else
    memcpy(new_filename + len, ".BAK", 5);

  File_error=0;

  // On fait un backup si le nom du fichier n'est pas celui qu'on a choisi
  // pour nommer les backups (c'est évident!).
  if (strcmp(new_filename,filename))
  {
    // S'il y avait déjà un fichier Backup, on l'efface
    if ((File_exists(new_filename))
     && (Remove_path(new_filename)!=0))
      File_error=1;

    if ((!File_error)
     && (rename(filename,new_filename)!=0))
      File_error=1;
  }
  free(new_filename);
}


void Save_picture(enum CONTEXT_TYPE type)
{
  byte  confirm;
  byte  old_cursor_shape;
  T_IO_Context save_context;
  word * filename_unicode = NULL;
  T_Selector_settings * selector;

  if (type == CONTEXT_MAIN_IMAGE)
  {
    filename_unicode = Main.backups->Pages->Filename_unicode;
    Init_context_layered_image(&save_context, Main.backups->Pages->Filename, Main.backups->Pages->File_directory);
    save_context.Format = Main.fileformat;
    selector = &Main.selector;
  }
  else if (type == CONTEXT_BRUSH)
  {
    filename_unicode = Brush_filename_unicode;
    Init_context_brush(&save_context, Brush_filename, Brush_file_directory);
    save_context.Format = Brush_fileformat;
    selector = &Brush_selector;
  }
  else if (type == CONTEXT_PALETTE)
  {
    char* pal_filename;
    char* dotpos;
    size_t len = strlen(Main.backups->Pages->Filename);

    pal_filename = malloc(len + 4 + 1); // reserve space for ".pal"
    memcpy(pal_filename, Main.backups->Pages->Filename, len + 1);
    // Replace extension with PAL
    dotpos = strrchr(pal_filename, '.');
    if (dotpos == NULL)
      dotpos = pal_filename + len;
    strcpy(dotpos, ".pal");

    Init_context_layered_image(&save_context, pal_filename, Main.backups->Pages->File_directory);
    save_context.Type = CONTEXT_PALETTE;

    free(pal_filename);

    // Set format to PAL
    save_context.Format = FORMAT_PAL;
    selector = &Palette_selector;
  }
  else
    return;

  save_context.File_name_unicode = Unicode_strdup(filename_unicode);
  confirm=Button_Load_or_Save(selector, 0, &save_context);

  if (confirm && File_exists(save_context.File_name))
  {
    confirm=Confirmation_box("Erase old file ?");
    if (confirm && (Config.Backup))
    {
      char * full_filename = Filepath_append_to_dir(save_context.File_directory, save_context.File_name);
      Backup_existing_file(full_filename);
      free(full_filename);
      if (File_error)
      {
        confirm=0;
        Error(0);
      }
    }
  }

  if (confirm)
  {
    const T_Format * format;

    old_cursor_shape=Cursor_shape;
    Hide_cursor();
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    Save_image(&save_context);

    format=Get_fileformat(save_context.Format);
    if (!File_error && type == CONTEXT_MAIN_IMAGE && !format->Palette_only && (Main.backups->Pages->Nb_layers==1 || format->Supports_layers))
    {
      Main.image_is_modified = 0;
      Main.fileformat = save_context.Format;
      free(Main.backups->Pages->Filename);
      Main.backups->Pages->Filename = save_context.File_name; // "steal" string from heap
      save_context.File_name = NULL;
      free(Main.backups->Pages->Filename_unicode);
      Main.backups->Pages->Filename_unicode = save_context.File_name_unicode; // "steal" string
      save_context.File_name_unicode = NULL;
      free(Main.backups->Pages->File_directory);
      Main.backups->Pages->File_directory = save_context.File_directory;
      save_context.File_directory = NULL;
    }
    if (type == CONTEXT_BRUSH)
    {
      Brush_fileformat = save_context.Format;
      free(Brush_filename);
      Brush_filename = save_context.File_name;
      save_context.File_name = NULL;
      free(Brush_filename_unicode);
      Brush_filename_unicode = save_context.File_name_unicode;
      save_context.File_name_unicode = NULL;
      free(Brush_file_directory);
      Brush_file_directory = save_context.File_directory;
      save_context.File_directory = NULL;
    }
    Hide_cursor();
    Cursor_shape=old_cursor_shape;
    Display_cursor();
  }
  Destroy_context(&save_context);

  Print_filename();
  Set_palette(Main.palette);
}


void Button_Save(int btn)
{
  (void)btn;
  Save_picture(CONTEXT_MAIN_IMAGE);
}

/// Save main image over existing file (no fileselector)
void Button_Autosave(int btn)
{
  byte old_cursor_shape;
  char * filename;
  byte file_already_exists;

  (void)btn;
  filename = Filepath_append_to_dir(Main.backups->Pages->File_directory, Main.backups->Pages->Filename);
  file_already_exists = File_exists(filename);

  if ( (!file_already_exists) || Confirmation_box("Erase old file ?") )
  {
    if ((file_already_exists) && (Config.Backup))
      Backup_existing_file(filename);
    else
      File_error=0;

    Hide_cursor();

    if (!File_error)
    {
      T_IO_Context save_context;

      old_cursor_shape=Cursor_shape;
      Cursor_shape=CURSOR_SHAPE_HOURGLASS;
      Display_cursor();

      Init_context_layered_image(&save_context, Main.backups->Pages->Filename, Main.backups->Pages->File_directory);
      save_context.File_name_unicode = Unicode_strdup(Main.backups->Pages->Filename_unicode);
      Save_image(&save_context);
      if (!File_error)
      {
        Main.image_is_modified=0;
      }
      Destroy_context(&save_context);

      Hide_cursor();
      Cursor_shape=old_cursor_shape;
    }
    else
      Error(0);
  }
  else
    Hide_cursor();

  free(filename);
  Unselect_button(BUTTON_SAVE);

  Display_cursor();
}


// -- Gestion des boutons de ligne ------------------------------------------

void Button_Lines(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(Selected_line_mode);
  Display_cursor();
}


void Button_Lines_switch_mode(int btn)
{
  char icon;

  if (Selected_line_mode==OPERATION_LINE)
  {
    Selected_line_mode=OPERATION_K_LINE;
    icon=MENU_SPRITE_K_LINE;
  }
  else if (Selected_line_mode==OPERATION_K_LINE)
  {
    Selected_line_mode=OPERATION_CENTERED_LINES;
    icon=MENU_SPRITE_CENTERED_LINES;
  }
  else
  {
    Selected_line_mode=OPERATION_LINE;
    icon=-1;
  }

  Hide_cursor();
  Display_sprite_in_menu(btn,icon);
  Draw_menu_button(btn,BUTTON_PRESSED);
  Start_operation_stack(Selected_line_mode);
  Display_cursor();
}


// -- Button de brosse ------------------------------------------------------

void Button_Brush(int btn)
{
  Hide_cursor();

  if (Current_operation!=OPERATION_GRAB_BRUSH)
    Start_operation_stack(OPERATION_GRAB_BRUSH);
  else
    Unselect_button(btn);

  Display_cursor();
}


void Button_Unselect_brush(int btn)
{
  (void)btn;
  // On fait de notre mieux pour restaurer l'ancienne opération:
  Start_operation_stack(Operation_before_interrupt);
}


void Button_Restore_brush(int btn)
{
  Hide_cursor();
  // On passe en brosse couleur:
  Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);

  Unselect_button(btn);

  Display_cursor();
}


// -- Button de prise de brosse au lasso ------------------------------------

void Button_Lasso(int btn)
{
  Hide_cursor();

  if (Current_operation!=OPERATION_POLYBRUSH)
  {
    Paintbrush_shape_before_lasso=Paintbrush_shape;
    Paintbrush_shape=PAINTBRUSH_SHAPE_POINT;
    Start_operation_stack(OPERATION_POLYBRUSH);
  }
  else
    Unselect_button(btn);

  Display_cursor();
}


void Button_Unselect_lasso(int btn)
{
  (void)btn;

  // If we aren't in OPERATION_POLYBRUSH, then we didn't save a brush shape,
  // and we shouldn't try to restore it.
  if (Current_operation==OPERATION_POLYBRUSH)
    Paintbrush_shape=Paintbrush_shape_before_lasso;

  // On fait de notre mieux pour restaurer l'ancienne opération:
  Start_operation_stack(Operation_before_interrupt);
}


// -- Button de pipette -----------------------------------------------------

void Button_Colorpicker(int btn)
{
  Hide_cursor();

  if (Current_operation!=OPERATION_COLORPICK)
  {
    Colorpicker_color=-1;
    Start_operation_stack(OPERATION_COLORPICK);
    Paintbrush_shape_before_colorpicker=Paintbrush_shape;
    Paintbrush_shape=PAINTBRUSH_SHAPE_NONE;
    if (Operation_before_interrupt!=OPERATION_REPLACE)
      if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
           ( (!Main.magnifier_mode) || (Mouse_X<Main.separator_position) || (Mouse_X>=Main.X_zoom) ) )
        Print_in_menu("X:       Y:       (    )",0);
  }
  else
    Unselect_button(btn);

  Display_cursor();
}


void Button_Unselect_colorpicker(int btn)
{
  (void)btn;
  // Erase the color block which shows the picked color
  if (Operation_before_interrupt!=OPERATION_REPLACE)
    if ( (Mouse_Y<Menu_Y) && (Menu_is_visible) &&
         ( (!Main.magnifier_mode) || (Mouse_X<Main.separator_position) || (Mouse_X>=Main.X_zoom) ) )
      Print_in_menu("X:       Y:             ",0);

  // On fait de notre mieux pour restaurer l'ancienne opération:
  if (Current_operation==OPERATION_COLORPICK)
  {
    Start_operation_stack(Operation_before_interrupt);
    Paintbrush_shape=Paintbrush_shape_before_colorpicker;
  }
}


  // -- Inversion de la couleur Fore et de la couleur Back --
void Button_Invert_foreback(int btn)
{
  byte temp_color;

  temp_color=Fore_color;
  Fore_color        =Back_color;
  Back_color        =temp_color;

  Hide_cursor();
  Frame_menu_color(Back_color);
  Frame_menu_color(Fore_color);
  Reposition_palette();
  Display_foreback();
  Unselect_button(btn);
  Display_cursor();
}


// -- Gestion du bouton Loupe -----------------------------------------------

byte Coming_from_zoom_factor_menu=0;

void Button_Magnify(int btn)
{
  Hide_cursor();
  if ( (Current_operation==OPERATION_MAGNIFY) || (Main.magnifier_mode) )
  {
    Unselect_button(btn);
  }
  else
  {
    Compute_magnifier_data();
    if ((!Config.Fast_zoom) || (Mouse_Y>=Menu_Y) || Coming_from_zoom_factor_menu)
    {
      Coming_from_zoom_factor_menu=0;
      Start_operation_stack(OPERATION_MAGNIFY);
    }
    else
    { /* Ceci est de la duplication de code de presque toute l'opération de */
      /* la loupe... Il serait peut-être plus propre de faire une procédure */
      /* qui s'en charge... */
      // On passe en mode loupe
      Main.magnifier_mode=1;

      // La fonction d'affichage dans la partie image est désormais un affichage
      // spécial loupe.
      Pixel_preview=Pixel_preview_magnifier;

      // On calcule l'origine de la loupe
      Main.magnifier_offset_X=Mouse_X-(Main.magnifier_width>>1);
      Main.magnifier_offset_Y=Mouse_Y-(Main.magnifier_height>>1);

     // Calcul des coordonnées absolues de ce coin DANS L'IMAGE
      Main.magnifier_offset_X+=Main.offset_X;
      Main.magnifier_offset_Y+=Main.offset_Y;

      Clip_magnifier_offsets(&Main.magnifier_offset_X, &Main.magnifier_offset_Y);

      // On calcule les bornes visibles dans l'écran
      Position_screen_according_to_zoom();
      Compute_limits();
      Display_all_screen();

      // Repositionner le curseur en fonction des coordonnées visibles
      Compute_paintbrush_coordinates();
    }
  }
  Display_cursor();
  Update_rect(0,0,0,0);
}

void Button_Magnify_menu(int btn)
{
  T_Dropdown_button dropdown;
  T_Dropdown_choice *item;
  int i;
  static const char * text[NB_ZOOM_FACTORS] =
    {"x2", "x3", "x4", "x5", "x6", "x8", "x10", "x12", "x14", "x16", "x18", "x20",
      "x24", "x28", "x32"};

  (void)btn;
  Hide_cursor();

  dropdown.Pos_X         =Buttons_Pool[BUTTON_MAGNIFIER].X_offset;
  dropdown.Pos_Y         =Buttons_Pool[BUTTON_MAGNIFIER].Y_offset;
  dropdown.Height        =Buttons_Pool[BUTTON_MAGNIFIER].Height;
  dropdown.Dropdown_width=28;
  dropdown.First_item    =NULL;
  dropdown.Bottom_up     =1;

  for(i = 0; i < NB_ZOOM_FACTORS; i++) {
    Window_dropdown_add_item(&dropdown, i, text[i]);
  }

  item=Dropdown_activate(&dropdown,0,Menu_Y);

  if (item)
  {
    Change_magnifier_factor(item->Number,0);
  }

  if ( (!item) && (!Main.magnifier_mode) && (Current_operation!=OPERATION_MAGNIFY) ) // Cancel
    Unselect_button(BUTTON_MAGNIFIER);

  Display_all_screen();
  Display_cursor();
  Update_rect(Main.separator_position,0,Screen_width-Main.separator_position,Menu_Y);

  if ( (item) && (!Main.magnifier_mode) && (Current_operation!=OPERATION_MAGNIFY) ) // Passage en mode zoom
  {
    Coming_from_zoom_factor_menu=1;
    Select_button(BUTTON_MAGNIFIER,LEFT_SIDE);
  }

  Window_dropdown_clear_items(&dropdown);
}

void Button_Unselect_magnifier(int btn)
{
  (void)btn;
  if (Main.magnifier_mode)
  {
    // On sort du mode loupe
    Main.magnifier_mode=0;


    // --> Recalculer le décalage de l'écran lorsqu'on sort de la loupe <--
    // Centrage "brut" de lécran par rapport à la loupe
    Main.offset_X=Main.magnifier_offset_X-((Screen_width-Main.magnifier_width)>>1);
    Main.offset_Y=Main.magnifier_offset_Y-((Menu_Y-Main.magnifier_height)>>1);

    // Correction en cas de débordement de l'image
    if (Main.offset_X+Screen_width>Main.image_width)
      Main.offset_X=Main.image_width-Screen_width;
    if (Main.offset_X<0)
      Main.offset_X=0;

    if (Main.offset_Y+Menu_Y>Main.image_height)
      Main.offset_Y=Main.image_height-Menu_Y;
    if (Main.offset_Y<0)
      Main.offset_Y=0;

    // La fonction d'affichage dans l'image est désormais un affichage normal.
    Pixel_preview=Pixel_preview_normal;

    // Calculer les bornes visibles dans l'écran
    Compute_limits();
    Display_all_screen();  // <=> Display_screen();
    // Repositionner le curseur en fonction des coordonnées visibles
    Compute_paintbrush_coordinates();

    Old_MX = -1;
    Old_MY = -1;
  }
  else // On fait de notre mieux pour restaurer l'ancienne opération:
    Start_operation_stack(Operation_before_interrupt);
}


// ----------------------- Modifications de brosse ---------------------------

void Button_Brush_FX(int btn)
{
  short clicked_button;
  short index;

  Open_window(310,162,"Brush effects");

  Window_display_frame(  6,19,298,61);
  Window_display_frame(  6,83,122,53);
  Window_display_frame(137,83,167,53);

  Window_set_normal_button(236,141, 67,14,"Cancel"          ,0,1,KEY_ESC); // 1
  Window_set_normal_button( 19, 46, 27,14,"X\035"           ,0,1,Config_Key[SPECIAL_FLIP_X][0]); // 2
  Window_set_normal_button( 19, 61, 27,14,"Y\022"           ,0,1,Config_Key[SPECIAL_FLIP_Y][0]); // 3
  Window_set_normal_button( 58, 46, 37,14,"90\xb0"             ,0,1,Config_Key[SPECIAL_ROTATE_90][0]); // 4
  Window_set_normal_button( 96, 46, 37,14,"180\xb0"            ,0,1,Config_Key[SPECIAL_ROTATE_180][0]); // 5
  Window_set_normal_button( 58, 61, 75,14,"any angle"       ,0,1,Config_Key[SPECIAL_ROTATE_ANY_ANGLE][0]); // 6
  Window_set_normal_button(145, 46, 67,14,"Stretch"         ,0,1,Config_Key[SPECIAL_STRETCH][0]); // 7
  Window_set_normal_button(145, 61, 67,14,"Distort"         ,0,1,Config_Key[SPECIAL_DISTORT][0]); // 8
  Window_set_normal_button(155, 99,131,14,"Recolorize"      ,0,1,Config_Key[SPECIAL_RECOLORIZE_BRUSH][0]); // 9
  Window_set_normal_button(155,117,131,14,"Get brush colors",0,1,Config_Key[SPECIAL_GET_BRUSH_COLORS][0]); // 10

  // Boutons représentant les coins du brush handle: (HG,HD,C,BG,BD)
  Window_set_normal_button( 75, 90,11,11,"",0,1,Config_Key[SPECIAL_TOP_LEFT_ATTACHMENT][0]); // 11
  Window_set_normal_button(103, 90,11,11,"",0,1,Config_Key[SPECIAL_TOP_RIGHT_ATTACHMENT][0]); // 12
  Window_set_normal_button( 89,104,11,11,"",0,1,Config_Key[SPECIAL_CENTER_ATTACHMENT][0]); // 13
  Window_set_normal_button( 75,118,11,11,"",0,1,Config_Key[SPECIAL_BOTTOM_LEFT_ATTACHMENT][0]); // 14
  Window_set_normal_button(103,118,11,11,"",0,1,Config_Key[SPECIAL_BOTTOM_RIGHT_ATTACHMENT][0]); // 15

  Window_set_normal_button(224,46,67,14,"Outline",0,1,Config_Key[SPECIAL_OUTLINE][0]); // 16
  Window_set_normal_button(224,61,67,14,"Nibble" ,0,1,Config_Key[SPECIAL_NIBBLE][0]); // 17

  Window_set_normal_button(  7,141, 60,14,"Load",0,1,Config_Key[SPECIAL_LOAD_BRUSH][0]); // 18
  Window_set_normal_button( 70,141, 60,14,"Save",0,1,Config_Key[SPECIAL_SAVE_BRUSH][0]); // 19

  Print_in_window( 80, 24,"Shape modifications",MC_Dark,MC_Light);
  Print_in_window( 10, 36,"Mirror",MC_Dark,MC_Light);
  Print_in_window( 72, 36,"Rotate",MC_Dark,MC_Light);
  Print_in_window(155, 36,"Deform",MC_Dark,MC_Light);
  Print_in_window(230, 36,"Borders",MC_Dark,MC_Light);
  Print_in_window(141, 88,"Colors modifications",MC_Dark,MC_Light);
  Print_in_window( 20,102,"Brush",MC_Dark,MC_Light);
  Print_in_window( 16,110,"handle",MC_Dark,MC_Light);

  // Dessin des pointillés pour le "brush handle"
  for (index=0; index<13; index+=2)
  {
    Pixel_in_window( 88+index, 92,MC_Dark);
    Pixel_in_window( 88+index,126,MC_Dark);
    Pixel_in_window( 77,103+index,MC_Dark);
    Pixel_in_window(111,103+index,MC_Dark);
  }
  // Dessin des coins et du centre pour les boutons du "brush handle"
    // Coin HG
  Window_rectangle(77, 92, 7, 1, MC_Black);
  Window_rectangle(77, 92, 1, 7, MC_Black);
    // Coin HD
  Window_rectangle(105, 92, 7, 1, MC_Black);
  Window_rectangle(111, 92, 1, 7, MC_Black);
    // Centre
  Window_rectangle(91, 109, 7, 1, MC_Black);
  Window_rectangle(94, 106, 1, 7, MC_Black);
    // Coin BG
  Window_rectangle(77, 126, 7, 1, MC_Black);
  Window_rectangle(77, 120, 1, 7, MC_Black);
    // Coin BD
  Window_rectangle(105, 126, 7, 1, MC_Black);
  Window_rectangle(111, 120, 1, 7, MC_Black);

  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    if (Is_shortcut(Key,0x100+BUTTON_HELP))
    {
      Key=0;
      Window_help(BUTTON_BRUSH_EFFECTS, NULL);
    }
    else if (Is_shortcut(Key,0x100+BUTTON_BRUSH_EFFECTS))
    {
      clicked_button=1;
    }
  }
  while (clicked_button<=0 && !Quit_is_required);

  Close_window();
  Unselect_button(btn);

  // Gestion du bouton clické
  switch (clicked_button)
  {
    case  2 : // Flip X
      Flip_X_lowlevel(Brush_original_pixels, Brush_width, Brush_height);
      // Remap according to the last used remap table
      Remap_general_lowlevel(Brush_colormap,Brush_original_pixels,Brush,Brush_width,Brush_height,Brush_width);
      break;
    case  3 : // Flip Y
      Flip_Y_lowlevel(Brush_original_pixels, Brush_width, Brush_height);
      // Remap according to the last used remap table
      Remap_general_lowlevel(Brush_colormap,Brush_original_pixels,Brush,Brush_width,Brush_height,Brush_width);
      break;
    case  4 : // 90° Rotation
      Rotate_90_deg();
      break;
    case  5 : // 180° Rotation
      Rotate_180_deg_lowlevel(Brush_original_pixels, Brush_width, Brush_height);
      Remap_general_lowlevel(Brush_colormap,Brush_original_pixels,Brush,Brush_width,Brush_height,Brush_width);
      Brush_offset_X=(Brush_width>>1);
      Brush_offset_Y=(Brush_height>>1);
      break;
    case  6 : // Any angle rotation
      Start_operation_stack(OPERATION_ROTATE_BRUSH);
      break;
    case  7 : // Stretch
      Start_operation_stack(OPERATION_STRETCH_BRUSH);
      break;
    case  8 : // Distort
      Start_operation_stack(OPERATION_DISTORT_BRUSH);
      break;
    case  9 : // Recolorize
      Remap_brush();
      break;
    case 10 : // Get brush colors
      Display_cursor();
      Get_colors_from_brush();
      Hide_cursor();
      break;
    case 11 : // Brush Attachment: Top-Left
      Brush_offset_X=0;
      Brush_offset_Y=0;
      break;
    case 12 : // Brush Attachment: Top-Right
      Brush_offset_X=(Brush_width-1);
      Brush_offset_Y=0;
      break;
    case 13 : // Brush Attachment: Center
      Brush_offset_X=(Brush_width>>1);
      Brush_offset_Y=(Brush_height>>1);
      break;
    case 14 : // Brush Attachment: Bottom-Left
      Brush_offset_X=0;
      Brush_offset_Y=(Brush_height-1);
      break;
    case 15 : // Brush Attachment: Bottom-Right
      Brush_offset_X=(Brush_width-1);
      Brush_offset_Y=(Brush_height-1);
      break;
    case 16 : // Outline
      Outline_brush();
      break;
    case 17 : // Nibble
      Nibble_brush();
      break;
    case 18 : // Load
      Display_cursor();
      Load_picture(CONTEXT_BRUSH);
      Hide_cursor();
      break;
    case 19 : // Save
      Display_cursor();
      Save_picture(CONTEXT_BRUSH);
      Hide_cursor();
      break;
  }

  Display_cursor();
}


//---------------------------- Courbes de Bézier ----------------------------

void Button_Curves(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(Selected_curve_mode);
  Display_cursor();
}


void Button_Curves_switch_mode(int btn)
{
  if (Selected_curve_mode==OPERATION_4_POINTS_CURVE)
    Selected_curve_mode=OPERATION_3_POINTS_CURVE;
  else
    Selected_curve_mode=OPERATION_4_POINTS_CURVE;

  Hide_cursor();
  Display_sprite_in_menu(btn,Selected_curve_mode==OPERATION_4_POINTS_CURVE?MENU_SPRITE_4_POINTS_CURVE:-1);
  Draw_menu_button(btn,BUTTON_PRESSED);
  Start_operation_stack(Selected_curve_mode);
  Display_cursor();
}


//--------------------------------- Spray -----------------------------------

void Button_Airbrush(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_AIRBRUSH);
  Display_cursor();
}


void Refresh_airbrush_settings(byte selected_color, byte update_slider)
{
  char  str[4];

  if (update_slider)
  {
    Window_scroller_button_list->Position=49-Airbrush_multi_flow[selected_color];
    Window_draw_slider(Window_scroller_button_list);
  }
  Num2str(Airbrush_multi_flow[selected_color],str,2);
  Print_in_window(196,130,str,MC_Black,MC_Light);

  Update_window_area(Window_palette_button_list->Pos_X+4+(selected_color >> 4)*10,
      Window_palette_button_list->Pos_Y+3+(selected_color & 15)* 5,
      2,5);
}


void Button_Airbrush_menu(int btn)
{
  static byte spray_init=1;
  short  clicked_button;
  char   str[4];
  word   index;
  byte   selected_color=Fore_color;
  byte   old_airbrush_mode     =Airbrush_mode;
  short  old_airbrush_size     =Airbrush_size;
  byte   old_airbrush_delay    =Airbrush_delay;
  byte   old_airbrush_mono_flow=Airbrush_mono_flow;
  byte   old_airbrush_multi_flow[256];
  T_Special_button * input_size_button;
  T_Special_button * input_delay_button;
  T_Special_button * input_flow_button;
  T_Special_button * input_init_button;
  word old_mouse_x;
  word old_mouse_y;
  byte old_mouse_k;
  byte color;
  byte click;

  (void)btn;
  memcpy(old_airbrush_multi_flow,Airbrush_multi_flow,256);


  Open_window(226,170,"Spray");

  Window_set_normal_button(110,148,51,14,"Cancel"    ,0,1,KEY_ESC); // 1
  Window_set_normal_button(166,148,51,14,"OK"        ,0,1,KEY_RETURN); // 2

  Window_set_scroller_button(178,62,74,50,1,49-Airbrush_multi_flow[selected_color]); // 3

  Window_set_palette_button(7,56);                                 // 4

  Window_set_normal_button(  8,148,83,14,"Mode:     ",0,1,KEY_TAB); // 5
  if (Airbrush_mode)
    Print_in_window(50,151," Mono",MC_Black,MC_Light);
  else
    Print_in_window(50,151,"Multi",MC_Black,MC_Light);

  Window_set_normal_button(194, 62,19,14,"+1"        ,0,1,KEY_KP_PLUS); // 6
  Window_set_normal_button(194, 79,19,14,"-1"        ,0,1,KEY_KP_MINUS); // 7
  Window_set_normal_button(194, 96,19,14,"x2"        ,0,1,KEY_KP_MULTIPLY); // 8
  Window_set_normal_button(194,113,19,14,"\xf7""2"   ,0,1,KEY_KP_ENTER); // 9

  Window_set_normal_button(  8, 37,43,14,"Clear"     ,1,1,KEY_c); // 10

  Print_in_window_underscore(142,25,"Size:"     ,MC_Dark,MC_Light,1);
  input_size_button = Window_set_input_button_s(186,23,3,KEY_s); // 11
  Num2str(Airbrush_size,str,3);
  Window_input_content(input_size_button,str);

  Print_in_window_underscore(142,39,"Delay:"    ,MC_Dark,MC_Light,1);
  input_delay_button = Window_set_input_button_s(194,37,2,KEY_d); // 12
  Num2str(Airbrush_delay,str,2);
  Window_input_content(input_delay_button,str);

  Print_in_window_underscore( 27,24,"Mono-Flow:",MC_Dark,MC_Light,1);
  input_flow_button = Window_set_input_button_s(111,22,2,KEY_m); // 13
  Num2str(Airbrush_mono_flow,str,2);
  Window_input_content(input_flow_button,str);

  Print_in_window_underscore( 67,40,"Init:",MC_Dark,MC_Light,1);
  input_init_button = Window_set_input_button_s(111,38,2,KEY_i); // 14
  Num2str(spray_init,str,2);
  Window_input_content(input_init_button,str);

  Window_display_frame(173,56,45,86);
  Window_display_frame(137,19,81,33);

  // On tagge toutes les couleurs utilisées
  for (index=0; index<256; index++)
    if (Airbrush_multi_flow[index])
      Stencil_tag_color(index,MC_Black);
  // Et enfin, on tagge la couleur sélectionnée
  Stencil_tag_color(selected_color,MC_White);
  Refresh_airbrush_settings(selected_color,0);

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();
  Stencil_update_color(selected_color);


  do
  {
    old_mouse_x=Mouse_X;
    old_mouse_y=Mouse_Y;
    old_mouse_k=Mouse_K;

    clicked_button=Window_clicked_button();

    switch (clicked_button)
    {
      case  0 :
      case  2 : // OK
        break;

      case  1 : // Cancel
        Airbrush_mode     =old_airbrush_mode;
        Airbrush_size     =old_airbrush_size;
        Airbrush_delay    =old_airbrush_delay;
        Airbrush_mono_flow=old_airbrush_mono_flow;
        memcpy(Airbrush_multi_flow,old_airbrush_multi_flow,256);
        break;

      case  3 : // Scroller
        Hide_cursor();
        Airbrush_multi_flow[selected_color]=49-Window_attribute2;
        Refresh_airbrush_settings(selected_color,0);
        Display_cursor();
        break;

      case -1 :
      case  4 : // Palette
        if ( (Mouse_X!=old_mouse_x) || (Mouse_Y!=old_mouse_y) || (Mouse_K!=old_mouse_k) )
        {
          Hide_cursor();
          Stencil_tag_color(selected_color,(Airbrush_multi_flow[selected_color])?MC_Black:MC_Light);
          Stencil_update_color(selected_color);
          // Mettre la couleur sélectionnée à jour suivant le click
          selected_color=(clicked_button==4) ? Window_attribute2 : Read_pixel(Mouse_X,Mouse_Y);
          if (Mouse_K==2)
            Airbrush_multi_flow[selected_color]=0;
          else
            if (Airbrush_multi_flow[selected_color]==0)
              Airbrush_multi_flow[selected_color]=spray_init;

          // Tagger la couleur sélectionnée en blanc
          Stencil_tag_color(selected_color,MC_White);
          Refresh_airbrush_settings(selected_color,1);
          Display_cursor();
          Stencil_update_color(selected_color);
        }
        break;

      case  5 : // Toggle Mode
        Airbrush_mode=(Airbrush_mode+1)&1;
        Hide_cursor();
        if (Airbrush_mode)
          Print_in_window(50,151," Mono",MC_Black,MC_Light);
        else
          Print_in_window(50,151,"Multi",MC_Black,MC_Light);
        Update_window_area(50,151,5*8,8);
        Display_cursor();
        break;

      case  6 : // +1
        for (index=0; index<256; index++)
        {
          if ( (Airbrush_multi_flow[index]) && (Airbrush_multi_flow[index]<49) )
            Airbrush_multi_flow[index]++;
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case  7 : // -1
        for (index=0; index<256; index++)
        {
          if (Airbrush_multi_flow[index]>1)
            Airbrush_multi_flow[index]--;
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case  8 : // x2
        for (index=0; index<256; index++)
        {
          if (Airbrush_multi_flow[index])
          {
            Airbrush_multi_flow[index]<<=1;
            if (Airbrush_multi_flow[index]>49)
              Airbrush_multi_flow[index]=49;
          }
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case  9 : // ÷2
        for (index=0; index<256; index++)
        {
          if (Airbrush_multi_flow[index]>1)
            Airbrush_multi_flow[index]>>=1;
        }
        Hide_cursor();
        Refresh_airbrush_settings(selected_color,1);
        Display_cursor();
        break;

      case 10 : // Clear
        memset(Airbrush_multi_flow,0,256);
        // On raffiche les infos de la couleur sélectionnée
        Refresh_airbrush_settings(selected_color,1);
        // On efface les anciens TAGs
        Window_clear_tags();
        // Tagger la couleur sélectionnée en blanc
        Stencil_tag_color(selected_color,MC_White);
        Stencil_update_color(selected_color);
        break;

      case 11 : // Size
        Num2str(Airbrush_size,str,3);
        Readline(188,25,str,3,INPUT_TYPE_INTEGER);
        Airbrush_size=atoi(str);
        // On corrige les dimensions
        if (Airbrush_size>256)
        {
          Airbrush_size=256;
          Num2str(Airbrush_size,str,3);
          Window_input_content(input_size_button,str);
        }
        else if (!Airbrush_size)
        {
          Airbrush_size=1;
          Num2str(Airbrush_size,str,3);
          Window_input_content(input_size_button,str);
        }
        Display_cursor();
        break;

      case 12 : // Delay
        Num2str(Airbrush_delay,str,2);
        Readline(196,39,str,2,INPUT_TYPE_INTEGER);
        Airbrush_delay=atoi(str);
        // On corrige le delai
        if (Airbrush_delay>99)
        {
          Airbrush_delay=99;
          Num2str(Airbrush_delay,str,2);
          Window_input_content(input_delay_button,str);
        }
        Display_cursor();
        break;

      case 13 : // Mono-Flow
        Num2str(Airbrush_mono_flow,str,2);
        Readline(113,24,str,2,INPUT_TYPE_INTEGER);
        Airbrush_mono_flow=atoi(str);
        // On corrige le flux
        if (!Airbrush_mono_flow)
        {
          Airbrush_mono_flow=1;
          Num2str(Airbrush_mono_flow,str,2);
          Window_input_content(input_flow_button,str);
        }
        Display_cursor();
        break;

      case 14 : // Init
        Num2str(spray_init,str,2);
        Readline(113,40,str,2,INPUT_TYPE_INTEGER);
        spray_init=atoi(str);
        // On corrige la valeur
        if (spray_init>=50)
        {
          spray_init=49;
          Num2str(spray_init,str,2);
          Window_input_content(input_init_button,str);
        }
        else if (spray_init<1)
        {
          spray_init=1;
          Num2str(spray_init,str,2);
          Window_input_content(input_init_button,str);
        }
        Display_cursor();
        break;
    }

    if (!Mouse_K)
    switch (Key)
    {
      case KEY_BACKQUOTE : // Récupération d'une couleur derrière le menu
      case KEY_COMMA :
        Get_color_behind_window(&color,&click);
        if (click)
        {
          Hide_cursor();
          Stencil_tag_color(selected_color,(Airbrush_multi_flow[selected_color])?MC_Black:MC_Light);
          Stencil_update_color(selected_color);
          // Mettre la couleur sélectionnée à jour suivant le click
          selected_color=color;
          if (click==2)
            Airbrush_multi_flow[selected_color]=0;
          else
            if (Airbrush_multi_flow[selected_color]==0)
              Airbrush_multi_flow[selected_color]=spray_init;

          // Tagger la couleur sélectionnée en blanc
          Stencil_tag_color(selected_color,MC_White);
          Refresh_airbrush_settings(selected_color,1);
          Display_cursor();
          Stencil_update_color(selected_color);
          Wait_end_of_click();
        }
        Key=0;
        break;
      default:
      if (Is_shortcut(Key,0x100+BUTTON_HELP))
      {
        Window_help(BUTTON_AIRBRUSH, NULL);
        Key=0;
        break;
      }
      if (Is_shortcut(Key,0x200+BUTTON_AIRBRUSH))
      {
        clicked_button=2;
        break;
      }
    }
  }
  while ( (clicked_button!=1) && (clicked_button!=2) && !Quit_is_required);

  Close_window();

/*
  //   Tant que l'on aura pas résolu le problème du désenclenchement du mode
  // de dessin précedent, il faudra laisser ça en remarque et donc passer en
  // spray même si on a clické sur Cancel (idem pour OK (un peu plus bas)).
  if (clicked_button==1) // Cancel
  {
    if (Current_operation!=OPERATION_AIRBRUSH)
      Unselect_button(BUTTON_AIRBRUSH);
  }
*/

  Display_cursor();

/*
  if (clicked_button==2) // OK
*/
    if (Current_operation!=OPERATION_AIRBRUSH)
      Select_button(BUTTON_AIRBRUSH,LEFT_SIDE);
}


// -- Gestion des boutons de polygone vide et plein -------------------------

void Button_polygon(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_POLYGON);
  Display_cursor();
}


void Button_Polyform(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_POLYFORM);
  Display_cursor();
}


void Button_Polyfill(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_POLYFILL);
  Display_cursor();
}


void Button_Filled_polyform(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_FILLED_POLYFORM);
  Display_cursor();
}


// -- Boutons d'ajustement de l'image ---------------------------------------

void Button_Adjust(int btn)
{
  (void)btn;
  Hide_cursor();
  Start_operation_stack(OPERATION_SCROLL);
  Display_cursor();
}


// -- Menu des effets (Shade, Stencil, etc...) ------------------------------

void Display_effect_sprite(int sprite_number, short start_x, short start_y)
{
  short x,y,x_pos,y_pos;

  for (y=0,y_pos=start_y;y<EFFECT_SPRITE_HEIGHT;y++,y_pos++)
    for (x=0,x_pos=start_x;x<EFFECT_SPRITE_WIDTH;x++,x_pos++)
      Pixel_in_window(x_pos,y_pos,Gfx->Effect_sprite[sprite_number][y][x]);

  Update_rect(ToWinX(start_x),ToWinY(start_y),EFFECT_SPRITE_WIDTH*Menu_factor_X,EFFECT_SPRITE_HEIGHT*Menu_factor_Y);
}


void Display_effect_state(short x, short y, char * label, byte state)
{
  Window_rectangle(x+22,y+5,12,8,MC_Light);

  Print_in_window(x+22,y+5,label,(state)?MC_White:MC_Black,MC_Light);
  if (state)
    Window_select_normal_button(x, y, 16, 16);
  else
    Window_unselect_normal_button(x, y, 16, 16);
}

#define C1 10
#define C2 99
#define C3 184

#define L1 19
#define L2 38
#define L3 57
#define L4 76
#define L5 95

void Display_effect_states(void)
{
  Display_effect_state(C1, L2, "Shade"  ,Shade_mode);
  Display_effect_state(C1, L3, "Q-shade",Quick_shade_mode);
  Display_effect_state(C1, L4, "Transp.",Colorize_mode);
  Display_effect_state(C1, L5, "Smooth" ,Smooth_mode);
  Display_effect_state(C2, L5, "Smear"  ,Smear_mode);
  Display_effect_state(C2, L1, "Stencil",Stencil_mode);
  Display_effect_state(C2, L2, "Mask"   ,Mask_mode);
  Display_effect_state(C2, L3, "Sieve"  ,Sieve_mode);
  Display_effect_state(C3, L2, "Snap"   ,Snap_mode);
  Display_effect_state(C3, L4, "Tiling" ,Tiling_mode);

  Display_effect_state(C2,L4, "8 bit"  ,Main.backups->Pages->Image_mode > IMAGE_MODE_ANIMATION);
  Display_effect_state(C3,L3, "Tilemap",Main.tilemap_mode);
}


void Display_feedback_state(void)
{
  Print_in_window(14,24,(Config.FX_Feedback)?"X":" ",MC_Black,MC_Light);
}


void Button_Effects(int btn)
{
  short clicked_button;
  byte exit_by_close_button=0;

  Open_window(270,152,"Drawing modes (effects)");

  Window_set_normal_button(C1, L2, 16,16,"",0,1,Config_Key[SPECIAL_SHADE_MODE][0]); // 1
  Window_set_normal_button(C1, L3, 16,16,"",0,1,Config_Key[SPECIAL_QUICK_SHADE_MODE][0]); // 2
  Window_set_normal_button(C1, L4, 16,16,"",0,1,Config_Key[SPECIAL_COLORIZE_MODE][0]); // 3
  Window_set_normal_button(C1, L5, 16,16,"",0,1,Config_Key[SPECIAL_SMOOTH_MODE][0]); // 4
  Window_set_normal_button(C2, L5, 16,16,"",0,1,Config_Key[SPECIAL_SMEAR_MODE][0]); // 5
  Window_set_normal_button(C2, L1, 16,16,"",0,1,Config_Key[SPECIAL_STENCIL_MODE][0]); // 6
  Window_set_normal_button(C2, L2, 16,16,"",0,1,Config_Key[SPECIAL_MASK_MODE][0]); // 7
  Window_set_normal_button(C2, L3, 16,16,"",0,1,Config_Key[SPECIAL_SIEVE_MODE][0]); // 8
  Window_set_normal_button(C3, L2, 16,16,"",0,1,Config_Key[SPECIAL_GRID_MODE][0]); // 9
  Window_set_normal_button(C3, L4, 16,16,"",0,1,Config_Key[SPECIAL_TILING_MODE][0]); // 10

  Window_set_normal_button(195,131, 68,14,"Close",0,1,KEY_RETURN); // 11
  Window_set_normal_button(118,131, 68,14,"All off",0,1,KEY_DELETE); // 12

  // "Feedback" frame
  Window_display_frame_mono(C1-5,L1+8,90,88,MC_Dark);
  Window_rectangle(C1-1, L1+2, 78, 14, MC_Light);

  Window_set_normal_button(C1+1,L1+2,14,14," ",0,1,KEY_f); // 13
  Print_in_window_underscore(28,24,"Feedback",MC_Dark,MC_Light,1);

  Window_set_normal_button(C2, L4, 16,16,"",0,1,Config_Key[SPECIAL_FORMAT_CHECKER_MENU][0]); // 14
  Window_set_normal_button(C3, L3, 16,16,"",0,1,Config_Key[SPECIAL_TILEMAP_MODE][0]); // 15

  // "Grid" frame
  Window_display_frame_mono(C3-5,L1+8,86,88,MC_Dark);
  Window_rectangle(C3-1, L1+2, 52, 14, MC_Light);

  Window_set_normal_button(C3+1,L1+2,14,14,Show_grid?"X":" ",0,1,Config_Key[SPECIAL_SHOW_GRID][0]); // 16
  Print_in_window(C3+17,L1+5,"Grid",MC_Dark,MC_Light);

  Display_feedback_state();
  Display_effect_sprite(EFFECTS_SPRITE_SHADE,  C1+1,L2+1);
  Display_effect_sprite(EFFECTS_SPRITE_SHADE,  C1+1,L3+1);
  Display_effect_sprite(EFFECTS_SPRITE_TRANSP, C1+1,L4+1);
  Display_effect_sprite(EFFECTS_SPRITE_SMOOTH, C1+1,L5+1);
  Display_effect_sprite(EFFECTS_SPRITE_SMEAR,  C2+1,L5+1);
  Display_effect_sprite(EFFECTS_SPRITE_STENCIL,C2+1,L1+1);
  Display_effect_sprite(EFFECTS_SPRITE_MASK,   C2+1,L2+1);
  Display_effect_sprite(EFFECTS_SPRITE_SIEVE,  C2+1,L3+1);
  Display_effect_sprite(EFFECTS_SPRITE_GRID,   C3+1,L2+1);
  Display_effect_sprite(EFFECTS_SPRITE_TILING, C3+1,L4+1);
  Display_effect_sprite(EFFECTS_SPRITE_8BIT,   C2+1,L4+1);
  Display_effect_sprite(EFFECTS_SPRITE_TILING, C3+1,L3+1); // tilemap
  Display_effect_states();

  Print_in_window(12,118,"click:",MC_Dark,MC_Light);
  Print_in_window(16,128,"Left:Switch",MC_Dark,MC_Light);
  Print_in_window(16,138,"Right:Edit",MC_Dark,MC_Light);

  Update_window_area(0,0,Window_width, Window_height);
  Display_cursor();

  do
  {
    clicked_button = Window_clicked_button();

    if (Key==KEY_ESC || Is_shortcut(Key,0x100+BUTTON_EFFECTS))
    {
      clicked_button=11;
      Key=0;
    }
    else if (Is_shortcut(Key,0x100+BUTTON_HELP))
    {
      // Aide contextuelle
      switch(Window_get_clicked_button())
      {
        case 1:
          Window_help(BUTTON_EFFECTS, "SHADE");
          break;
        case 2:
          Window_help(BUTTON_EFFECTS, "QUICK SHADE");
          break;
        case 3:
          Window_help(BUTTON_EFFECTS, "TRANSPARENCY");
          break;
        case 4:
          Window_help(BUTTON_EFFECTS, "SMOOTH");
          break;
        case 5:
          Window_help(BUTTON_EFFECTS, "SMEAR");
          break;
        case 6:
          Window_help(BUTTON_EFFECTS, "STENCIL");
          break;
        case 7:
          Window_help(BUTTON_EFFECTS, "MASK");
          break;
        case 8:
          Window_help(BUTTON_EFFECTS, "SIEVE");
          break;
        case 9:
          Window_help(BUTTON_EFFECTS, "GRID");
          break;
        case 10:
          Window_help(BUTTON_EFFECTS, "TILING");
          break;
        case 14:
          Window_help(BUTTON_EFFECTS, "8 BIT");
          break;
        case 15:
          Window_help(BUTTON_EFFECTS, "TILEMAP");
          break;
        default:
          Window_help(BUTTON_EFFECTS, NULL);
      }
      // Hack because we have used Window_get_clicked_button()
      Input_sticky_control=0;
      //
    }

    switch (clicked_button)
    {
      case 1 : // Shade
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Shade_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Shade_menu();
          clicked_button=11;
        }
        break;
      case 2 : // Quick-shade
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Quick_shade_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Quick_shade_menu();
          clicked_button=11;
        }
        break;
      case 3 : // Colorize / Transparency
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Colorize_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Colorize_menu();
          clicked_button=11;
        }
        break;
      case 4 : // Smooth
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Smooth_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Smooth_menu();
          clicked_button=11;
        }
        break;
      case 5 : // Smear
        Button_Smear_mode();
        Hide_cursor();
        Display_effect_states();
        Display_cursor();
        break;
      case 6 : // Stencil
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Stencil_mode();
          Hide_cursor();
          Display_effect_state(C2,L1,"Stencil",Stencil_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Stencil_menu();
          clicked_button=11;
        }
        break;
      case 7 : // Mask
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Mask_mode();
          Hide_cursor();
          Display_effect_state(C2,L2,"Mask",Mask_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Mask_menu();
          clicked_button=11;
        }
        break;
      case 8 : // Sieve
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Sieve_mode();
          Hide_cursor();
          Display_effect_state(C2,L3,"Sieve",Sieve_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Sieve_menu();
          clicked_button=11;
        }
        break;
      case 9 : // Grid
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Snap_mode();
          Hide_cursor();
          Display_effect_state(C3,L2,"Snap",Snap_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Grid_menu();
          clicked_button=11;
        }
        break;
      case 10 : // Tiling
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Tiling_mode();
          Hide_cursor();
          Display_effect_states();
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Tiling_menu();
          clicked_button=11;
        }
        break;
      case 11 : // Close
        exit_by_close_button=1;
        break;
      case 12 : // All off
        Effects_off();
        Hide_cursor();
        Display_effect_states();
        Display_cursor();
        break;
      case 13 : // Feedback (pour Colorize et Shade)
        Config.FX_Feedback = !Config.FX_Feedback;
        Update_FX_feedback(Config.FX_Feedback);
        Hide_cursor();
        Display_feedback_state();
        Display_cursor();
        break;


      case 14: // Constraint checker/enforcer
        if (Window_attribute1==LEFT_SIDE && Selected_Constraint_Mode > IMAGE_MODE_ANIMATION)
        {
          Button_Constraint_mode();
          Hide_cursor();
          Display_effect_state(C2,L4, "8 bit" ,Main.backups->Pages->Image_mode > IMAGE_MODE_ANIMATION);
          Display_cursor();
        } else {
          Close_window();
          Display_cursor();
          Button_Constraint_menu();
          clicked_button = 11;
        }
        break;
      case 15: // Tilemap
        if (Window_attribute1==LEFT_SIDE)
        {
          Button_Tilemap_mode();
          Hide_cursor();
          Display_effect_state(C3,L3, "Tilemap" ,Main.tilemap_mode);
          Display_cursor();
        }
        else
        {
          Close_window();
          Display_cursor();
          Button_Tilemap_menu();
          clicked_button=11;
        }
        break;
      case 16: // Show grid
        Show_grid = !Show_grid;
        Hide_cursor();
        Print_in_window(C3+4, L1+5, Show_grid?"X":" ", MC_Black, MC_Light);
        Display_cursor();
    }
  }
  while (clicked_button!=11 && !Quit_is_required);

  if (exit_by_close_button || Quit_is_required)
    Close_window();
  else
    Hide_cursor();

  if (!Any_effect_active())
    Unselect_button(btn);

  Display_cursor();
}

#undef C2

// Callback to display a font name in the list
void Draw_one_font_name(word x, word y, word index, byte highlighted)
{
  Print_in_window(x,y,Font_label(index), MC_Black, (highlighted)?MC_Dark:MC_Light);
}

void Button_Text(int btn)
{
  static char str[256]="";
  static int font_size=32;
  static int antialias=1;
  static short list_start=0; // index de le premiere fonte dans le selector
  static short cursor_position=0; // index de la ligne active dans le selector
  static short selected_font_index=0;
  static short is_bold=0;
  static short is_italic=0;

  byte * new_brush=NULL;
  T_Palette text_palette;
  int new_width;
  int new_height;
  int clicked_button;
  const int NB_FONTS=8;
  char size_buffer[4];
  T_Special_button * input_size_button;
  T_Special_button * input_text_button;
  T_Special_button * preview_button;
  T_Special_button * font_list_button;
  T_Scroller_button * font_scroller;
  T_List_button * font_list;

  byte redraw_is_needed=1;
  byte preview_is_needed=1;

  (void)btn;
  Open_window(288,180,"Text");

  // Texte saisi
  Print_in_window_underscore(6,20,"Text:",MC_Dark,MC_Light,1);
  input_text_button = Window_set_input_button_s(48,18,29,KEY_t); // 1

  // TrueType options
  Window_display_frame_in(182,34,100,68);
  Print_in_window(199,31,"TrueType", MC_Dark, MC_Light);
  // AA
  Window_set_normal_button(188,58,13,11,antialias?"X":" ",0,1,KEY_a); // 2
  Print_in_window_underscore(206,60,"AntiAlias", MC_Dark, MC_Light,5);
  // Bold
  Window_set_normal_button(188,72,13,11,is_bold?"X":" ",0,1,KEY_b); // 3
  Print_in_window_underscore(206,75,"Bold", MC_Dark, MC_Light,1);
  // Italic
  Window_set_normal_button(188,86,13,11,is_italic?"X":" ",0,1,KEY_i); // 4
  Print_in_window_underscore(206,89,"Italic", MC_Dark, MC_Light,1);

  // Scroller des fontes
  font_scroller = Window_set_scroller_button(165,35,NB_FONTS*8,Font_count(),NB_FONTS,list_start); // 5
  // Liste des fontes disponibles
  font_list_button = Window_set_special_button(8,35,152,NB_FONTS*8,0); // 6
  Window_display_frame_in(7, 33, 154, NB_FONTS*8+4);

  // Taille texte
  input_size_button = Window_set_input_button_s(220,43,3,KEY_s); // 7
  Window_set_repeatable_button(202,43,13,11,"-",0,1,KEY_NONE); // 8
  Window_set_repeatable_button(251,43,13,11,"+",0,1,KEY_NONE); // 9

  // Preview
  preview_button = Window_set_special_button(8,106,273,50,0); // 10
  Window_display_frame_in(7, 105, 275, 52);

  Window_set_normal_button(8,160,40,14,"OK",0,1,KEY_RETURN); // 11
  Window_set_normal_button(54,160,60,14,"Cancel",0,1,KEY_ESC); // 12

  // List of fonts
  font_list = Window_set_list_button(font_list_button, font_scroller, Draw_one_font_name, 2); // 13
  // Restore its settings from last passage in screen
  font_list->List_start = list_start;
  font_list->Cursor_position = cursor_position;

  Window_redraw_list(font_list);

  Update_window_area(0,0,Window_width, Window_height);

  // str texte
  Window_input_content(input_text_button,str);
  // Taille police
  redraw_is_needed=1;
  // --

  while (1)
  {
    if (redraw_is_needed)
    {
      // Taille
      Num2str(font_size,size_buffer,3);
      Window_input_content(input_size_button,size_buffer);
    }
    if (preview_is_needed)
    {
      const char * preview_string = "AaBbCcDdEeFf012345";
      byte is_truetype;

      if (str[0])
        preview_string=str;
      is_truetype=TrueType_font(selected_font_index);
      free(new_brush);
      new_brush = Render_text(preview_string, selected_font_index, font_size, antialias, is_bold, is_italic, &new_width, &new_height, text_palette);
      // Background:
      if (antialias&&is_truetype)
        // Solid
        Window_rectangle(8, 106, 273, 50,MC_Black);
      else if (is_truetype)
      {
        long l = text_palette[Fore_color].R+text_palette[Fore_color].G+text_palette[Fore_color].B;
        Window_rectangle(8, 106, 273, 50,l>128*3? MC_Black:MC_Light);
      }
      else
      {
        long l = text_palette[Back_color].R+text_palette[Back_color].G+text_palette[Back_color].B;
        Window_rectangle(8, 106, 273, 50,l>128*3? MC_Light:MC_Black);
      }
      if (new_brush)
      {
        if (!is_truetype || (is_truetype&&antialias))
        {
          // Display brush in remapped form.
          byte *remapped_brush;

          remapped_brush=(byte *)malloc(new_width*new_height);
          if (remapped_brush)
          {
            // This code is mostly copied from Remap_brush()
            short x_pos;
            short y_pos;
            int   color;
            byte colmap[256];

            for (color=0;color<=255;color++)
              colmap[color]=0;

            for (y_pos=0;y_pos<new_height;y_pos++)
              for (x_pos=0;x_pos<new_width;x_pos++)
                colmap[*(new_brush + y_pos * new_width + x_pos)]=1;

            colmap[Back_color]=0;

            for (color=0;color<=255;color++)
              if (colmap[color] != 0)
              {
                byte r,g,b;
                r=text_palette[color].R;
                g=text_palette[color].G;
                b=text_palette[color].B;

                //if (r==Main.palette[color].R && g==Main.palette[color].G && b==Main.palette[color].B)
                //  colmap[color]=color;
                //else
                  colmap[color]=Best_color_perceptual_except(r,g,b,Back_color);
              }

            colmap[Back_color]=Back_color;
            Remap_general_lowlevel(colmap,new_brush,remapped_brush,new_width,new_height,new_width);

            Display_brush(
              remapped_brush,
              Window_pos_X+preview_button->Pos_X*Menu_factor_X,
              Window_pos_Y+preview_button->Pos_Y*Menu_factor_Y,
              0,
              0,
              Min(preview_button->Width*Menu_factor_X, new_width),
              Min(preview_button->Height*Menu_factor_Y, new_height),
              Back_color,
              new_width);

            free(remapped_brush);
          }

        }
        else
        {
          // Solid
          Display_brush(
            new_brush,
            Window_pos_X+preview_button->Pos_X*Menu_factor_X,
            Window_pos_Y+preview_button->Pos_Y*Menu_factor_Y,
            0,
            0,
            Min(preview_button->Width*Menu_factor_X, new_width),
            Min(preview_button->Height*Menu_factor_Y, new_height),
            Back_color,
            new_width);
        }

      }
      Update_window_area(
        preview_button->Pos_X,
        preview_button->Pos_Y,
        preview_button->Width,
        preview_button->Height);
    }
    if (redraw_is_needed || preview_is_needed)
    {
      redraw_is_needed=0;
      preview_is_needed=0;
      Display_cursor();
    }

    clicked_button=Window_clicked_button();
    if (Quit_is_required)
      clicked_button = 12; // cancel
    if (clicked_button==0)
    {
      if (Is_shortcut(Key,0x100+BUTTON_HELP))
        Window_help(BUTTON_TEXT, NULL);
      else if (Is_shortcut(Key,0x100+BUTTON_TEXT))
        clicked_button = 12;  // cancel
    }
    switch(clicked_button)
    {
      case 1: // Texte saisi
      Readline_ex(50,20,str,29,250,INPUT_TYPE_STRING,0);
      preview_is_needed=1;
      break;

      case 2: // AA
      antialias = (antialias==0);
      Hide_cursor();
      Print_in_window(191,60,antialias?"X":" ", MC_Black, MC_Light);
      preview_is_needed=1;
      break;

      case 3: // Bold
      is_bold = (is_bold==0);
      Hide_cursor();
      Print_in_window(191,74,is_bold?"X":" ", MC_Black, MC_Light);
      preview_is_needed=1;
      break;

      case 4: // Italic
      is_italic = (is_italic==0);
      Hide_cursor();
      Print_in_window(191,88,is_italic?"X":" ", MC_Black, MC_Light);
      preview_is_needed=1;
      break;

      case 5: // Scroller des fontes
      /* Cannot happen, event is catched by the list control */
      break;

      case 13: // Font selection
        selected_font_index = Window_attribute2;
        Hide_cursor();
        preview_is_needed=1;
      break;

      case 7: // Taille du texte (nombre)
      Readline(222,45,size_buffer,3,INPUT_TYPE_INTEGER);
      font_size=atoi(size_buffer);
      // On corrige les dimensions
      if (font_size < 1)
      {
        font_size = 1;
      }
      else if (font_size>500)
      {
        font_size = 500;
      }
      redraw_is_needed=1;
      preview_is_needed=1;
      break;

      case 8: // Taille -
      if (font_size > 1)
      {
        font_size--;
        Hide_cursor();
        redraw_is_needed=1;
        preview_is_needed=1;
      }
      break;

      case 9: // Taille +
      if (font_size < 255)
      {
        font_size++;
        Hide_cursor();
        redraw_is_needed=1;
        preview_is_needed=1;
      }
      break;


      case 6: // Double-click font selector
      case 11: // OK
      // Save the selector settings
      list_start = font_list->List_start;
      cursor_position = font_list->Cursor_position;

      if (!new_brush)
      {
        // Si echec de rendu
        Close_window();
        Unselect_button(BUTTON_TEXT);
        Display_cursor();
        Error(0);
        return;
      }
      if (Realloc_brush(new_width, new_height, new_brush, NULL))
      {
        free(new_brush);
        Close_window();
        Unselect_button(BUTTON_TEXT);
        Display_cursor();
        Error(0);
      }
      // Grab palette
      memcpy(Brush_original_palette, text_palette,sizeof(T_Palette));
      // Remap to image's palette
      Remap_brush();

      Brush_offset_X=Brush_width>>1;
      Brush_offset_Y=Brush_height>>1;

      // Fermeture
      Close_window();
      Unselect_button(BUTTON_TEXT);

      // On passe en brosse:
      Display_cursor();
      if (antialias || !TrueType_font(selected_font_index))
        Change_paintbrush_shape(PAINTBRUSH_SHAPE_COLOR_BRUSH);
      else
        Change_paintbrush_shape(PAINTBRUSH_SHAPE_MONO_BRUSH);
      // Activate alpha mode
      if (antialias && TrueType_font(selected_font_index))
      {
        Shade_mode=0;
        Quick_shade_mode=0;
        Smooth_mode=0;
        Tiling_mode=0;
        Smear_mode=0;
        Colorize_mode=1;
        Colorize_current_mode=3;
        Effect_function=Effect_alpha_colorize;

        Draw_menu_button(BUTTON_EFFECTS,BUTTON_PRESSED);
      }

      Select_button(BUTTON_DRAW,LEFT_SIDE);
      if (Config.Auto_discontinuous)
      {
        // On se place en mode Dessin discontinu à la main
        while (Current_operation!=OPERATION_DISCONTINUOUS_DRAW)
          Select_button(BUTTON_DRAW,RIGHT_SIDE);
      }
      //Display_cursor();
      return;

      case 12: // Cancel
      // Save the selector settings
      list_start = font_list->List_start;
      cursor_position = font_list->Cursor_position;

      free(new_brush);
      new_brush = NULL;
      Close_window();
      Unselect_button(BUTTON_TEXT);
      Display_cursor();
      return;
    }
  }
}

void Display_stored_brush_in_window(word x_pos,word y_pos,int index)
{
  if (Brush_container[index].Paintbrush_shape < PAINTBRUSH_SHAPE_MAX)
  {
    int x,y;
    int offset_x=0, offset_y=0;
    //int brush_offset_x=0, brush_offset_y=0;

    // Determine draw offset (small brushes are stacked on corner of their preview)
    if (Brush_container[index].Width<BRUSH_CONTAINER_PREVIEW_WIDTH)
      offset_x = (BRUSH_CONTAINER_PREVIEW_WIDTH-Brush_container[index].Width)/2;
    if (Brush_container[index].Height<BRUSH_CONTAINER_PREVIEW_HEIGHT)
      offset_y = (BRUSH_CONTAINER_PREVIEW_HEIGHT-Brush_container[index].Height)/2;
    // Determine corner pixel of paintbrush to draw (if bigger than preview area)
    //

    // Clear
    Window_rectangle(x_pos,y_pos,BRUSH_CONTAINER_PREVIEW_WIDTH,BRUSH_CONTAINER_PREVIEW_HEIGHT,MC_Light);

    // Draw up to 16x16
    for (y=0; y<Brush_container[index].Height && y<BRUSH_CONTAINER_PREVIEW_HEIGHT; y++)
    {
      for (x=0; x<Brush_container[index].Width && x<BRUSH_CONTAINER_PREVIEW_WIDTH; x++)
      {
        byte color;
        if (Brush_container[index].Paintbrush_shape <= PAINTBRUSH_SHAPE_MISC)
          color = Brush_container[index].Thumbnail[y][x]?MC_Black:MC_Light;
        else
          color = Brush_container[index].Colormap[Brush_container[index].Thumbnail[y][x]];
        Pixel_in_window(x_pos+x+offset_x,y_pos+y+offset_y,color);
      }
    }
    Update_window_area(x_pos,y_pos,BRUSH_CONTAINER_PREVIEW_WIDTH,BRUSH_CONTAINER_PREVIEW_HEIGHT);

  }
}

/// Store the current brush in brush container
void Store_brush(int index)
{
  if (Brush_container[index].Paintbrush_shape < PAINTBRUSH_SHAPE_MAX)
  {
    // Free previous stored brush
    Brush_container[index].Paintbrush_shape = PAINTBRUSH_SHAPE_MAX;
    free(Brush_container[index].Brush);
    Brush_container[index].Brush = NULL;
  }

  // Store a mono brush
  if (Paintbrush_shape <= PAINTBRUSH_SHAPE_MISC)
  {
    int x,y;
    int brush_offset_x=0, brush_offset_y=0;

    Brush_container[index].Paintbrush_shape=Paintbrush_shape;
    Brush_container[index].Width=Paintbrush_width;
    Brush_container[index].Height=Paintbrush_height;
    //memcpy(Brush_container[index].Palette,Main.palette,sizeof(T_Palette));
    // Preview: pick center for big mono brush
    if (Paintbrush_width>BRUSH_CONTAINER_PREVIEW_WIDTH)
      brush_offset_x = (Paintbrush_width-BRUSH_CONTAINER_PREVIEW_WIDTH)/2;
    if (Paintbrush_height>BRUSH_CONTAINER_PREVIEW_HEIGHT)
      brush_offset_y = (Paintbrush_height-BRUSH_CONTAINER_PREVIEW_HEIGHT)/2;

    for (y=0; y<BRUSH_CONTAINER_PREVIEW_HEIGHT && y<Paintbrush_height; y++)
      for (x=0; x<BRUSH_CONTAINER_PREVIEW_WIDTH && x<Paintbrush_width; x++)
        Brush_container[index].Thumbnail[y][x]=Paintbrush_sprite[((y+brush_offset_y)*MAX_PAINTBRUSH_SIZE)+x+brush_offset_x];
    // Re-init the rest
    Brush_container[index].Transp_color=0;
  }
  else if (Paintbrush_shape == PAINTBRUSH_SHAPE_MONO_BRUSH &&
    Brush_width <= BRUSH_CONTAINER_PREVIEW_WIDTH &&
    Brush_height <= BRUSH_CONTAINER_PREVIEW_HEIGHT)
  {
    // Color brush transformed into a real mono paintbrush
    int x,y;

    Brush_container[index].Paintbrush_shape=PAINTBRUSH_SHAPE_MISC;
    Brush_container[index].Width=Brush_width;
    Brush_container[index].Height=Brush_height;
    // Preview: pick center for big mono brush
    for (y=0; y<BRUSH_CONTAINER_PREVIEW_HEIGHT && y<Brush_height; y++)
      for (x=0; x<BRUSH_CONTAINER_PREVIEW_WIDTH && x<Brush_width; x++)
        Brush_container[index].Thumbnail[y][x]=(Brush[y*Brush_width+x]!=Back_color);
    // Re-init the rest
    Brush_container[index].Transp_color=0;
  }
  else if (Paintbrush_shape == PAINTBRUSH_SHAPE_COLOR_BRUSH ||
     Paintbrush_shape == PAINTBRUSH_SHAPE_MONO_BRUSH)
  {
    // Color brush : saved bitmap and palette
    byte * buffer;
    buffer=(byte *)malloc(Brush_width*Brush_height);
    if (buffer)
    {
      Brush_container[index].Brush=buffer;
      Brush_container[index].Paintbrush_shape=Paintbrush_shape;
      Brush_container[index].Width=Brush_width;
      Brush_container[index].Height=Brush_height;

      memcpy(Brush_container[index].Brush, Brush_original_pixels,Brush_height*Brush_width);
      memcpy(Brush_container[index].Palette, Brush_original_palette,sizeof(T_Palette));
      memcpy(Brush_container[index].Colormap, Brush_colormap,256);

      // Scale for preview
      if (Brush_width>BRUSH_CONTAINER_PREVIEW_WIDTH ||
          Brush_height>BRUSH_CONTAINER_PREVIEW_HEIGHT)
      {
        // Scale
        Rescale(Brush_original_pixels, Brush_width, Brush_height, (byte *)(Brush_container[index].Thumbnail), BRUSH_CONTAINER_PREVIEW_WIDTH, BRUSH_CONTAINER_PREVIEW_HEIGHT, 0, 0);
      }
      else
      {
        // Direct copy
        Copy_part_of_image_to_another(Brush_original_pixels, 0,0,Brush_width, Brush_height,Brush_width,(byte *)(Brush_container[index].Thumbnail),0,0,BRUSH_CONTAINER_PREVIEW_WIDTH);
      }
    }
    else
    {
      Error(0);
    }
  }
}

/// Retrieve a normal paintbrush
void Select_paintbrush(int index)
{
  int x_pos,y_pos;

  Paintbrush_shape=Paintbrush[index].Shape;

  if (Paintbrush[index].Width<=PAINTBRUSH_WIDTH &&
    Paintbrush[index].Height<=PAINTBRUSH_HEIGHT)
  {
    Paintbrush_width=Paintbrush[index].Width;
    Paintbrush_height=Paintbrush[index].Height;
    Paintbrush_offset_X=Paintbrush[index].Offset_X;
    Paintbrush_offset_Y=Paintbrush[index].Offset_Y;

    for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
      for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
        Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=Paintbrush[index].Sprite[y_pos][x_pos];
  }
  else
  {
    // Too big to read from the preview: need re-generate it
    Set_paintbrush_size(Paintbrush[index].Width,Paintbrush[index].Height);
  }
  Change_paintbrush_shape(Paintbrush[index].Shape);
}

/// Store the current brush in paintbrush slot, if possible.
byte Store_paintbrush(int index)
{
  // Store a mono brush
  if (Paintbrush_shape <= PAINTBRUSH_SHAPE_MISC)
  {
    int x_pos,y_pos, x_off=0, y_off=0;

    Paintbrush[index].Shape=Paintbrush_shape;
    Paintbrush[index].Width=Paintbrush_width;
    Paintbrush[index].Height=Paintbrush_height;
    Paintbrush[index].Offset_X=Paintbrush_offset_X;
    Paintbrush[index].Offset_Y=Paintbrush_offset_Y;

    if (Paintbrush_width>PAINTBRUSH_WIDTH)
      x_off=(Paintbrush_width-PAINTBRUSH_WIDTH)/2;
    if (Paintbrush_height>PAINTBRUSH_HEIGHT)
      y_off=(Paintbrush_height-PAINTBRUSH_HEIGHT)/2;

    for (y_pos=0; y_pos<Paintbrush_height && y_pos<PAINTBRUSH_HEIGHT; y_pos++)
      for (x_pos=0; x_pos<Paintbrush_width && x_pos<PAINTBRUSH_WIDTH; x_pos++)
        Paintbrush[index].Sprite[y_pos][x_pos]=Paintbrush_sprite[((y_pos+y_off)*MAX_PAINTBRUSH_SIZE)+(x_pos+x_off)];

    return 0;
  }

  else if ((Paintbrush_shape == PAINTBRUSH_SHAPE_MONO_BRUSH ||
    Paintbrush_shape == PAINTBRUSH_SHAPE_COLOR_BRUSH))
  {
    // Color brush transformed into a real mono paintbrush
    int x_pos,y_pos;

    Paintbrush[index].Shape=PAINTBRUSH_SHAPE_MISC;
    Paintbrush[index].Width=Min(Brush_width,PAINTBRUSH_WIDTH);
    Paintbrush[index].Height=Min(Brush_height,PAINTBRUSH_HEIGHT);
    Paintbrush[index].Offset_X=Brush_offset_X*Paintbrush[index].Width/Brush_width;
    Paintbrush[index].Offset_Y=Brush_offset_Y*Paintbrush[index].Height/Brush_height;

    for (y_pos=0; y_pos<Brush_height&&y_pos<PAINTBRUSH_HEIGHT; y_pos++)
      for (x_pos=0; x_pos<Brush_width&&x_pos<PAINTBRUSH_WIDTH; x_pos++)
        Paintbrush[index].Sprite[y_pos][x_pos]=Brush[(y_pos*Brush_width)+x_pos]!=Back_color;

    return 0;
  }
  // Can't store it
  return 1;
}

byte Restore_brush(int index)
{
  byte shape;
  word x_pos;
  word y_pos;

  shape = Brush_container[index].Paintbrush_shape;

  if (shape == PAINTBRUSH_SHAPE_MAX)
    return 0;
  // Mono brushes
  if (shape <= PAINTBRUSH_SHAPE_MISC)
  {
    Paintbrush_shape=shape;
    Paintbrush_width=Brush_container[index].Width;
    Paintbrush_height=Brush_container[index].Height;
    if (shape == PAINTBRUSH_SHAPE_HORIZONTAL_BAR)
      Paintbrush_height=1;
    else if (shape == PAINTBRUSH_SHAPE_VERTICAL_BAR)
      Paintbrush_width=1;

    if (Paintbrush_width <= BRUSH_CONTAINER_PREVIEW_WIDTH &&
        Paintbrush_height <= BRUSH_CONTAINER_PREVIEW_HEIGHT)
    {
      // Manually copy the "pixels"
      for (y_pos=0; y_pos<Paintbrush_height; y_pos++)
        for (x_pos=0; x_pos<Paintbrush_width; x_pos++)
          Paintbrush_sprite[(y_pos*MAX_PAINTBRUSH_SIZE)+x_pos]=Brush_container[index].Thumbnail[y_pos][x_pos];

      Paintbrush_offset_X=Paintbrush_width>>1;
      Paintbrush_offset_Y=Paintbrush_height>>1;
    }
    else
    {
      // Recreate the brush pixels from its shape and dimensions
      Set_paintbrush_size(Paintbrush_width,Paintbrush_height);
    }
  }
  // Color brushes
  if (shape == PAINTBRUSH_SHAPE_COLOR_BRUSH ||
     shape == PAINTBRUSH_SHAPE_MONO_BRUSH)
  {
    Paintbrush_shape=shape;
    if (!Realloc_brush(Brush_container[index].Width,Brush_container[index].Height,NULL,NULL))
    {
      // Recover pixels
      memcpy(Brush_original_pixels, Brush_container[index].Brush, (long)Brush_height*Brush_width);
      // Grab palette
      memcpy(Brush_original_palette, Brush_container[index].Palette, sizeof(T_Palette));
      // Recover colormap
      memcpy(Brush_colormap, Brush_container[index].Colormap, 256);
      // Remap using current colormap
      Remap_general_lowlevel(Brush_colormap,Brush_original_pixels,Brush,Brush_width,Brush_height,Brush_width);

      Brush_offset_X=Brush_width>>1;
      Brush_offset_Y=Brush_height>>1;
    }

  }
  Change_paintbrush_shape(shape);

  return 1;
}

void Button_Brush_container(void)
{
  short clicked_button;
  short x_pos,y_pos;
  byte index;

  Open_window(BRUSH_CONTAINER_COLUMNS*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)+8,
    BRUSH_CONTAINER_ROWS*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+40,
    "Brushes");

  Window_set_normal_button(
    (BRUSH_CONTAINER_COLUMNS*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)-59)/2,
    (BRUSH_CONTAINER_ROWS)*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+18,
    67,14,"Cancel",0,1,KEY_ESC); // 1

  index=0;
  for (index=0; index < BRUSH_CONTAINER_ROWS*BRUSH_CONTAINER_COLUMNS; index++)
  {
    x_pos = (index % BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)+7;
    y_pos = (index / BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+18;
    Window_set_normal_button(
      x_pos,
      y_pos,
      BRUSH_CONTAINER_PREVIEW_WIDTH+2,
      BRUSH_CONTAINER_PREVIEW_HEIGHT+2,
      "",0,1,KEY_NONE
      );
    Display_stored_brush_in_window(x_pos+1, y_pos+1, index);
  }
  Update_window_area(0,0,Window_width, Window_height);

  Display_cursor();

  do
  {
    clicked_button=Window_clicked_button();
    //if (Is_shortcut(Key,0x100+BUTTON_HELP))
    //  Window_help(BUTTON_PAINTBRUSHES, NULL);

    if (clicked_button == 1)
      break;

    if (clicked_button>1)
    {
      index = clicked_button-2;

      if (Window_attribute1==RIGHT_SIDE)
      {
        // Store

        x_pos = (index % BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_WIDTH+8)+7;
        y_pos = (index / BRUSH_CONTAINER_COLUMNS)*(BRUSH_CONTAINER_PREVIEW_HEIGHT+8)+18;

        Store_brush(index);
        Hide_cursor();
        Display_stored_brush_in_window(x_pos+1, y_pos+1, index);
        Display_cursor();
      }
      else
      {
        // Restore and exit

        if (Restore_brush(index))
          break;
      }
    }
  }
  while (1);
  Close_window();

  //Unselect_button(BUTTON_PAINTBRUSHES);
  Display_cursor();
}

byte Any_effect_active(void)
{
    return Shade_mode||Quick_shade_mode||Colorize_mode||Smooth_mode||Tiling_mode||Smear_mode
      ||Stencil_mode||Mask_mode||Sieve_mode||Snap_mode||Main.tilemap_mode || (Main.backups->Pages->Image_mode > IMAGE_MODE_ANIMATION);
}
