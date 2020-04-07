/* vim:expandtab:ts=2 sw=2:
*/
/*  Grafx2 - The Ultimate 256-color bitmap paint program

    Copyright 2018 Thomas Bernard
    Copyright 2008 Franck Charlet
    Copyright 2007-2017 Adrien Destugues
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

********************************************************************************

    Drawing functions and effects.

*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "global.h"
#include "struct.h"
#include "engine.h"
#include "buttons.h"
#include "pages.h"
#include "errors.h"
#include "screen.h"
#include "graph.h"
#include "misc.h"
#include "pxsimple.h"
#include "pxtall.h"
#include "pxwide.h"
#include "pxdouble.h"
#include "pxtriple.h"
#include "pxwide2.h"
#include "pxtall2.h"
#include "pxtall3.h"
#include "pxquad.h"
#include "windows.h"
#include "input.h"
#include "brush.h"
#include "tiles.h"
#if defined(USE_SDL) || defined(USE_SDL2)
#include "sdlscreen.h"
#endif

#ifndef M_PI
    #define M_PI 3.141592653589793238462643
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

// Generic pixel-drawing function.
static Func_pixel Pixel_figure;

void Set_Pixel_figure(Func_pixel func)
{
  Pixel_figure = func;
}


typedef struct
{
  long  vertical_radius_squared;
  long  horizontal_radius_squared;
  qword limit;
} T_Ellipse_limits;

// Calcule les valeurs suivantes en fonction des deux paramètres:
//
// Ellipse_vertical_radius_squared
// Ellipse_horizontal_radius_squared
// Ellipse_Limit_High
// Ellipse_Limit_Low
static void Ellipse_compute_limites(short horizontal_radius,short vertical_radius, T_Ellipse_limits * Ellipse)
{
  Ellipse->horizontal_radius_squared = (long)horizontal_radius * horizontal_radius;
  Ellipse->vertical_radius_squared =   (long)vertical_radius * vertical_radius;
  Ellipse->limit = (qword)Ellipse->horizontal_radius_squared * Ellipse->vertical_radius_squared;
}

//   Indique si le pixel se trouvant à Ellipse_cursor_X pixels
// (Ellipse_cursor_X>0 = à droite, Ellipse_cursor_X<0 = à gauche) et à
// Ellipse_cursor_Y pixels (Ellipse_cursor_Y>0 = en bas,
// Ellipse_cursor_Y<0 = en haut) du centre se trouve dans l'ellipse en
// cours.
static byte Pixel_in_ellipse(long x, long y, const T_Ellipse_limits * Ellipse)
{
  qword ediesi = (qword)x * x * Ellipse->vertical_radius_squared +
    (qword)y * y * Ellipse->horizontal_radius_squared;
  if((ediesi) <= Ellipse->limit) return 255;

  return 0;
}

//   Indique si le pixel se trouvant à Circle_cursor_X pixels
// (Circle_cursor_X>0 = à droite, Circle_cursor_X<0 = à gauche) et à
// Circle_cursor_Y pixels (Circle_cursor_Y>0 = en bas,
// Circle_cursor_Y<0 = en haut) du centre se trouve dans le cercle en
// cours.
static byte Pixel_in_circle(long x, long y, long limit)
{
  if((x * x + y * y) <= limit)
    return 255;
  return 0;
}

/** Update the picture on screen, for the area passed in parameters.
 *
 * Takes into account the X/Y scrolling and zoom, and performs all safety checks so no updates will
 * go outside the display area.
 */
void Update_part_of_screen(short x, short y, short width, short height)
{
  short effective_w, effective_h;
  short effective_X;
  short effective_Y;
  short diff;

  // First make sure the zone is in forward direction (positive width/height)
  if (width < 0)
  {
    x += width;
    width = - width;
  }

  if (height < 0)
  {
    y += height;
    height = - height;
  }

  // Round up to a multiple of 8 pixels, because some special modes (ZX, Thomson, ...) can change
  // more pixels than expected (attribute clash)
  width = ((width + (x & 7)) | 7) + 1;
  height = ((height + (y & 7)) | 7) + 1;
  x &= 0xFFF8;
  y &= 0xFFF8;

  // Update "normal" view
  diff = x-Main.offset_X;
  if (diff<0)
  {
    effective_w = width + diff;
    effective_X = 0;
  }
  else
  {
    effective_w = width;
    effective_X = diff;
  }
  diff = y-Main.offset_Y;
  if (diff<0)
  {
    effective_h = height + diff;
    effective_Y = 0;
  }
  else
  {
    effective_h = height;
    effective_Y = diff;
  }

  // Clamp to actually visible area. All tools are normally constrained to this, but there are some
  // exceptions:
  // - Brush preview requests updates outside the visible screen,
  // - ZX/Thomson constraints can lead to pixel changes outside the visible area.
  if(Main.magnifier_mode && effective_X + effective_w > Main.separator_position)
    effective_w = Main.separator_position - effective_X;
  else if(effective_X + effective_w > Screen_width)
    effective_w = Screen_width - effective_X;

  if(effective_Y + effective_h > Menu_Y)
    effective_h = Menu_Y - effective_Y;

  /* (for debug purposes, highlight the rectangle that is updated)
  SDL_Rect r;
  r.x=effective_X;
  r.y=effective_Y;
  r.h=effective_h;
  r.w=effective_w;
  SDL_FillRect(Screen_SDL,&r,3);
  */
  Update_rect(effective_X,effective_Y,effective_w,effective_h);

  // Now update the "zoomed" part of the display
  if(Main.magnifier_mode)
  {
    // Convert picture to zoomed-screen coordinates
    effective_X = (x-Main.magnifier_offset_X)*Main.magnifier_factor;
    effective_Y = (y-Main.magnifier_offset_Y)*Main.magnifier_factor;
    effective_w = width * Main.magnifier_factor;
    effective_h = height * Main.magnifier_factor;

    // Apply horizontal clipping
    if (effective_X < 0)
    {
      effective_w+=effective_X;
      if (effective_w<0)
        return;

      effective_X = Main.separator_position + SEPARATOR_WIDTH*Menu_factor_X;
    }
    else
      effective_X += Main.separator_position + SEPARATOR_WIDTH*Menu_factor_X;
    diff = effective_X+effective_w-Min(Screen_width, Main.X_zoom+(Main.image_width-Main.magnifier_offset_X)*Main.magnifier_factor);
    if (diff>0)
    {
      effective_w -=diff;
      if (effective_w<0)
        return;
    }


    // Vertical clipping
    if (effective_Y < 0)
    {
      effective_h+=effective_Y;
      if (effective_h<0)
        return;
      effective_Y = 0;
    }
    diff = effective_Y+effective_h-Min(Menu_Y, (Main.image_height-Main.magnifier_offset_Y)*Main.magnifier_factor);
    if (diff>0)
    {
      effective_h -=diff;
      if (effective_h<0)
        return;
    }


 // Again, for debugging purposes, display the touched rectangle
    /*SDL_Rect r;
    r.x=effective_X;
    r.y=effective_Y;
    r.h=effective_h;
    r.w=effective_w;
    SDL_FillRect(Screen_SDL,&r,3);*/

    // When the grid is displayed in Tilemap mode, this tests if
    // one edge of the grid has been touched :
    // In this case, the whole magnified area requires a refreshed grid.
    // This could be optimized further, but at the moment this seemed
    // fast enough.
    if (Show_grid && Main.tilemap_mode && (
        x/Snap_width <(x+width )/Snap_width ||
        y/Snap_height<(y+height)/Snap_height))
    {
      short w,h;

      w=Min(Screen_width-Main.X_zoom, (Main.image_width-Main.magnifier_offset_X)*Main.magnifier_factor);
      h=Min(Menu_Y, (Main.image_height-Main.magnifier_offset_Y)*Main.magnifier_factor);

      Redraw_grid(Main.X_zoom,0,w,h);
      Update_rect(Main.X_zoom,0,w,h);
    }
    else
    {
      Redraw_grid(effective_X,effective_Y,effective_w,effective_h);
      Update_rect(effective_X,effective_Y,effective_w,effective_h);
    }
  }
}



void Transform_point(short x, short y, float cos_a, float sin_a,
                       short * rx, short * ry)
{
  *rx=Round(((float)x*cos_a)+((float)y*sin_a));
  *ry=Round(((float)y*cos_a)-((float)x*sin_a));
}


//--------------------- Initialisation d'un mode vidéo -----------------------

int Init_mode_video(int width, int height, int fullscreen, int pix_ratio)
{
  int index;
  int factor;
  int pix_width;
  int pix_height;
  byte screen_changed;
  byte pixels_changed;
  int absolute_mouse_x=Mouse_X*Pixel_width;
  int absolute_mouse_y=Mouse_Y*Pixel_height;
  static int Wrong_resize;

try_again:

  GFX2_Log(GFX2_DEBUG, "Init_mode_video(%d, %d, %d, %d)\n", width, height, fullscreen, pix_ratio);
  switch (pix_ratio)
  {
      default:
      case PIXEL_SIMPLE:
          pix_width=1;
          pix_height=1;
      break;
      case PIXEL_TALL:
          pix_width=1;
          pix_height=2;
      break;
      case PIXEL_WIDE:
          pix_width=2;
          pix_height=1;
      break;
      case PIXEL_DOUBLE:
          pix_width=2;
          pix_height=2;
      break;
      case PIXEL_TRIPLE:
          pix_width=3;
          pix_height=3;
      break;
      case PIXEL_WIDE2:
          pix_width=4;
          pix_height=2;
      break;
      case PIXEL_TALL2:
          pix_width=2;
          pix_height=4;
      break;
      case PIXEL_TALL3:
          pix_width=3;
          pix_height=4;
      break;
      case PIXEL_QUAD:
          pix_width=4;
          pix_height=4;
      break;
  }

  screen_changed = (Screen_width*Pixel_width!=width ||
                    Screen_height*Pixel_height!=height ||
                    Video_mode[Current_resolution].Fullscreen != fullscreen);

  // Valeurs raisonnables: minimum 320x200
  if (!fullscreen)
  {
    if (Wrong_resize>20 && (width < 320*pix_width || height < 200*pix_height))
    {
      if(pix_ratio != PIXEL_SIMPLE) {
        pix_ratio = PIXEL_SIMPLE;
        Verbose_message("Error!", "Your WM is forcing GrafX2 to resize to something "
          "smaller than the minimal resolution.\n"
          "GrafX2 switched to a smaller\npixel scaler to avoid problems                     ");
        goto try_again;
      }
    }

    if (width > 320*pix_width && height > 200*pix_height)
      Wrong_resize = 0;

    if (width < 320*pix_width)
    {
        width = 320*pix_width;
        screen_changed=1;
        Wrong_resize++;
    }
    if (height < 200*pix_height)
    {
        height = 200*pix_height;
        screen_changed=1;
        Wrong_resize++;
    }
    Video_mode[0].Width = width;
    Video_mode[0].Height = height;

  }
  else
  {
    if (width < 320*pix_width || height < 200*pix_height)
      return 1;
  }
  // La largeur doit être un multiple de 4
  #ifdef __amigaos4__
      // On AmigaOS the systems adds some more constraints on that ...
      width = (width + 15) & 0xFFFFFFF0;
  #else
      //width = (width + 3 ) & 0xFFFFFFFC;
  #endif

  pixels_changed = (Pixel_ratio!=pix_ratio);

  if (!screen_changed && !pixels_changed)
  {
    Resize_width=0;
    Resize_height=0;
    return 0;
  }

  if (screen_changed)
  {
    GFX2_Set_mode(&width, &height, fullscreen);
  }

  if (screen_changed || pixels_changed)
  {
    Pixel_ratio=pix_ratio;
    Pixel_width=pix_width;
    Pixel_height=pix_height;
    switch (Pixel_ratio)
    {
        default:
        case PIXEL_SIMPLE:
#define Display_line_on_screen_fast_simple Display_line_on_screen_simple
#define SETPIXEL(x) \
            Pixel = Pixel_##x ; \
            Read_pixel= Read_pixel_##x ; \
            Display_screen = Display_part_of_screen_##x ; \
            Block = Block_##x ; \
            Pixel_preview_normal = Pixel_preview_normal_##x ; \
            Pixel_preview_magnifier = Pixel_preview_magnifier_##x ; \
            Horizontal_XOR_line = Horizontal_XOR_line_##x ; \
            Vertical_XOR_line = Vertical_XOR_line_##x ; \
            Display_brush_color = Display_brush_color_##x ; \
            Display_brush_mono = Display_brush_mono_##x ; \
            Clear_brush = Clear_brush_##x ; \
            Remap_screen = Remap_screen_##x ; \
            Display_line = Display_line_on_screen_##x ; \
            Display_line_fast = Display_line_on_screen_fast_##x ; \
            Read_line = Read_line_screen_##x ; \
            Display_zoomed_screen = Display_part_of_screen_scaled_##x ; \
            Display_brush_color_zoom = Display_brush_color_zoom_##x ; \
            Display_brush_mono_zoom = Display_brush_mono_zoom_##x ; \
            Clear_brush_scaled = Clear_brush_scaled_##x ; \
            Display_brush = Display_brush_##x ;
			SETPIXEL(simple)
        break;
        case PIXEL_TALL:
#define Display_line_on_screen_fast_tall Display_line_on_screen_tall
			SETPIXEL(tall)
        break;
        case PIXEL_WIDE:
			SETPIXEL(wide)
        break;
        case PIXEL_DOUBLE:
			SETPIXEL(double)
        break;
        case PIXEL_TRIPLE:
			SETPIXEL(triple)
        break;
        case PIXEL_WIDE2:
			SETPIXEL(wide2)
        break;
        case PIXEL_TALL2:
			SETPIXEL(tall2)
        break;
        case PIXEL_TALL3:
			SETPIXEL(tall3)
        break;
        case PIXEL_QUAD:
			SETPIXEL(quad)
        break;
    }
  }
  Screen_width = width/Pixel_width;
  Screen_height = height/Pixel_height;

  Clear_border(MC_Black); // Requires up-to-date Screen_* and Pixel_*

  // Set menu size (software zoom)
  if (Screen_width/320 > Screen_height/200)
    factor=Screen_height/200;
  else
    factor=Screen_width/320;

  switch (Config.Ratio)
  {
    case 1: // Always the biggest possible
      Menu_factor_X=factor;
      Menu_factor_Y=factor;
      break;
    case 2: // Only keep the aspect ratio
      Menu_factor_X=factor-1;
      if (Menu_factor_X<1) Menu_factor_X=1;
      Menu_factor_Y=factor-1;
      if (Menu_factor_Y<1) Menu_factor_Y=1;
      break;
    case 0: // Always smallest possible
      Menu_factor_X=1;
      Menu_factor_Y=1;
      break;
    default: // Stay below some reasonable size
      if (factor>Max(Pixel_width,Pixel_height))
        factor/=Max(Pixel_width,Pixel_height);
      Menu_factor_X=Min(factor,abs(Config.Ratio));
      Menu_factor_Y=Min(factor,abs(Config.Ratio));
  }
  if (Pixel_height>Pixel_width && Screen_width>=Menu_factor_X*2*320)
    Menu_factor_X*=2;
  else if (Pixel_width>Pixel_height && Screen_height>=Menu_factor_Y*2*200)
    Menu_factor_Y*=2;

  free(Horizontal_line_buffer);
  Horizontal_line_buffer=(byte *)malloc(Pixel_width *
    ((Screen_width>Main.image_width)?Screen_width:Main.image_width));

  Set_palette(Main.palette);

  Current_resolution=0;
  if (fullscreen)
  {
    for (index=1; index<Nb_video_modes; index++)
    {
      if (Video_mode[index].Width/Pixel_width==Screen_width &&
          Video_mode[index].Height/Pixel_height==Screen_height)
      {
        Current_resolution=index;
        break;
      }
    }
  }

  Change_palette_cells();

  Menu_Y = Screen_height;
  if (Menu_is_visible)
    Menu_Y -= Menu_height * Menu_factor_Y;
  Menu_status_Y = Screen_height-(Menu_factor_Y<<3);

  Adjust_mouse_sensitivity(fullscreen);

  Mouse_X=absolute_mouse_x/Pixel_width;
  if (Mouse_X>=Screen_width)
    Mouse_X=Screen_width-1;
  Mouse_Y=absolute_mouse_y/Pixel_height;
  if (Mouse_Y>=Screen_height)
    Mouse_Y=Screen_height-1;
  if (fullscreen)
    Set_mouse_position();

  Spare.offset_X=0; // |  Il faut penser à éviter les incohérences
  Spare.offset_Y=0; // |- de décalage du brouillon par rapport à
  Spare.magnifier_mode=0; // |  la résolution.

  if (Main.magnifier_mode)
  {
    Pixel_preview=Pixel_preview_magnifier;
  }
  else
  {
    Pixel_preview=Pixel_preview_normal;
    // Recaler la vue (meme clipping que dans Scroll_screen())
    if (Main.offset_X+Screen_width>Main.image_width)
      Main.offset_X=Main.image_width-Screen_width;
    if (Main.offset_X<0)
      Main.offset_X=0;
    if (Main.offset_Y+Menu_Y>Main.image_height)
      Main.offset_Y=Main.image_height-Menu_Y;
    if (Main.offset_Y<0)
      Main.offset_Y=0;
  }

  Compute_magnifier_data();
  if (Main.magnifier_mode)
    Position_screen_according_to_zoom();
  Compute_limits();
  Compute_paintbrush_coordinates();

  Resize_width=0;
  Resize_height=0;
  return 0;
}



  // -- Redimentionner l'image (nettoie l'écran virtuel) --

void Resize_image(word chosen_width,word chosen_height)
{
  word old_width=Main.image_width;
  word old_height=Main.image_height;
  int i;

  // +-+-+
  // |C| |  A+B+C = Ancienne image
  // +-+A|
  // |B| |    C   = Nouvelle image
  // +-+-+

  Upload_infos_page(&Main);
  if (Backup_with_new_dimensions(chosen_width,chosen_height))
  {
    // La nouvelle page a pu être allouée, elle est pour l'instant pleine de
    // 0s. Elle fait Main.image_width de large.

    Main.image_is_modified=1;

    // On copie donc maintenant la partie C dans la nouvelle image.
    for (i=0; i<Main.backups->Pages->Nb_layers; i++)
    {
      Copy_part_of_image_to_another(
        Main.backups->Pages->Next->Image[i].Pixels,0,0,Min(old_width,Main.image_width),
        Min(old_height,Main.image_height),old_width,
        Main.backups->Pages->Image[i].Pixels,0,0,Main.image_width);
    }
    Redraw_layered_image();
  }
  else
  {
    // Afficher un message d'erreur
    Display_cursor();
    Message_out_of_memory();
    Hide_cursor();
  }
}



void Remap_spare(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  used[256]; // Tableau de booléens "La couleur est utilisée"
  int   color;
  int   layer;

  // On commence par initialiser le tableau de booléens à faux
  for (color=0;color<=255;color++)
    used[color]=0;

  // On calcule la table d'utilisation des couleurs
  for (layer=0; layer<Spare.backups->Pages->Nb_layers; layer++)
    for (y_pos=0;y_pos<Spare.image_height;y_pos++)
      for (x_pos=0;x_pos<Spare.image_width;x_pos++)
        used[*(Spare.backups->Pages->Image[layer].Pixels+(y_pos*Spare.image_width+x_pos))]=1;

  //   On va maintenant se servir de la table "used" comme table de
  // conversion: pour chaque indice, la table donne une couleur de
  // remplacement.
  // Note : Seules les couleurs utilisées on besoin d'êtres recalculées: les
  //       autres ne seront jamais consultées dans la nouvelle table de
  //       conversion puisque elles n'existent pas dans l'image, donc elles
  //       ne seront pas utilisées par Remap_general_lowlevel.
  for (color=0;color<=255;color++)
    if (used[color])
      used[color]=Best_color_perceptual(Spare.palette[color].R,Spare.palette[color].G,Spare.palette[color].B);

  //   Maintenant qu'on a une super table de conversion qui n'a que le nom
  // qui craint un peu, on peut faire l'échange dans la brosse de toutes les
  // teintes.
  for (layer=0; layer<Spare.backups->Pages->Nb_layers; layer++)
    Remap_general_lowlevel(used,Spare.backups->Pages->Image[layer].Pixels,Spare.backups->Pages->Image[layer].Pixels,Spare.image_width,Spare.image_height,Spare.image_width);

  // Change transparent color index
  Spare.backups->Pages->Transparent_color=used[Spare.backups->Pages->Transparent_color];
}



void Get_colors_from_brush(void)
{
  short x_pos; // Variable de balayage de la brosse
  short y_pos; // Variable de balayage de la brosse
  byte  brush_used[256]; // Tableau de booléens "La couleur est utilisée"
  dword usage[256];
  int   color;
  int   image_color;

  //if (Confirmation_box("Modify current palette ?"))

  // Backup with unchanged layers, only palette is modified
  Backup_layers(LAYER_NONE);

  // Init array of new colors
  for (color=0;color<=255;color++)
    brush_used[color]=0;

  // Tag used colors
  for (y_pos=0;y_pos<Brush_height;y_pos++)
    for (x_pos=0;x_pos<Brush_width;x_pos++)
      brush_used[*(Brush_original_pixels + y_pos * Brush_width + x_pos)]=1;

  // Check used colors in picture (to know which palette entries are free)
  Count_used_colors(usage);

  // First pass : omit colors that are already in palette
  for (color=0; color<256; color++)
  {
    // For each color used in brush (to add in palette)
    if (brush_used[color])
    {
      // Try locate it in current palette
      for (image_color=0; image_color<256; image_color++)
      {
        if (Brush_original_palette[color].R==Main.palette[image_color].R
         && Brush_original_palette[color].G==Main.palette[image_color].G
         && Brush_original_palette[color].B==Main.palette[image_color].B)
        {
          // Color already in main palette:

          // Tag as used, so that no new color will overwrite it
          usage[image_color]=1;

          // Tag as non-new, to avoid it in pass 2
          brush_used[color]=0;

          break;
        }
      }
    }
  }

  // Second pass : For each color to add, find an empty slot in
  // main palette to add it
  image_color=0;
  for (color=0; color<256 && image_color<256; color++)
  {
    // For each color used in brush
    if (brush_used[color])
    {
      for (; image_color<256; image_color++)
      {
        if (!usage[image_color])
        {
          // Copy from color to image_color
          Main.palette[image_color].R=Brush_original_palette[color].R;
          Main.palette[image_color].G=Brush_original_palette[color].G;
          Main.palette[image_color].B=Brush_original_palette[color].B;

          image_color++;
          break;
        }
      }
    }
  }
  Remap_brush();

  Set_palette(Main.palette);
  Compute_optimal_menu_colors(Main.palette);
  Hide_cursor();
  Display_all_screen();
  Display_menu();
  Display_cursor();
  End_of_modification();

  Main.image_is_modified=1;
}



//////////////////////////////////////////////////////////////////////////////
////////////////////////////// GESTION DU FILLER /////////////////////////////
//////////////////////////////////////////////////////////////////////////////


void Fill(short * top_reached  , short * bottom_reached,
          short * left_reached, short * right_reached)
//
//   Cette fonction fait un remplissage classique d'une zone délimitée de
// l'image. Les limites employées sont Limit_top, Limit_bottom, Limit_left
// et Limit_right. Le point de départ du remplissage est Paintbrush_X,Paintbrush_Y
// et s'effectue en théorie sur la couleur 1 et emploie la couleur 2 pour le
// remplissage. Ces restrictions sont dûes à l'utilisation qu'on en fait dans
// la fonction principale "Fill_general", qui se charge de faire une gestion de
// tous les effets.
//   Cette fonction ne doit pas être directement appelée.
//
{
  short x_pos;   // Abscisse de balayage du segment, utilisée lors de l'"affichage"
  short line;   // Ordonnée de la ligne en cours de traitement
  short start_x; // Abscisse de départ du segment traité
  short end_x;   // Abscisse de fin du segment traité
  int   changes_made;    // Booléen "On a fait une modif dans le dernier passage"
  int   can_propagate; // Booléen "On peut propager la couleur dans le segment"
  short current_limit_bottom;  // Intervalle vertical restreint
  short current_limit_top;
  int   line_is_modified;       // Booléen "On a fait une modif dans la ligne"

  changes_made=1;
  current_limit_top=Paintbrush_Y;
  current_limit_bottom =Min(Paintbrush_Y+1,Limit_bottom);
  *left_reached=Paintbrush_X;
  *right_reached=Paintbrush_X+1;
  Pixel_in_current_layer(Paintbrush_X,Paintbrush_Y,2);

  while (changes_made)
  {
    changes_made=0;

    for (line=current_limit_top;line<=current_limit_bottom;line++)
    {
      line_is_modified=0;
      // On va traiter le cas de la ligne n° line.

      // On commence le traitement à la gauche de l'écran
      start_x=Limit_left;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (start_x<=Limit_right)
      {
        // On cherche son début
        while((start_x<=Limit_right) &&
                (Read_pixel_from_current_layer(start_x,line)!=1))
             start_x++;

        if (start_x<=Limit_right)
        {
          // Un segment de couleur 1 existe et commence à la position start_x.
          // On va donc en chercher la fin.
          for (end_x=start_x+1;(end_x<=Limit_right) &&
               (Read_pixel_from_current_layer(end_x,line)==1);end_x++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // start_x et qui se termine en end_x-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          can_propagate=(
            // Test de la présence d'un point à gauche du segment
            ((start_x>Limit_left) &&
             (Read_pixel_from_current_layer(start_x-1,line)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((end_x-1<Limit_right) &&
             (Read_pixel_from_current_layer(end_x    ,line)==2))
                               );

          // Test de la présence d'un point en haut du segment
          if (!can_propagate && (line>Limit_top))
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              if (Read_pixel_from_current_layer(x_pos,line-1)==2)
              {
                can_propagate=1;
                break;
              }

          if (can_propagate)
          {
            if (start_x<*left_reached)
              *left_reached=start_x;
            if (end_x>*right_reached)
              *right_reached=end_x;
            // On remplit le segment de start_x à end_x-1.
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              Pixel_in_current_layer(x_pos,line,2);
            // On vient d'effectuer des modifications.
            changes_made=1;
            line_is_modified=1;
          }

          start_x=end_x+1;
        }
      }

      // Si on est en bas, et qu'on peut se propager vers le bas...
      if ( (line==current_limit_bottom) &&
           (line_is_modified) &&
           (current_limit_bottom<Limit_bottom) )
        current_limit_bottom++; // On descend cette limite vers le bas
    }

    // Pour le prochain balayage vers le haut, on va se permettre d'aller
    // voir une ligne plus haut.
    // Si on ne le fait pas, et que la première ligne (current_limit_top)
    // n'était pas modifiée, alors cette limite ne serait pas remontée, donc
    // le filler ne progresserait pas vers le haut.
    if (current_limit_top>Limit_top)
      current_limit_top--;

    for (line=current_limit_bottom;line>=current_limit_top;line--)
    {
      line_is_modified=0;
      // On va traiter le cas de la ligne n° line.

      // On commence le traitement à la gauche de l'écran
      start_x=Limit_left;

      // Pour chaque segment de couleur 1 que peut contenir la ligne
      while (start_x<=Limit_right)
      {
        // On cherche son début
        for (;(start_x<=Limit_right) &&
             (Read_pixel_from_current_layer(start_x,line)!=1);start_x++);

        if (start_x<=Limit_right)
        {
          // Un segment de couleur 1 existe et commence à la position start_x.
          // On va donc en chercher la fin.
          for (end_x=start_x+1;(end_x<=Limit_right) &&
               (Read_pixel_from_current_layer(end_x,line)==1);end_x++);

          //   On sait qu'il existe un segment de couleur 1 qui commence en
          // start_x et qui se termine en end_x-1.

          //   On va maintenant regarder si une couleur sur la périphérie
          // permet de colorier ce segment avec la couleur 2.

          can_propagate=(
            // Test de la présence d'un point à gauche du segment
            ((start_x>Limit_left) &&
             (Read_pixel_from_current_layer(start_x-1,line)==2)) ||
            // Test de la présence d'un point à droite du segment
            ((end_x-1<Limit_right) &&
             (Read_pixel_from_current_layer(end_x    ,line)==2))
                               );

          // Test de la présence d'un point en bas du segment
          if (!can_propagate && (line<Limit_bottom))
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              if (Read_pixel_from_current_layer(x_pos,line+1)==2)
              {
                can_propagate=1;
                break;
              }

          if (can_propagate)
          {
            if (start_x<*left_reached)
              *left_reached=start_x;
            if (end_x>*right_reached)
              *right_reached=end_x;
            // On remplit le segment de start_x à end_x-1.
            for (x_pos=start_x;x_pos<end_x;x_pos++)
              Pixel_in_current_layer(x_pos,line,2);
            // On vient d'effectuer des modifications.
            changes_made=1;
            line_is_modified=1;
          }

          start_x=end_x+1;
        }
      }

      // Si on est en haut, et qu'on peut se propager vers le haut...
      if ( (line==current_limit_top) &&
           (line_is_modified) &&
           (current_limit_top>Limit_top) )
        current_limit_top--; // On monte cette limite vers le haut
    }
  }

  *top_reached=current_limit_top;
  *bottom_reached =current_limit_bottom;
  (*right_reached)--;
} // end de la routine de remplissage "Fill"

byte Read_pixel_from_backup_layer(word x,word y)
{
  return *((y)*Main.image_width+(x)+Main.backups->Pages->Next->Image[Main.current_layer].Pixels);
}

void Fill_general(byte fill_color)
//
//  Cette fonction fait un remplissage qui gère tous les effets. Elle fait
// appel à "Fill()".
//
{
  byte   cursor_shape_before_fill;
  short  x_pos,y_pos;
  short  top_reached  ,bottom_reached;
  short  left_reached,right_reached;
  byte   replace_table[256];
  int old_limit_right=Limit_right;
  int old_limit_left=Limit_left;
  int old_limit_top=Limit_top;
  int old_limit_bottom=Limit_bottom;


  // Avant toute chose, on vérifie que l'on n'est pas en train de remplir
  // en dehors de l'image:

  if ( (Paintbrush_X>=Limit_left) &&
       (Paintbrush_X<=Limit_right) &&
       (Paintbrush_Y>=Limit_top)   &&
       (Paintbrush_Y<=Limit_bottom) )
  {
    // If tilemap mode is ON, ignore action if it's outside grid limits
    if (Main.tilemap_mode)
    {
      if (Paintbrush_X<Snap_offset_X)
        return;
      if (Paintbrush_X >= (Main.image_width-Snap_offset_X)/Snap_width*Snap_width+Snap_offset_X)
        return;
      if (Paintbrush_Y<Snap_offset_Y)
        return;
      if (Paintbrush_Y >= (Main.image_height-Snap_offset_Y)/Snap_height*Snap_height+Snap_offset_Y)
        return;
    }

    // On suppose que le curseur est déjà caché.
    // Hide_cursor();

    //   On va faire patienter l'utilisateur en lui affichant un joli petit
    // sablier:
    cursor_shape_before_fill=Cursor_shape;
    Cursor_shape=CURSOR_SHAPE_HOURGLASS;
    Display_cursor();

    // On commence par effectuer un backup de l'image.
    Backup();

    // On fait attention au Feedback qui DOIT se faire avec le backup.
    Update_FX_feedback(0);

    // If tilemap mode is ON, adapt limits to current tile only
    if (Main.tilemap_mode)
    {
      Limit_right = Min(Limit_right, (Paintbrush_X-Snap_offset_X)/Snap_width*Snap_width+Snap_width-1+Snap_offset_X);
      Limit_left = Max(Limit_left, (Paintbrush_X-Snap_offset_X)/Snap_width*Snap_width+Snap_offset_X);
      Limit_bottom = Min(Limit_bottom, (Paintbrush_Y-Snap_offset_Y)/Snap_height*Snap_height+Snap_height-1+Snap_offset_Y);
      Limit_top = Max(Limit_top, (Paintbrush_Y-Snap_offset_Y)/Snap_height*Snap_height+Snap_offset_Y);
    }

    // On va maintenant "épurer" la zone visible de l'image:
    memset(replace_table,0,256);
    replace_table[Read_pixel_from_backup_layer(Paintbrush_X,Paintbrush_Y)]=1;
    Replace_colors_within_limits(replace_table);

    // On fait maintenant un remplissage classique de la couleur 1 avec la 2
    Fill(&top_reached  ,&bottom_reached,
         &left_reached,&right_reached);

    //  On s'apprête à faire des opérations qui nécessitent un affichage. Il
    // faut donc retirer de l'écran le curseur:
    Hide_cursor();
    Cursor_shape=cursor_shape_before_fill;

    //  Il va maintenant falloir qu'on "turn" ce gros caca "into" un truc qui
    // ressemble un peu plus à ce à quoi l'utilisateur peut s'attendre.
    if (top_reached>Limit_top)
      Copy_part_of_image_to_another(Main.backups->Pages->Next->Image[Main.current_layer].Pixels, // source
                                               Limit_left,Limit_top,       // Pos X et Y dans source
                                               (Limit_right-Limit_left)+1, // width copie
                                               top_reached-Limit_top,// height copie
                                               Main.image_width,         // width de la source
                                               Main.backups->Pages->Image[Main.current_layer].Pixels, // Destination
                                               Limit_left,Limit_top,       // Pos X et Y destination
                                               Main.image_width);        // width destination
    if (bottom_reached<Limit_bottom)
      Copy_part_of_image_to_another(Main.backups->Pages->Next->Image[Main.current_layer].Pixels,
                                               Limit_left,bottom_reached+1,
                                               (Limit_right-Limit_left)+1,
                                               Limit_bottom-bottom_reached,
                                               Main.image_width,Main.backups->Pages->Image[Main.current_layer].Pixels,
                                               Limit_left,bottom_reached+1,Main.image_width);
    if (left_reached>Limit_left)
      Copy_part_of_image_to_another(Main.backups->Pages->Next->Image[Main.current_layer].Pixels,
                                               Limit_left,top_reached,
                                               left_reached-Limit_left,
                                               (bottom_reached-top_reached)+1,
                                               Main.image_width,Main.backups->Pages->Image[Main.current_layer].Pixels,
                                               Limit_left,top_reached,Main.image_width);
    if (right_reached<Limit_right)
      Copy_part_of_image_to_another(Main.backups->Pages->Next->Image[Main.current_layer].Pixels,
                                               right_reached+1,top_reached,
                                               Limit_right-right_reached,
                                               (bottom_reached-top_reached)+1,
                                               Main.image_width,Main.backups->Pages->Image[Main.current_layer].Pixels,
                                               right_reached+1,top_reached,Main.image_width);

    // Restore image limits : this is needed by the tilemap effect,
    // otherwise it will not display other modified tiles.
    Limit_right=old_limit_right;
    Limit_left=old_limit_left;
    Limit_top=old_limit_top;
    Limit_bottom=old_limit_bottom;

    for (y_pos=top_reached;y_pos<=bottom_reached;y_pos++)
    {
      for (x_pos=left_reached;x_pos<=right_reached;x_pos++)
      {
        byte filled = Read_pixel_from_current_layer(x_pos,y_pos);

        // First, restore the color.
        Pixel_in_current_screen(x_pos,y_pos,Read_pixel_from_backup_layer(x_pos,y_pos));

        if (filled==2)
        {
          // Update the color according to the fill color and all effects
          Display_pixel(x_pos,y_pos,fill_color);
        }
      }
    }

    // Restore original feedback value
    Update_FX_feedback(Config.FX_Feedback);

    //   A la fin, on n'a pas besoin de réafficher le curseur puisque c'est
    // l'appelant qui s'en charge, et on n'a pas besoin de rafficher l'image
    // puisque les seuls points qui ont changé dans l'image ont été raffichés
    // par l'utilisation de "Display_pixel()", et que les autres... eh bein
    // on n'y a jamais touché à l'écran les autres: ils sont donc corrects.
    if(Main.magnifier_mode)
    {
      short w,h;

      w=Min(Screen_width-Main.X_zoom, (Main.image_width-Main.magnifier_offset_X)*Main.magnifier_factor);
      h=Min(Menu_Y, (Main.image_height-Main.magnifier_offset_Y)*Main.magnifier_factor);

      Redraw_grid(Main.X_zoom,0,w,h);
    }

    Update_rect(0,0,0,0);
    End_of_modification();
  }
}



//////////////////////////////////////////////////////////////////////////////
////////////////// TRACéS DE FIGURES GéOMéTRIQUES STANDARDS //////////////////
////////////////////////// avec gestion de previews //////////////////////////
//////////////////////////////////////////////////////////////////////////////

  // Data used by ::Init_permanent_draw() and ::Pixel_figure_permanent()
  static dword Permanent_draw_next_refresh=0;
  static int Permanent_draw_count=0;

  void Init_permanent_draw(void)
  {
    Permanent_draw_count = 0;
    Permanent_draw_next_refresh = GFX2_GetTicks() + 100;
  }

  // Affichage d'un point de façon définitive (utilisation du pinceau)
  void Pixel_figure_permanent(word x_pos,word y_pos,byte color)
  {
    Draw_paintbrush(x_pos,y_pos,color);
    Permanent_draw_count ++;

    // Check every 8 pixels
    if (! (Permanent_draw_count&7))
    {
      dword now = GFX2_GetTicks();
#if defined(USE_SDL) || defined(USE_SDL2)
      SDL_PumpEvents();
#endif
      if (now>= Permanent_draw_next_refresh)
      {
        Permanent_draw_next_refresh = now+100;
        Flush_update();
      }
    }
  }

  // Affichage d'un point de façon définitive
  void Pixel_clipped(word x_pos,word y_pos,byte color)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
    Display_pixel(x_pos,y_pos,color);
  }

  // Affichage d'un point pour une preview
  void Pixel_figure_preview(word x_pos,word y_pos,byte color)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,color);
  }
  // Affichage d'un point pour une preview, avec sa propre couleur
  void Pixel_figure_preview_auto(word x_pos,word y_pos)
  {
    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,Read_pixel_from_current_screen(x_pos,y_pos));
  }

  // Affichage d'un point pour une preview en xor
  void Pixel_figure_preview_xor(word x_pos,word y_pos,byte color)
  {
    (void)color; // unused

    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,xor_lut[Read_pixel(x_pos-Main.offset_X,
                                           y_pos-Main.offset_Y)]);
  }

  // Affichage d'un point pour une preview en xor additif
  // (Il lit la couleur depuis la page backup)
  void Pixel_figure_preview_xorback(word x_pos,word y_pos,byte color)
  {
    (void)color; // unused

    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,xor_lut[Main_screen[x_pos+y_pos*Main.image_width]]);
  }


  // Effacement d'un point de preview
  void Pixel_figure_clear_preview(word x_pos,word y_pos,byte color)
  {
    (void)color; // unused

    if ( (x_pos>=Limit_left) &&
         (x_pos<=Limit_right) &&
         (y_pos>=Limit_top)   &&
         (y_pos<=Limit_bottom) )
      Pixel_preview(x_pos,y_pos,Read_pixel_from_current_screen(x_pos,y_pos));
  }

  // Affichage d'un point dans la brosse
  void Pixel_figure_in_brush(word x_pos,word y_pos,byte color)
  {
    x_pos-=Brush_offset_X;
    y_pos-=Brush_offset_Y;
    if ( (x_pos<Brush_width) && // Les pos sont des word donc jamais < 0 ...
         (y_pos<Brush_height) )
      Pixel_in_brush(x_pos,y_pos,color);
  }


  // -- Tracer général d'un cercle vide -------------------------------------

void Draw_empty_circle_general(short center_x,short center_y, long sqradius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;
  long x, y;
  short radius;

  radius = sqrt(sqradius);

  // Ensuite, on va parcourire le quart haut gauche du cercle
  start_x=center_x-radius;
  start_y=center_y-radius;

  // Affichage des extremitées du cercle sur chaque quart du cercle:
  for (y_pos=start_y,y=-radius;y_pos<center_y;y_pos++,y++)
    for (x_pos=start_x,x=-radius;x_pos<center_x;x_pos++,x++)
      if (Pixel_in_circle(x, y, sqradius))
      {
        // On vient de tomber sur le premier point sur la ligne horizontale
        // qui fait partie du cercle.
        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(x_pos,y_pos,color);
         // Quart Haut-droite
        Pixel_figure((center_x<<1)-x_pos,y_pos,color);
         // Quart Bas-droite
        Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);
         // Quart Bas-gauche
        Pixel_figure(x_pos,(center_y<<1)-y_pos,color);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas au cercle:
        for (y--,x_pos++,x++;x_pos<center_x;x_pos++,x++)
          if (!Pixel_in_circle(x, y, sqradius))
          {
             // Quart Haut-gauche
            Pixel_figure(x_pos,y_pos,color);
             // Quart Haut-droite
            Pixel_figure((center_x<<1)-x_pos,y_pos,color);
             // Quart Bas-gauche
            Pixel_figure(x_pos,(center_y<<1)-y_pos,color);
             // Quart Bas-droite
            Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);
          }
          else
            break;

        y++;
        break;
      }

  // On affiche à la fin les points cardinaux:
  Pixel_figure(center_x,center_y-radius,color); // Haut
  Pixel_figure(center_x-radius,center_y,color); // Gauche
  Pixel_figure(center_x+radius,center_y,color); // Droite
  Pixel_figure(center_x,center_y+radius,color); // Bas

}

  // -- Tracé définitif d'un cercle vide --

void Draw_empty_circle_permanent(short center_x,short center_y,long sqradius,byte color)
{
  short radius = sqrt(sqradius);
  Pixel_figure=Pixel_figure_permanent;
  Init_permanent_draw();
  Draw_empty_circle_general(center_x,center_y,sqradius,color);
  Update_part_of_screen(center_x - radius, center_y - radius, 2* radius+1, 2*radius+1);
}

  // -- Tracer la preview d'un cercle vide --

void Draw_empty_circle_preview(short center_x,short center_y,long sqradius,byte color)
{
  short radius = sqrt(sqradius);
  Pixel_figure=Pixel_figure_preview;
  Draw_empty_circle_general(center_x,center_y,sqradius,color);
  Update_part_of_screen(center_x - radius, center_y - radius, 2* radius+1, 2*radius+1);
}

  // -- Effacer la preview d'un cercle vide --

void Hide_empty_circle_preview(short center_x,short center_y,long sqradius)
{
  short radius = sqrt(sqradius);
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_empty_circle_general(center_x,center_y,sqradius,0);
  Update_part_of_screen(center_x - radius, center_y - radius, 2* radius+1, 2*radius+1);
}

  // -- Tracer un cercle plein --

void Draw_filled_circle(short center_x,short center_y,long sqradius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;
  short end_x;
  short end_y;
  long x, y;
  short radius = sqrt(sqradius);

  start_x=center_x-radius;
  start_y=center_y-radius;
  end_x=center_x+radius;
  end_y=center_y+radius;

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  // Affichage du cercle
  for (y_pos=start_y,y=(long)start_y-center_y;y_pos<=end_y;y_pos++,y++)
    for (x_pos=start_x,x=(long)start_x-center_x;x_pos<=end_x;x_pos++,x++)
      if (Pixel_in_circle(x, y, sqradius))
        Display_pixel(x_pos,y_pos,color);

  Update_part_of_screen(start_x,start_y,end_x+1-start_x,end_y+1-start_y);
}

int Circle_squared_diameter(int diameter)
{
  int result = diameter*diameter;
  // Trick to make some circles rounder, even though
  // mathematically incorrect.
  if (diameter==3 || diameter==9)
    return result-2;
  if (diameter==11)
    return result-6;
  if (diameter==14)
    return result-4;

  return result;
}

  // -- Tracer général d'une ellipse vide -----------------------------------

static void Draw_empty_ellipse_general(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;
  long x, y;
  T_Ellipse_limits Ellipse;

  start_x=center_x-horizontal_radius;
  start_y=center_y-vertical_radius;

  // Calcul des limites de l'ellipse
  Ellipse_compute_limites(horizontal_radius+1, vertical_radius+1, &Ellipse);

  // Affichage des extremitées de l'ellipse sur chaque quart de l'ellipse:
  for (y_pos=start_y,y=-vertical_radius;y_pos<center_y;y_pos++,y++)
    for (x_pos=start_x,x=-horizontal_radius;x_pos<center_x;x_pos++,x++)
      if (Pixel_in_ellipse(x, y, &Ellipse))
      {
        // On vient de tomber sur le premier point qui sur la ligne
        // horizontale fait partie de l'ellipse.

        // Donc on peut l'afficher (lui et ses copains symétriques)

         // Quart Haut-gauche
        Pixel_figure(x_pos,y_pos,color);
         // Quart Haut-droite
        Pixel_figure((center_x<<1)-x_pos,y_pos,color);
         // Quart Bas-gauche
        Pixel_figure(x_pos,(center_y<<1)-y_pos,color);
         // Quart Bas-droite
        Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);

        // On peut ensuite afficher tous les points qui le suivent dont le
        // pixel voisin du haut n'appartient pas à l'ellipse:
        for (y--,x_pos++,x++;x_pos<center_x;x_pos++,x++)
          if (!Pixel_in_ellipse(x, y, &Ellipse))
          {
             // Quart Haut-gauche
            Pixel_figure(x_pos,y_pos,color);
             // Quart Haut-droite
            Pixel_figure((center_x<<1)-x_pos,y_pos,color);
             // Quart Bas-gauche
            Pixel_figure(x_pos,(center_y<<1)-y_pos,color);
             // Quart Bas-droite
            Pixel_figure((center_x<<1)-x_pos,(center_y<<1)-y_pos,color);
          }
          else
            break;

        y++;
        break;
      }

  // On affiche à la fin les points cardinaux:

  // points verticaux:
  x_pos=center_x;
  x=-1;
  for (y_pos=center_y+1-vertical_radius,y=-vertical_radius+1;y_pos<center_y+vertical_radius;y_pos++,y++)
    if (!Pixel_in_ellipse(x, y, &Ellipse))
      Pixel_figure(x_pos,y_pos,color);

  // points horizontaux:
  y_pos=center_y;
  y=-1;
  for (x_pos=center_x+1-horizontal_radius,x=-horizontal_radius+1;x_pos<center_x+horizontal_radius;x_pos++,x++)
    if (!Pixel_in_ellipse(x, y, &Ellipse))
      Pixel_figure(x_pos,y_pos,color);

  Pixel_figure(center_x,center_y-vertical_radius,color);   // Haut
  Pixel_figure(center_x-horizontal_radius,center_y,color); // Gauche
  Pixel_figure(center_x+horizontal_radius,center_y,color); // Droite
  Pixel_figure(center_x,center_y+vertical_radius,color);   // Bas

  Update_part_of_screen(center_x-horizontal_radius,center_y-vertical_radius,2*horizontal_radius+1,2*vertical_radius+1);
}

static void Draw_inscribed_ellipse_general(short x1, short y1, short x2, short y2, byte color, byte filled)
{
  short left, right, top, bottom;
  short dbl_center_x; // double of center_x
  short dbl_center_y; // double of center_y
  short dbl_x_radius; // double of horizontal radius
  short dbl_y_radius; // double of vertical radius
  long sq_dbl_x_radius;
  long sq_dbl_y_radius;
  qword sq_dbl_radius_product;
  short x_pos;
  short y_pos;
  short x_max;

  if (x1 > x2)
  {
    left = x2;
    right = x1;
  }
  else
  {
    left = x1;
    right = x2;
  }
  if (y1 > y2)
  {
    top = y2;
    bottom = y1;
  }
  else
  {
    top = y1;
    bottom = y2;
  }
  dbl_center_x = left+right;
  dbl_center_y = top+bottom;
  dbl_x_radius = right-left+1;
  dbl_y_radius = bottom-top+1;
  if ((Selected_circle_ellipse_mode & MASK_CIRCLE_ELLIPSE) == MODE_CIRCLE)
  {
    if (dbl_x_radius > dbl_y_radius)
      dbl_x_radius = dbl_y_radius;
    else
      dbl_y_radius = dbl_x_radius;
  }
  sq_dbl_x_radius = (long)dbl_x_radius*dbl_x_radius;
  sq_dbl_y_radius = (long)dbl_y_radius*dbl_y_radius;
  sq_dbl_radius_product = (qword)sq_dbl_x_radius * sq_dbl_y_radius;

  x_max = right;
  for (y_pos = top; y_pos <= (dbl_center_y >> 1); y_pos++)
  {
    long dbl_y = 2*y_pos - dbl_center_y;
    long sq_dbl_y = dbl_y*dbl_y;
    for (x_pos = left; x_pos <= (dbl_center_x >> 1); x_pos++)
    {
      long dbl_x = 2*x_pos - dbl_center_x;
      long sq_dbl_x = dbl_x*dbl_x;
      if (((qword)sq_dbl_x * sq_dbl_y_radius + (qword)sq_dbl_y * sq_dbl_x_radius) < sq_dbl_radius_product)
      {
        short x_pos_backup = x_pos;
        do
        {
          Pixel_figure(x_pos,y_pos,color);
          Pixel_figure(dbl_center_x - x_pos,y_pos,color);
          Pixel_figure(x_pos,dbl_center_y - y_pos,color);
          Pixel_figure(dbl_center_x - x_pos,dbl_center_y - y_pos,color);
          x_pos++;
        }
        while (x_pos <= (dbl_center_x >> 1) && x_pos < x_max);
        if (!filled && x_pos_backup < x_max)
          x_max = x_pos_backup;
        break;
      }
    }
  }

  Update_part_of_screen(left, top, right-left, bottom-top);
}
  // -- Tracé définitif d'une ellipse vide --

void Draw_empty_ellipse_permanent(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  Pixel_figure=Pixel_figure_permanent;
  Init_permanent_draw();
  Draw_empty_ellipse_general(center_x,center_y,horizontal_radius,vertical_radius,color);
  //Update_part_of_screen(center_x - horizontal_radius, center_y - vertical_radius, 2* horizontal_radius+1, 2*vertical_radius+1);
}

void Draw_empty_inscribed_ellipse_permanent(short x1,short y1,short x2, short y2,byte color)
{
  Pixel_figure=Pixel_figure_permanent;
  Init_permanent_draw();
  Draw_inscribed_ellipse_general(x1, y1, x2, y2, color, 0);
}

  // -- Tracer la preview d'une ellipse vide --

void Draw_empty_ellipse_preview(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  Pixel_figure=Pixel_figure_preview;
  Draw_empty_ellipse_general(center_x,center_y,horizontal_radius,vertical_radius,color);
  //Update_part_of_screen(center_x - horizontal_radius, center_y - vertical_radius, 2* horizontal_radius+1, 2*vertical_radius +1);
}

void Draw_empty_inscribed_ellipse_preview(short x1,short y1,short x2,short y2,byte color)
{
  Pixel_figure=Pixel_figure_preview;
  Draw_inscribed_ellipse_general(x1, y1, x2, y2, color, 0);
}

  // -- Effacer la preview d'une ellipse vide --

void Hide_empty_ellipse_preview(short center_x,short center_y,short horizontal_radius,short vertical_radius)
{
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_empty_ellipse_general(center_x,center_y,horizontal_radius,vertical_radius,0);
  //Update_part_of_screen(center_x - horizontal_radius, center_y - vertical_radius, 2* horizontal_radius+1, 2*vertical_radius+1);
}

void Hide_empty_inscribed_ellipse_preview(short x1,short y1,short x2,short y2)
{
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_inscribed_ellipse_general(x1,y1,x2,y2,0,0);
}

  // -- Tracer une ellipse pleine --

void Draw_filled_ellipse(short center_x,short center_y,short horizontal_radius,short vertical_radius,byte color)
{
  short start_x;
  short start_y;
  short x_pos;
  short y_pos;
  short end_x;
  short end_y;
  long x, y;
  T_Ellipse_limits Ellipse;

  start_x=center_x-horizontal_radius;
  start_y=center_y-vertical_radius;
  end_x=center_x+horizontal_radius;
  end_y=center_y+vertical_radius;

  // Calcul des limites de l'ellipse
  Ellipse_compute_limites(horizontal_radius+1, vertical_radius+1, &Ellipse);

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  // Affichage de l'ellipse
  for (y_pos=start_y,y=start_y-center_y;y_pos<=end_y;y_pos++,y++)
    for (x_pos=start_x,x=start_x-center_x;x_pos<=end_x;x_pos++,x++)
      if (Pixel_in_ellipse(x, y, &Ellipse))
        Display_pixel(x_pos,y_pos,color);
  Update_part_of_screen(center_x-horizontal_radius,center_y-vertical_radius,2*horizontal_radius+1,2*vertical_radius+1);
}

void Draw_filled_inscribed_ellipse(short x1,short y1,short x2,short y2,byte color)
{
  Pixel_figure = Pixel_clipped;
  Draw_inscribed_ellipse_general(x1, y1, x2, y2, color, 1);
}

/******************
* TRACÉ DE LIGNES *
******************/

/// Alters bx and by so the (AX,AY)-(BX,BY) segment becomes either horizontal,
/// vertical, 45degrees, or isometrical for pixelart (ie 2:1 ratio)
void Clamp_coordinates_regular_angle(short ax, short ay, short* bx, short* by)
{
  int dx, dy;
  float angle;

  dx = *bx-ax;
  dy = *by-ay;

  // No mouse move: no need to clamp anything
  if (dx==0 || dy == 0) return;

  // Determine angle (heading)
  angle = atan2(dx, dy);

  // Get absolute values, useful from now on:
  //dx=abs(dx);
  //dy=abs(dy);

  // Negative Y
  if (angle < M_PI*(-15.0/16.0) || angle > M_PI*(15.0/16.0))
  {
    *bx=ax;
    *by=ay + dy;
  }
  // Iso close to negative Y
  else if (angle < M_PI*(-13.0/16.0))
  {
    dy=dy | 1; // Round up to next odd number
    *bx=ax + dy/2;
    *by=ay + dy;
  }
  // 45deg
  else if (angle < M_PI*(-11.0/16.0))
  {
    *by = (*by + ay + dx)/2;
    *bx = ax  - ay + *by;
  }
  // Iso close to negative X
  else if (angle < M_PI*(-9.0/16.0))
  {
    dx=dx | 1; // Round up to next odd number
    *bx=ax + dx;
    *by=ay + dx/2;
  }
  // Negative X
  else if (angle < M_PI*(-7.0/16.0))
  {
    *bx=ax + dx;
    *by=ay;
  }
  // Iso close to negative X
  else if (angle < M_PI*(-5.0/16.0))
  {
    dx=dx | 1; // Round up to next odd number
    *bx=ax + dx;
    *by=ay - dx/2;
  }
  // 45 degrees
  else if (angle < M_PI*(-3.0/16.0))
  {
    *by = (*by + ay - dx)/2;
    *bx = ax  + ay - *by;
  }
  // Iso close to positive Y
  else if (angle < M_PI*(-1.0/16.0))
  {
    dy=dy | 1; // Round up to next odd number
    *bx=ax - dy/2;
    *by=ay + dy;
  }
  // Positive Y
  else if (angle < M_PI*(1.0/16.0))
  {
    *bx=ax;
    *by=ay + dy;
  }
  // Iso close to positive Y
  else if (angle < M_PI*(3.0/16.0))
  {
    dy=dy | 1; // Round up to next odd number
    *bx=ax + dy/2;
    *by=ay + dy;
  }
  // 45 degrees
  else if (angle < M_PI*(5.0/16.0))
  {
    *by = (*by + ay + dx)/2;
    *bx = ax  - ay + *by;
  }
  // Iso close to positive X
  else if (angle < M_PI*(7.0/16.0))
  {
    dx=dx | 1; // Round up to next odd number
    *bx=ax + dx;
    *by=ay + dx/2;
  }
  // Positive X
  else if (angle < M_PI*(9.0/16.0))
  {
    *bx=ax + dx;
    *by=ay;
  }
  // Iso close to positive X
  else if (angle < M_PI*(11.0/16.0))
  {
    dx=dx | 1; // Round up to next odd number
    *bx=ax + dx;
    *by=ay - dx/2;
  }
  // 45 degrees
  else if (angle < M_PI*(13.0/16.0))
  {
    *by = (*by + ay - dx)/2;
    *bx = ax  + ay - *by;
  }
  // Iso close to negative Y
  else //if (angle < M_PI*(15.0/16.0))
  {
    dy=dy | 1; // Round up to next odd number
    *bx=ax - dy/2;
    *by=ay + dy;
  }

  return;
}

  // -- Tracer général d'une ligne ------------------------------------------

void Draw_line_general(short start_x,short start_y,short end_x,short end_y, byte color)
{
  short x_pos,y_pos;
  short incr_x,incr_y;
  short i,cumul;
  short delta_x,delta_y;

  x_pos=start_x;
  y_pos=start_y;

  if (start_x<end_x)
  {
    incr_x=+1;
    delta_x=end_x-start_x;
  }
  else
  {
    incr_x=-1;
    delta_x=start_x-end_x;
  }

  if (start_y<end_y)
  {
    incr_y=+1;
    delta_y=end_y-start_y;
  }
  else
  {
    incr_y=-1;
    delta_y=start_y-end_y;
  }

  if (delta_y>delta_x)
  {
    cumul=delta_y>>1;
    for (i=1; i<delta_y; i++)
    {
      y_pos+=incr_y;
      cumul+=delta_x;
      if (cumul>=delta_y)
      {
        cumul-=delta_y;
        x_pos+=incr_x;
      }
      Pixel_figure(x_pos,y_pos,color);
    }
  }
  else
  {
    cumul=delta_x>>1;
    for (i=1; i<delta_x; i++)
    {
      x_pos+=incr_x;
      cumul+=delta_y;
      if (cumul>=delta_x)
      {
        cumul-=delta_x;
        y_pos+=incr_y;
      }
      Pixel_figure(x_pos,y_pos,color);
    }
  }

  if ( (start_x!=end_x) || (start_y!=end_y) )
    Pixel_figure(end_x,end_y,color);

}

  // -- Tracer définitif d'une ligne --

void Draw_line_permanent(short start_x,short start_y,short end_x,short end_y, byte color)
{

  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_permanent;
  Init_permanent_draw();
  Draw_line_general(start_x,start_y,end_x,end_y,color);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne --

void Draw_line_preview(short start_x,short start_y,short end_x,short end_y,byte color)
{
  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_preview;
  Draw_line_general(start_x,start_y,end_x,end_y,color);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne en xor --

void Draw_line_preview_xor(short start_x,short start_y,short end_x,short end_y,byte color)
{
  int w, h;

  Pixel_figure=(Func_pixel)Pixel_figure_preview_xor;
  // Needed a cast because this function supports signed shorts,
  // (it's usually in image space), while this time it's in screen space
  // and some line endpoints can be out of screen.
  Draw_line_general(start_x,start_y,end_x,end_y,color);

  if (start_x<Limit_left)
    start_x=Limit_left;
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_x<Limit_left)
    end_x=Limit_left;
  if (end_y<Limit_top)
    end_y=Limit_top;
  // bottom & right limits are handled by Update_part_of_screen()

  w = end_x-start_x;
  h = end_y-start_y;
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Tracer la preview d'une ligne en xor additif --

void Draw_line_preview_xorback(short start_x,short start_y,short end_x,short end_y,byte color)
{
  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_preview_xorback;
  Draw_line_general(start_x,start_y,end_x,end_y,color);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}

  // -- Effacer la preview d'une ligne --

void Hide_line_preview(short start_x,short start_y,short end_x,short end_y)
{
  int w = end_x-start_x, h = end_y - start_y;
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_line_general(start_x,start_y,end_x,end_y,0);
  Update_part_of_screen((start_x<end_x)?start_x:end_x,(start_y<end_y)?start_y:end_y,abs(w)+1,abs(h)+1);
}


  // -- Tracer un rectangle vide --

void Draw_empty_rectangle(short start_x,short start_y,short end_x,short end_y,byte color)
{
  short temp;
  short x_pos;
  short y_pos;


  // On vérifie que les bornes soient dans le bon sens:
  if (start_x>end_x)
  {
    temp=start_x;
    start_x=end_x;
    end_x=temp;
  }
  if (start_y>end_y)
  {
    temp=start_y;
    start_y=end_y;
    end_y=temp;
  }

  // On trace le rectangle:
  Init_permanent_draw();

  for (x_pos=start_x;x_pos<=end_x;x_pos++)
  {
    Pixel_figure_permanent(x_pos,start_y,color);
    Pixel_figure_permanent(x_pos,  end_y,color);
  }

  for (y_pos=start_y+1;y_pos<end_y;y_pos++)
  {
    Pixel_figure_permanent(start_x,y_pos,color);
    Pixel_figure_permanent(  end_x,y_pos,color);
  }

#if defined(__macosx__) || defined(__FreeBSD__)
  Update_part_of_screen(start_x,end_x,end_x-start_x,end_y-start_y);
#endif
}

  // -- Tracer un rectangle plein --

void Draw_filled_rectangle(short start_x,short start_y,short end_x,short end_y,byte color)
{
  short temp;
  short x_pos;
  short y_pos;


  // On vérifie que les bornes sont dans le bon sens:
  if (start_x>end_x)
  {
    temp=start_x;
    start_x=end_x;
    end_x=temp;
  }
  if (start_y>end_y)
  {
    temp=start_y;
    start_y=end_y;
    end_y=temp;
  }

  // Correction en cas de dépassement des limites de l'image
  if (end_x>Limit_right)
    end_x=Limit_right;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;

  // On trace le rectangle:
  for (y_pos=start_y;y_pos<=end_y;y_pos++)
    for (x_pos=start_x;x_pos<=end_x;x_pos++)
      // Display_pixel traite chaque pixel avec tous les effets ! (smear, ...)
      // Donc on ne peut pas otimiser en traçant ligne par ligne avec memset :(
      Display_pixel(x_pos,y_pos,color);
  Update_part_of_screen(start_x,start_y,end_x-start_x,end_y-start_y);

}




  // -- Tracer une courbe de Bézier --

void Draw_curve_general(short x1, short y1,
                           short x2, short y2,
                           short x3, short y3,
                           short x4, short y4,
                           byte color)
{
  float delta,t,t2,t3;
  short x,y,old_x,old_y;
  word  i;
  int   cx[4];
  int   cy[4];

  // Calcul des vecteurs de coefficients
  cx[0]= -   x1 + 3*x2 - 3*x3 + x4;
  cx[1]= + 3*x1 - 6*x2 + 3*x3;
  cx[2]= - 3*x1 + 3*x2;
  cx[3]= +   x1;
  cy[0]= -   y1 + 3*y2 - 3*y3 + y4;
  cy[1]= + 3*y1 - 6*y2 + 3*y3;
  cy[2]= - 3*y1 + 3*y2;
  cy[3]= +   y1;

  // Traçage de la courbe
  old_x=x1;
  old_y=y1;
  Pixel_figure(old_x,old_y,color);
  delta=0.05f; // 1.0/20
  t=0;
  for (i=1; i<=20; i++)
  {
    t=t+delta; t2=t*t; t3=t2*t;
    x=Round(t3*cx[0] + t2*cx[1] + t*cx[2] + cx[3]);
    y=Round(t3*cy[0] + t2*cy[1] + t*cy[2] + cy[3]);
    Draw_line_general(old_x,old_y,x,y,color);
    old_x=x;
    old_y=y;
  }

  x = Min(Min(x1,x2),Min(x3,x4));
  y = Min(Min(y1,y2),Min(y3,y4));
  old_x = Max(Max(x1,x2),Max(x3,x4)) - x;
  old_y = Max(Max(y1,y2),Max(y3,y4)) - y;
  Update_part_of_screen(x,y,old_x+1,old_y+1);
}

  // -- Tracer une courbe de Bézier définitivement --

void Draw_curve_permanent(short x1, short y1,
                             short x2, short y2,
                             short x3, short y3,
                             short x4, short y4,
                             byte color)
{
  Pixel_figure=Pixel_figure_permanent;
  Init_permanent_draw();
  Draw_curve_general(x1,y1,x2,y2,x3,y3,x4,y4,color);
}

  // -- Tracer la preview d'une courbe de Bézier --

void Draw_curve_preview(short x1, short y1,
                           short x2, short y2,
                           short x3, short y3,
                           short x4, short y4,
                           byte color)
{
  Pixel_figure=Pixel_figure_preview;
  Draw_curve_general(x1,y1,x2,y2,x3,y3,x4,y4,color);
}

  // -- Effacer la preview d'une courbe de Bézier --

void Hide_curve_preview(short x1, short y1,
                            short x2, short y2,
                            short x3, short y3,
                            short x4, short y4,
                            byte color)
{
  Pixel_figure=Pixel_figure_clear_preview;
  Draw_curve_general(x1,y1,x2,y2,x3,y3,x4,y4,color);
}




  // -- Spray : un petit coup de Pschiitt! --

void Airbrush(short clicked_button)
{
  short x_pos,y_pos;
  short radius=Airbrush_size>>1;
  long  radius_squared=(long)radius*radius;
  short index,count;
  byte  color_index;
  byte  direction;


  Hide_cursor();

  if (Airbrush_mode)
  {
    for (count=1; count<=Airbrush_mono_flow; count++)
    {
      x_pos=(rand()%Airbrush_size)-radius;
      y_pos=(rand()%Airbrush_size)-radius;
      if ( (x_pos*x_pos)+(y_pos*y_pos) <= radius_squared )
      {
        x_pos+=Paintbrush_X;
        y_pos+=Paintbrush_Y;
        if (clicked_button==1)
          Draw_paintbrush(x_pos,y_pos,Fore_color);
        else
          Draw_paintbrush(x_pos,y_pos,Back_color);
      }
    }
  }
  else
  {
    //   On essaye de se balader dans la table des flux de façon à ce que ce
    // ne soit pas toujours la dernière couleur qui soit affichée en dernier
    // Pour ça, on part d'une couleur au pif dans une direction aléatoire.
    direction=rand()&1;
    for (index=0,color_index=rand()/*%256*/; index<256; index++)
    {
      for (count=1; count<=Airbrush_multi_flow[color_index]; count++)
      {
        x_pos=(rand()%Airbrush_size)-radius;
        y_pos=(rand()%Airbrush_size)-radius;
        if ( (x_pos*x_pos)+(y_pos*y_pos) <= radius_squared )
        {
          x_pos+=Paintbrush_X;
          y_pos+=Paintbrush_Y;
          if (clicked_button==LEFT_SIDE)
            Draw_paintbrush(x_pos,y_pos,color_index);
          else
            Draw_paintbrush(x_pos,y_pos,Back_color);
        }
      }
      if (direction)
        color_index++;
      else
        color_index--;
    }
  }

  Display_cursor();
}



  //////////////////////////////////////////////////////////////////////////
  ////////////////////////// GESTION DES DEGRADES //////////////////////////
  //////////////////////////////////////////////////////////////////////////


  // -- Gestion d'un dégradé de base (le plus moche) --

void Gradient_basic(long index,short x_pos,short y_pos)
{
  long position;

  // On fait un premier calcul partiel
  position=(index*Gradient_bounds_range);

  // On gère un déplacement au hasard
  position+=(Gradient_total_range*(rand()%Gradient_random_factor)) >>6;
  position-=(Gradient_total_range*Gradient_random_factor) >>7;

  position/=Gradient_total_range;

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (position<0)
    position=0;
  else if (position>=Gradient_bounds_range)
    position=Gradient_bounds_range-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Gradient_is_inverted)
    Gradient_pixel(x_pos,y_pos,Gradient_upper_bound-position);
  else
    Gradient_pixel(x_pos,y_pos,Gradient_lower_bound+position);
}


  // -- Gestion d'un dégradé par trames simples --

void Gradient_dithered(long index,short x_pos,short y_pos)
{
  long position_in_gradient;
  long position_in_segment;

  //
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "position_in_gradient", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  position_in_gradient=(index*Gradient_bounds_range);

  // On gère un déplacement au hasard...
  position_in_gradient+=(Gradient_total_range*(rand()%Gradient_random_factor)) >>6;
  position_in_gradient-=(Gradient_total_range*Gradient_random_factor) >>7;

  if (position_in_gradient<0)
    position_in_gradient=0;

  // ... qui nous permet de calculer la position dans le segment
  position_in_segment=((position_in_gradient<<2)/Gradient_total_range)&3;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  position_in_gradient/=Gradient_total_range;

  // On va pouvoir discuter de la valeur de position_in_gradient en fonction
  // de la position dans l'écran et de la position_in_segment.

  switch (position_in_segment)
  {
    case 0 : // On est sur la gauche du segment
      if (((x_pos+y_pos)&1)==0)
        position_in_gradient--;
      break;

      // On n'a pas à traiter les cas 1 et 2 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 3 : // On est sur la droite du segment
      if (((x_pos+y_pos)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        position_in_gradient++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (position_in_gradient<0)
    position_in_gradient=0;
  else if (position_in_gradient>=Gradient_bounds_range)
    position_in_gradient=Gradient_bounds_range-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Gradient_is_inverted)
    position_in_gradient=Gradient_upper_bound-position_in_gradient;
  else
    position_in_gradient=Gradient_lower_bound+position_in_gradient;

  Gradient_pixel(x_pos,y_pos,position_in_gradient);
}


  // -- Gestion d'un dégradé par trames étendues --

void Gradient_extra_dithered(long index,short x_pos,short y_pos)
{
  long position_in_gradient;
  long position_in_segment;

//
  //   But de l'opération: en plus de calculer la position de base (désignée
  // dans cette procédure par "position_in_gradient", on calcule la position
  // de l'indice dans le schéma suivant:
  //
  //         | Les indices qui traînent de ce côté du segment se voient subir
  //         | une incrémentation conditionnelle à leur position dans l'écran.
  //         v
  //  |---|---|---|---- - - -
  //   ^
  //   |_ Les indices qui traînent de ce côté du segment se voient subir une
  //      décrémentation conditionnelle à leur position dans l'écran.

  // On fait d'abord un premier calcul partiel
  position_in_gradient=(index*Gradient_bounds_range);

  // On gère un déplacement au hasard
  position_in_gradient+=(Gradient_total_range*(rand()%Gradient_random_factor)) >>6;
  position_in_gradient-=(Gradient_total_range*Gradient_random_factor) >>7;

  if (position_in_gradient<0)
    position_in_gradient=0;

  // Qui nous permet de calculer la position dans le segment
  position_in_segment=((position_in_gradient<<3)/Gradient_total_range)&7;

  // On peut ensuite terminer le calcul de l'indice dans le dégradé
  position_in_gradient/=Gradient_total_range;

  // On va pouvoir discuter de la valeur de position_in_gradient en fonction
  // de la position dans l'écran et de la position_in_segment.

  switch (position_in_segment)
  {
    case 0 : // On est sur l'extrême gauche du segment
      if (((x_pos+y_pos)&1)==0)
        position_in_gradient--;
      break;

    case 1 : // On est sur la gauche du segment
    case 2 : // On est sur la gauche du segment
      if (((x_pos & 1)==0) && ((y_pos & 1)==0))
        position_in_gradient--;
      break;

      // On n'a pas à traiter les cas 3 et 4 car ils représentent des valeurs
      // suffisament au centre du segment pour ne pas avoir à subir la trame

    case 5 : // On est sur la droite du segment
    case 6 : // On est sur la droite du segment
      if (((x_pos & 1)==0) && ((y_pos & 1)!=0))
        position_in_gradient++;
      break;

    case 7 : // On est sur l'extreme droite du segment
      if (((x_pos+y_pos)&1)!=0) // Note: on doit faire le test inverse au cas gauche pour synchroniser les 2 côtés de la trame.
        position_in_gradient++;
  }

  //   On va vérifier que nos petites idioties n'ont pas éjecté la valeur hors
  // des valeurs autorisées par le dégradé défini par l'utilisateur.

  if (position_in_gradient<0)
    position_in_gradient=0;
  else if (position_in_gradient>=Gradient_bounds_range)
    position_in_gradient=Gradient_bounds_range-1;

  // On ramène ensuite la position dans le dégradé vers un numéro de couleur
  if (Gradient_is_inverted)
    position_in_gradient=Gradient_upper_bound-position_in_gradient;
  else
    position_in_gradient=Gradient_lower_bound+position_in_gradient;

  Gradient_pixel(x_pos,y_pos,position_in_gradient);
}



  // -- Tracer un cercle degradé (une sphère) --

void Draw_grad_circle(short center_x,short center_y,long sqradius,short spot_x,short spot_y)
{
  long start_x;
  long start_y;
  long x_pos;
  long y_pos;
  long end_x;
  long end_y;
  long distance_x; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long distance_y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage
  long x, y;
  short radius = sqrt(sqradius);

  start_x=center_x-radius;
  start_y=center_y-radius;
  end_x=center_x+radius;
  end_y=center_y+radius;

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  Gradient_total_range=sqradius+
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))+
                           (2L*radius*sqrt(
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))));

  if (Gradient_total_range==0)
    Gradient_total_range=1;

  // Affichage du cercle
  for (y_pos=start_y,y=(long)start_y-center_y;y_pos<=end_y;y_pos++,y++)
  {
    distance_y =(y_pos-spot_y);
    distance_y*=distance_y;
    for (x_pos=start_x,x=(long)start_x-center_x;x_pos<=end_x;x_pos++,x++)
      if (Pixel_in_circle(x, y, sqradius))
      {
        distance_x =(x_pos-spot_x);
        distance_x*=distance_x;
        Gradient_function(distance_x+distance_y,x_pos,y_pos);
      }
  }

  Update_part_of_screen(center_x-radius,center_y-radius,2*radius+1,2*radius+1);
}


  // -- Tracer une ellipse degradée --

void Draw_grad_ellipse(short center_x,short center_y,short horizontal_radius,short vertical_radius,short spot_x,short spot_y)
{
  long start_x;
  long start_y;
  long x_pos;
  long y_pos;
  long end_x;
  long end_y;
  long distance_x; // Distance (au carré) sur les X du point en cours au centre d'éclairage
  long distance_y; // Distance (au carré) sur les Y du point en cours au centre d'éclairage
  long x, y;
  T_Ellipse_limits Ellipse;

  start_x=center_x-horizontal_radius;
  start_y=center_y-vertical_radius;
  end_x=center_x+horizontal_radius;
  end_y=center_y+vertical_radius;

  // Calcul des limites de l'ellipse
  Ellipse_compute_limites(horizontal_radius+1, vertical_radius+1, &Ellipse);

  // On calcule la distance maximale:
  Gradient_total_range=(horizontal_radius*horizontal_radius)+
                           (vertical_radius*vertical_radius)+
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))+
                           (2L
                           *sqrt(
                           (horizontal_radius*horizontal_radius)+
                           (vertical_radius  *vertical_radius  ))
                           *sqrt(
                           ((center_x-spot_x)*(center_x-spot_x))+
                           ((center_y-spot_y)*(center_y-spot_y))));

  if (Gradient_total_range==0)
    Gradient_total_range=1;

  // Correction des bornes d'après les limites
  if (start_y<Limit_top)
    start_y=Limit_top;
  if (end_y>Limit_bottom)
    end_y=Limit_bottom;
  if (start_x<Limit_left)
    start_x=Limit_left;
  if (end_x>Limit_right)
    end_x=Limit_right;

  // Affichage de l'ellipse
  for (y_pos=start_y,y=start_y-center_y;y_pos<=end_y;y_pos++,y++)
  {
    distance_y =(y_pos-spot_y);
    distance_y*=distance_y;
    for (x_pos=start_x,x=start_x-center_x;x_pos<=end_x;x_pos++,x++)
      if (Pixel_in_ellipse(x, y, &Ellipse))
      {
        distance_x =(x_pos-spot_x);
        distance_x*=distance_x;
        Gradient_function(distance_x+distance_y,x_pos,y_pos);
      }
  }

  Update_part_of_screen(start_x,start_y,end_x-start_x+1,end_y-start_y+1);
}

void Draw_grad_inscribed_ellipse(short x1, short y1, short x2, short y2, short spot_x, short spot_y)
{
  short left, right, top, bottom;
  short dbl_center_x; // double of center_x
  short dbl_center_y; // double of center_y
  short dbl_x_radius; // double of horizontal radius
  short dbl_y_radius; // double of vertical radius
  long sq_dbl_x_radius;
  long sq_dbl_y_radius;
  qword sq_dbl_radius_product;
  short x_pos;
  short y_pos;
  long sq_dist_x; // Square horizontal distance with the lightning point
  long sq_dist_y; // Square vertical distance with the lightning point

  if (x1 > x2)
  {
    left = x2;
    right = x1;
  }
  else
  {
    left = x1;
    right = x2;
  }
  if (y1 > y2)
  {
    top = y2;
    bottom = y1;
  }
  else
  {
    top = y1;
    bottom = y2;
  }
  dbl_center_x = left+right;
  dbl_center_y = top+bottom;
  dbl_x_radius = right-left+1;
  dbl_y_radius = bottom-top+1;
  if ((Selected_circle_ellipse_mode & MASK_CIRCLE_ELLIPSE) == MODE_CIRCLE)
  {
    if (dbl_x_radius > dbl_y_radius)
      dbl_x_radius = dbl_y_radius;
    else
      dbl_y_radius = dbl_x_radius;
  }
  sq_dbl_x_radius = (long)dbl_x_radius*dbl_x_radius;
  sq_dbl_y_radius = (long)dbl_y_radius*dbl_y_radius;
  sq_dbl_radius_product = (qword)sq_dbl_x_radius * sq_dbl_y_radius;

  // calculate grandient range
  Gradient_total_range= (sq_dbl_x_radius + sq_dbl_y_radius) / 4 +
                           ((dbl_center_x/2-spot_x)*(dbl_center_x/2-spot_x))+
                           ((dbl_center_y/2-spot_y)*(dbl_center_y/2-spot_y))+
                           (sqrt(sq_dbl_x_radius + sq_dbl_y_radius)
                           *sqrt(
                           ((dbl_center_x/2-spot_x)*(dbl_center_x/2-spot_x))+
                           ((dbl_center_y/2-spot_y)*(dbl_center_y/2-spot_y))));

  if (Gradient_total_range==0)
    Gradient_total_range=1;

  for (y_pos = top; y_pos <= bottom; y_pos++)
  {
    long dbl_y = 2*y_pos - dbl_center_y;
    long sq_dbl_y = dbl_y*dbl_y;
    sq_dist_y =(y_pos-spot_y);
    sq_dist_y *= sq_dist_y;
    for (x_pos = left; x_pos <= right; x_pos++)
    {
      long dbl_x = 2*x_pos - dbl_center_x;
      long sq_dbl_x = dbl_x*dbl_x;
      if (((qword)sq_dbl_x * sq_dbl_y_radius + (qword)sq_dbl_y * sq_dbl_x_radius) < sq_dbl_radius_product)
      {
        sq_dist_x =(x_pos-spot_x);
        sq_dist_x *= sq_dist_x;
        Gradient_function(sq_dist_x+sq_dist_y,x_pos,y_pos);
      }
    }
  }

  Update_part_of_screen(left, top, right-left+1, bottom-top+1);
}

// Tracé d'un rectangle (rax ray - rbx rby) dégradé selon le vecteur (vax vay - vbx - vby)
void Draw_grad_rectangle(short rax,short ray,short rbx,short rby,short vax,short vay, short vbx, short vby)
{
    short y_pos, x_pos;

    // On commence par s'assurer que le rectangle est à l'endroit
    if(rbx < rax)
    {
      x_pos = rbx;
      rbx = rax;
      rax = x_pos;
    }

    if(rby < ray)
    {
      y_pos = rby;
      rby = ray;
      ray = y_pos;
    }

    // Correction des bornes d'après les limites
    if (ray<Limit_top)
      ray=Limit_top;
    if (rby>Limit_bottom)
      rby=Limit_bottom;
    if (rax<Limit_left)
      rax=Limit_left;
    if (rbx>Limit_right)
      rbx=Limit_right;

    if(vbx == vax)
    {
      // Le vecteur est vertical, donc on évite la partie en dessous qui foirerait avec une division par 0...
      if (vby == vay) return;  // L'utilisateur fait n'importe quoi
      Gradient_total_range = abs(vby - vay);
      for(y_pos=ray;y_pos<=rby;y_pos++)
        for(x_pos=rax;x_pos<=rbx;x_pos++)
          Gradient_function(abs(vby - y_pos),x_pos,y_pos);

    }
    else
    {
      float a;
      float b;
      float distance_x, distance_y;

      Gradient_total_range = sqrt(pow(vby - vay,2)+pow(vbx - vax,2));
      a = (float)(vby - vay)/(float)(vbx - vax);
      b = vay - a*vax;

      for (y_pos=ray;y_pos<=rby;y_pos++)
        for (x_pos = rax;x_pos<=rbx;x_pos++)
        {
          // On calcule ou on en est dans le dégradé
          distance_x = pow((y_pos - vay),2)+pow((x_pos - vax),2);
          distance_y = pow((-a * x_pos + y_pos - b),2)/(a*a+1);

          Gradient_function((int)sqrt(distance_x - distance_y),x_pos,y_pos);
        }
    }
    Update_part_of_screen(rax,ray,rbx,rby);
}




// -- Tracer un polygône plein --

typedef struct T_Polygon_edge      /* an active edge */
{
    short top;                     /* top y position */
    short bottom;                  /* bottom y position */
    float x, dx;                   /* floating point x position and gradient */
    float w;                       /* width of line segment */
    struct T_Polygon_edge *prev;     /* doubly linked list */
    struct T_Polygon_edge *next;
} T_Polygon_edge;



/* Fill_edge_structure:
 *  Polygon helper function: initialises an edge structure for the 2d
 *  rasteriser.
 */
void Fill_edge_structure(T_Polygon_edge *edge, short *i1, short *i2)
{
  short *it;

  if (i2[1] < i1[1])
  {
    it = i1;
    i1 = i2;
    i2 = it;
  }

  edge->top = i1[1];
  edge->bottom = i2[1] - 1;
  edge->dx = ((float) i2[0] - (float) i1[0]) / ((float) i2[1] - (float) i1[1]);
  edge->x = i1[0] + 0.4999999;
  edge->prev = NULL;
  edge->next = NULL;

  if (edge->dx+1 < 0.0)
    edge->x += edge->dx+1;

  if (edge->dx >= 0.0)
    edge->w = edge->dx;
  else
    edge->w = -(edge->dx);

  if (edge->w-1.0<0.0)
    edge->w = 0.0;
  else
    edge->w = edge->w-1;
}



/* Add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
T_Polygon_edge * Add_edge(T_Polygon_edge *list, T_Polygon_edge *edge, int sort_by_x)
{
  T_Polygon_edge *pos = list;
  T_Polygon_edge *prev = NULL;

  if (sort_by_x)
  {
    while ( (pos) && ((pos->x+((pos->w+pos->dx)/2)) < (edge->x+((edge->w+edge->dx)/2))) )
    {
      prev = pos;
      pos = pos->next;
    }
  }
  else
  {
    while ((pos) && (pos->top < edge->top))
    {
      prev = pos;
      pos = pos->next;
    }
  }

  edge->next = pos;
  edge->prev = prev;

  if (pos)
    pos->prev = edge;

  if (prev)
  {
    prev->next = edge;
    return list;
  }
  else
    return edge;
}



/* Remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
T_Polygon_edge * Remove_edge(T_Polygon_edge *list, T_Polygon_edge *edge)
{
  if (edge->next)
    edge->next->prev = edge->prev;

  if (edge->prev)
  {
    edge->prev->next = edge->next;
    return list;
  }
  else
    return edge->next;
}



/* polygon:
 *  Draws a filled polygon with an arbitrary number of corners. Pass the
 *  number of vertices, then an array containing a series of x, y points
 *  (a total of vertices*2 values).
 */
void Polyfill_general(int vertices, short * points, int color)
{
  short c;
  short top;
  short bottom;
  short *i1, *i2;
  short x_pos,end_x;
  T_Polygon_edge *edge, *next_edge, *initial_edge;
  T_Polygon_edge *active_edges = NULL;
  T_Polygon_edge *inactive_edges = NULL;

  if (vertices < 1)
    return;

  top = bottom = points[1];

  /* allocate some space and fill the edge table */
  initial_edge=edge=(T_Polygon_edge *) malloc(sizeof(T_Polygon_edge) * vertices);

  i1 = points;
  i2 = points + ((vertices-1)<<1);

  for (c=0; c<vertices; c++)
  {
    if (i1[1] != i2[1])
    {
      Fill_edge_structure(edge, i1, i2);

      if (edge->bottom >= edge->top)
      {
        if (edge->top < top)
          top = edge->top;

        if (edge->bottom > bottom)
          bottom = edge->bottom;

        inactive_edges = Add_edge(inactive_edges, edge, 0);
        edge++;
      }
    }
    i2 = i1;
    i1 += 2;
  }

  /* for each scanline in the polygon... */
  for (c=top; c<=bottom; c++)
  {
    /* check for newly active edges */
    edge = inactive_edges;
    while ((edge) && (edge->top == c))
    {
      next_edge = edge->next;
      inactive_edges = Remove_edge(inactive_edges, edge);
      active_edges = Add_edge(active_edges, edge, 1);
      edge = next_edge;
    }

    /* draw horizontal line segments */
    if ((c>=Limit_top) && (c<=Limit_bottom))
    {
      edge = active_edges;
      while ((edge) && (edge->next))
      {
        x_pos=/*Round*/(edge->x);
        end_x=/*Round*/(edge->next->x+edge->next->w);
        if (x_pos<Limit_left)
          x_pos=Limit_left;
        if (end_x>Limit_right)
          end_x=Limit_right;
        for (; x_pos<=end_x; x_pos++)
          Pixel_figure(x_pos,c,color);
        edge = edge->next->next;
      }
    }

    /* update edges, sorting and removing dead ones */
    edge = active_edges;
    while (edge)
    {
      next_edge = edge->next;
      if (c >= edge->bottom)
        active_edges = Remove_edge(active_edges, edge);
      else
      {
        edge->x += edge->dx;
        while ((edge->prev) && ( (edge->x+(edge->w/2)) < (edge->prev->x+(edge->prev->w/2))) )
        {
          if (edge->next)
            edge->next->prev = edge->prev;
          edge->prev->next = edge->next;
          edge->next = edge->prev;
          edge->prev = edge->prev->prev;
          edge->next->prev = edge;
          if (edge->prev)
            edge->prev->next = edge;
          else
            active_edges = edge;
        }
      }
      edge = next_edge;
    }
  }

  free(initial_edge);
  initial_edge = NULL;

  // On ne connait pas simplement les xmin et xmax ici, mais de toutes façon ce n'est pas utilisé en preview
  Update_part_of_screen(0,top,Main.image_width,bottom-top+1);
}


void Polyfill(int vertices, short * points, int color)
{
  int index;

  Pixel_clipped(points[0],points[1],color);
  if (vertices==1)
  {
    Update_part_of_screen(points[0],points[1],1,1);
    return;
  }

  // Comme pour le Fill, cette operation fait un peu d'"overdraw"
  // (pixels dessinés plus d'une fois) alors on force le FX Feedback à OFF
  Update_FX_feedback(0);

  Pixel_figure=Pixel_clipped;
  Polyfill_general(vertices,points,color);

  // Remarque: pour dessiner la bordure avec la brosse en cours au lieu
  // d'un pixel de couleur premier-plan, il suffit de mettre ici:
  // Pixel_figure=Pixel_figure_permanent;

  // Dessin du contour
  for (index=0; index<vertices-1;index+=1)
    Draw_line_general(points[index*2],points[index*2+1],points[index*2+2],points[index*2+3],color);
  Draw_line_general(points[0],points[1],points[index*2],points[index*2+1],color);

  // Restore original feedback value
  Update_FX_feedback(Config.FX_Feedback);

}



//------------ Remplacement de la couleur pointée par une autre --------------

void Replace(byte new_color)
{
  byte old_color;

  if ((Paintbrush_X<Main.image_width)
   && (Paintbrush_Y<Main.image_height))
  {
    old_color=Read_pixel_from_current_layer(Paintbrush_X,Paintbrush_Y);
    if ( (old_color!=new_color)
      && ((!Stencil_mode) || (!Stencil[old_color])) )
    {
      word x;
      word y;

      // Update all pixels
      for (y=0; y<Main.image_height; y++)
        for (x=0; x<Main.image_width; x++)
          if (Read_pixel_from_current_layer(x,y) == old_color)
            Pixel_in_current_screen(x,y,new_color);
    }
  }
}



/******************************************************************************/
/********************************** SHADES ************************************/

// Transformer une liste de shade en deux tables de conversion
void Shade_list_to_lookup_tables(word * list,short step,byte mode,byte * table_inc,byte * table_dec)
{
  int index;
  int first;
  int last;
  int color;
  int temp;


  // On initialise les deux tables de conversion en Identité
  for (index=0;index<256;index++)
  {
    table_inc[index]=index;
    table_dec[index]=index;
  }

  // On s'apprête à examiner l'ensemble de la liste
  for (index=0;index<512;index++)
  {
    // On recherche la première case de la liste non vide (et non inhibée)
    while ((index<512) && (list[index]>255))
      index++;

    // On note la position de la première case de la séquence
    first=index;

    // On recherche la position de la dernière case de la séquence
    for (last=first;list[last+1]<256;last++);

    // Pour toutes les cases non vides (et non inhibées) qui suivent
    switch (mode)
    {
      case SHADE_MODE_NORMAL :
        for (;(index<512) && (list[index]<256);index++)
        { // On met à jour les tables de conversion
          color=list[index];
          table_inc[color]=list[(index+step<=last)?index+step:last];
          table_dec[color]=list[(index-step>=first)?index-step:first];
        }
        break;
      case SHADE_MODE_LOOP :
        temp=1+last-first;
        for (;(index<512) && (list[index]<256);index++)
        { // On met à jour les tables de conversion
          color=list[index];
          table_inc[color]=list[first+((step+index-first)%temp)];
          table_dec[color]=list[first+(((temp-step)+index-first)%temp)];
        }
        break;
      default : // SHADE_MODE_NOSAT
        for (;(index<512) && (list[index]<256);index++)
        { // On met à jour les tables de conversion
          color=list[index];
          if (index+step<=last)
            table_inc[color]=list[index+step];
          if (index-step>=first)
            table_dec[color]=list[index-step];
        }
    }
  }
}



// -- Interface avec l'image, affectée par le facteur de grossissement -------

  // fonction d'affichage "Pixel" utilisée pour les opérations définitivement
  // Ne doit à aucune condition être appelée en dehors de la partie visible
  // de l'image dans l'écran (ça pourrait être grave)
void Display_pixel(word x,word y,byte color)
  // x & y    sont la position d'un point dans l'IMAGE
  // color  est la couleur du point
  // Le Stencil est géré.
  // Les effets sont gérés par appel à Effect_function().
  // La Loupe est gérée par appel à Pixel_preview().
{
  if ( ( (!Sieve_mode)   || (Effect_sieve(x,y)) )
    && (!((Stencil_mode) && (Stencil[Read_pixel_from_current_layer(x,y)])))
    && (!((Mask_mode)    && (Mask_table[Read_pixel_from_spare_screen(x,y)]))) )
  {
    color=Effect_function(x,y,color);
    if (Main.tilemap_mode)
    {
      Tilemap_draw(x,y, color);
    }
    else
      Pixel_in_current_screen_with_preview(x,y,color);
  }
}



// -- Calcul des différents effets -------------------------------------------

  // -- Aucun effet en cours --

byte No_effect(word x, word y, byte color)
{
  (void)x; // unused
  (void)y; // unused

  return color;
}

  // -- Effet de Shading --

byte Effect_shade(word x,word y,byte color)
{
  (void)color; // unused

  return Shade_table[Read_pixel_from_feedback_screen(x,y)];
}

byte Effect_quick_shade(word x,word y,byte color)
{
  int c=color=Read_pixel_from_feedback_screen(x,y);
  int direction=(Fore_color<=Back_color);
  byte start,end;
  int width;

  if (direction)
  {
    start=Fore_color;
    end  =Back_color;
  }
  else
  {
    start=Back_color;
    end  =Fore_color;
  }

  if ((c>=start) && (c<=end) && (start!=end))
  {
    width=1+end-start;

    if ( ((Shade_table==Shade_table_left) && direction) || ((Shade_table==Shade_table_right) && (!direction)) )
      c-=Quick_shade_step%width;
    else
      c+=Quick_shade_step%width;

    if (c<start)
      switch (Quick_shade_loop)
      {
        case SHADE_MODE_NORMAL : return start;
        case SHADE_MODE_LOOP : return (width+c);
        default : return color;
      }

    if (c>end)
      switch (Quick_shade_loop)
      {
        case SHADE_MODE_NORMAL : return end;
        case SHADE_MODE_LOOP : return (c-width);
        default : return color;
      }
  }

  return c;
}

  // -- Effet de Tiling --

byte Effect_tiling(word x,word y,byte color)
{
  (void)color; // unused

  return Read_pixel_from_brush((x+Brush_width-Tiling_offset_X)%Brush_width,
                               (y+Brush_height-Tiling_offset_Y)%Brush_height);
}

  // -- Effet de Smooth --

byte Effect_smooth(word x,word y,byte color)
{
  int r,g,b;
  byte c;
  int weight,total_weight;
  byte x2=((x+1)<Main.image_width);
  byte y2=((y+1)<Main.image_height);
  (void)color; // unused

  // On commence par le pixel central
  c=Read_pixel_from_feedback_screen(x,y);
  total_weight=Smooth_matrix[1][1];
  r=total_weight*Main.palette[c].R;
  g=total_weight*Main.palette[c].G;
  b=total_weight*Main.palette[c].B;

  if (x)
  {
    c=Read_pixel_from_feedback_screen(x-1,y);
    total_weight+=(weight=Smooth_matrix[0][1]);
    r+=weight*Main.palette[c].R;
    g+=weight*Main.palette[c].G;
    b+=weight*Main.palette[c].B;

    if (y)
    {
      c=Read_pixel_from_feedback_screen(x-1,y-1);
      total_weight+=(weight=Smooth_matrix[0][0]);
      r+=weight*Main.palette[c].R;
      g+=weight*Main.palette[c].G;
      b+=weight*Main.palette[c].B;

      if (y2)
      {
        c=Read_pixel_from_feedback_screen(x-1,y+1);
        total_weight+=(weight=Smooth_matrix[0][2]);
        r+=weight*Main.palette[c].R;
        g+=weight*Main.palette[c].G;
        b+=weight*Main.palette[c].B;
      }
    }
  }

  if (x2)
  {
    c=Read_pixel_from_feedback_screen(x+1,y);
    total_weight+=(weight=Smooth_matrix[2][1]);
    r+=weight*Main.palette[c].R;
    g+=weight*Main.palette[c].G;
    b+=weight*Main.palette[c].B;

    if (y)
    {
      c=Read_pixel_from_feedback_screen(x+1,y-1);
      total_weight+=(weight=Smooth_matrix[2][0]);
      r+=weight*Main.palette[c].R;
      g+=weight*Main.palette[c].G;
      b+=weight*Main.palette[c].B;

      if (y2)
      {
        c=Read_pixel_from_feedback_screen(x+1,y+1);
        total_weight+=(weight=Smooth_matrix[2][2]);
        r+=weight*Main.palette[c].R;
        g+=weight*Main.palette[c].G;
        b+=weight*Main.palette[c].B;
      }
    }
  }

  if (y)
  {
    c=Read_pixel_from_feedback_screen(x,y-1);
    total_weight+=(weight=Smooth_matrix[1][0]);
    r+=weight*Main.palette[c].R;
    g+=weight*Main.palette[c].G;
    b+=weight*Main.palette[c].B;
  }

  if (y2)
  {
    c=Read_pixel_from_feedback_screen(x,y+1);
    total_weight+=(weight=Smooth_matrix[1][2]);
    r+=weight*Main.palette[c].R;
    g+=weight*Main.palette[c].G;
    b+=weight*Main.palette[c].B;
  }

  return (total_weight)? // On regarde s'il faut éviter le 0/0.
    Best_color(Round_div(r,total_weight),
                      Round_div(g,total_weight),
                      Round_div(b,total_weight)):
    Read_pixel_from_current_screen(x,y); // C'est bien l'écran courant et pas
                                       // l'écran feedback car il s'agit de ne
}                                      // pas modifier l'écran courant.

byte Effect_layer_copy(word x,word y,byte color)
{
  if (color<Main.backups->Pages->Nb_layers)
  {
    return Read_pixel_from_layer(color, x, y);
  }
  return Read_pixel_from_feedback_screen(x,y);
}

void Horizontal_grid_line(word x_pos,word y_pos,word width)
{
  int x;

  for (x=!(x_pos&1);x<width;x+=2)
    Pixel(x_pos+x, y_pos, xor_lut[Get_Screen_pixel((x_pos+x)*Pixel_width, (y_pos-1)*Pixel_height)]);
}

void Vertical_grid_line(word x_pos,word y_pos,word height)
{
  int y;

  for (y=!(y_pos&1);y<height;y+=2)
    Pixel(x_pos, y_pos+y, xor_lut[Get_Screen_pixel(x_pos*Pixel_width-1, (y_pos+y)*Pixel_height)]);
}

// Tile Grid
void Redraw_grid(short x, short y, unsigned short w, unsigned short h)
{
  int row, col;
  if (!Show_grid)
    return;

  row=y+((Snap_height*1000-(y-0)/Main.magnifier_factor-Main.magnifier_offset_Y+Snap_offset_Y-1)%Snap_height)*Main.magnifier_factor+Main.magnifier_factor-1;
  while (row < y+h)
  {
    Horizontal_grid_line(x, row, w);
    row+= Snap_height*Main.magnifier_factor;
  }

  col=x+((Snap_width*1000-(x-Main.X_zoom)/Main.magnifier_factor-Main.magnifier_offset_X+Snap_offset_X-1)%Snap_width)*Main.magnifier_factor+Main.magnifier_factor-1;
  while (col < x+w)
  {
    Vertical_grid_line(col, y, h);
    col+= Snap_width*Main.magnifier_factor;
  }
}

byte Read_pixel_from_current_screen(word x,word y)
{
  byte depth;
  byte color;

  if (Main.backups->Pages->Image_mode == IMAGE_MODE_ANIMATION)
  {
    return Read_pixel_from_current_layer(x, y);
  }

  if (Main.backups->Pages->Image_mode == IMAGE_MODE_MODE5
  	|| Main.backups->Pages->Image_mode == IMAGE_MODE_RASTER)
    if (Main.current_layer==4)
      return Read_pixel_from_current_layer(x, y);

  color = *(Main_screen+y*Main.image_width+x);
  if (color != Main.backups->Pages->Transparent_color) // transparent color
    return color;

  depth = *(Main_visible_image_depth_buffer.Image+x+y*Main.image_width);
  return Read_pixel_from_layer(depth, x, y);
}

/// Paint a a single pixel in image and optionnaly on screen: as-is.
static void Pixel_in_screen_direct_with_opt_preview(word x, word y, byte color, int preview)
{
  Pixel_in_current_layer(x, y, color);
  if (preview)
    Pixel_preview(x,y,color);
}

/// Paint a a single pixel in image and on optionnaly on screen : using layered display.
static void Pixel_in_screen_layered_with_opt_preview(word x,word y,byte color, int preview)
{
  byte depth = *(Main_visible_image_depth_buffer.Image+x+y*Main.image_width);
  Pixel_in_current_layer(x, y, color);
  if ( depth <= Main.current_layer)
  {
    if (color == Main.backups->Pages->Transparent_color) // transparent color
      // fetch pixel color from the topmost visible layer
      color = Read_pixel_from_layer(depth, x, y);

    *(x+y*Main.image_width+Main_screen)=color;

    if (preview)
      Pixel_preview(x,y,color);
  }
}

/// Paint in a specific layer and update optionnaly the screen
static void Pixel_in_layer_with_opt_preview(int layer, word x,word y,byte color, int preview)
{
  byte depth = *(Main_visible_image_depth_buffer.Image+x+y*Main.image_width);

  Pixel_in_layer(layer, x, y, color);
  // if (depth > layer) => another layer hides this one
  if (depth <= layer && ((1 << layer) & Main.layers_visible))
  {
    if (color == Main.backups->Pages->Transparent_color) // transparent color
      // fetch pixel color from the topmost visible layer
      color = Read_pixel_from_layer(depth, x, y);

    Main_screen[x+y*Main.image_width]=color;

    if (preview)
      Pixel_preview(x,y,color);
  }
}

/// @defgroup constraints Special constaints drawing modes
/// For 8bits machines modes (ZX Spectrum, C64, etc.)
/// @{

/// Paint a pixel in CPC EGX mode
///
/// even lines have 2x more pixel than odd lines, but less colors
static void Pixel_in_screen_egx_with_opt_preview(word x,word y,byte color,int preview)
{
  uint8_t mask;
  if (Main.backups->Pages->Image_mode == IMAGE_MODE_EGX)
  {
    mask = 0xF3;  // 11110011
  } else {
    mask = 0xFD;  // 11111101
  }

  if (y & 1)
  {
    Pixel_in_screen_layered_with_opt_preview(x & ~1,y,color,preview);
    Pixel_in_screen_layered_with_opt_preview(x |  1,y,color,preview);
  }
  else
    Pixel_in_screen_layered_with_opt_preview(x,y,color & mask,preview);
}

/// Paint a pixel in 40col Thomson MO/TO mode
/// or TMS9918 Graphics 2
///
/// Only 2 colors in a 8x1 pixel block
static void Pixel_in_screen_thomson_with_opt_preview(word x,word y,byte color,int preview)
{
  word start = x & 0xFFF8;
  word x2;
  uint8_t c1, c2;

  // The color we are going to replace
  c1 = Read_pixel_from_current_layer(x, y);

  if (c1 == color)
    return;

  for (x2 = 0; x2 < 8; x2++)
  {
    c2 = Read_pixel_from_current_layer(start+x2, y);
    if (c2 == color)
      continue;
    if (c2 != c1)
      break;
  }

  if (c2 == c1 || c2 == color)
  {
    // There was only one color, so we can add a second one.
    Pixel_in_screen_layered_with_opt_preview(x,y,color,preview);
    return;
  }

  for (x2 = 0; x2 < 8; x2++)
  {
    c2 = Read_pixel_from_current_layer(start+x2, y);
    if (c2 == c1) {
      Pixel_in_screen_layered_with_opt_preview(x2+start,y,color,preview);
    }
  }
}

/// Paint a pixel with 8x8 block constraints
///
/// Used for ZX Spectrum and C64 HiRes modes.
/// Only 2 colors in a 8x8 block, and for the ZX Spectrum both must be either bight or not.
static void Pixel_in_screen_zx_with_opt_preview(word x,word y,byte color,int preview)
{
  word start = x & 0xFFF8;
  word starty = y & 0xFFF8;
  word x2, y2;
  uint8_t c1, c2;

  // The color we are going to replace
  c1 = Read_pixel_from_current_layer(x, y);

  // Pixel is already of the wanted color: nothing to do
  if (c1 == color)
    return;

  // Check the whole cell
  for (x2 = 0; x2 < 8; x2++)
  for (y2 = 0; y2 < 8; y2++)
  {
    c2 = Read_pixel_from_current_layer(x2 + start, y2 + starty);
    // Pixel is already of the color we are going to add, it is no problem
    if (c2 == color)
      continue;
    // We have found another color, which is the one we will keep from the cell
    if (c2 != c1)
      goto done;
  }
done:

  if ((c2 == c1 || c2 == color))
  {
    // There was only one color, so we can add a second one

    // First make sure we have a single brightness
    if (Main.backups->Pages->Image_mode == IMAGE_MODE_ZX
       && (c2 & 8) != (color & 8))
    {
      for (x2 = 0; x2 < 8; x2++)
      for (y2 = 0; y2 < 8; y2++)
      {
        Pixel_in_screen_layered_with_opt_preview(x2+start,y2+starty,c2 ^ 8,preview);
      }
    }

    Pixel_in_screen_layered_with_opt_preview(x,y,color,preview);
    return;
  }

  // Replace all C1 with color
  for (x2 = 0; x2 < 8; x2++)
  for (y2 = 0; y2 < 8; y2++)
  {
    c2 = Read_pixel_from_current_layer(x2 + start, y2 + starty);
    if (c2 == c1)
      Pixel_in_screen_layered_with_opt_preview(x2+start,y2+starty,color,preview);
    else if (Main.backups->Pages->Image_mode == IMAGE_MODE_ZX)  // Force the brightness bit
      Pixel_in_screen_layered_with_opt_preview(x2+start,y2+starty,(c2 & ~8) | (color & 8),preview);
  }
}

/// Paint a pixel with GBC constraints
///
/// Same 4 color palette for all pixels in a 8x8 block.
///
/// Megadrive constraints are nearly the same : same 16 color palette in a 8x8 tile
static void Pixel_in_screen_gbc_with_opt_preview(word x,word y,byte color,int preview)
{
  word startx = x & ~7;
  word starty = y & ~7;
  word x2, y2;
  byte palette;
  byte col_mask, pal_mask;

  if (Main.backups->Pages->Image_mode == IMAGE_MODE_MEGADRIVE)
    col_mask = 15;
  else
    col_mask = 3;
  pal_mask = ~col_mask;

  // first set the pixel
  Pixel_in_screen_layered_with_opt_preview(x,y,color,preview);
  palette = color & pal_mask;
  // force all pixels of the block to the same palette
  for (y2 = 0; y2 < 8; y2++)
  {
    for (x2 = 0; x2 < 8; x2++)
    {
      byte col = Read_pixel_from_current_layer(startx+x2, starty+y2);
      if ((col & pal_mask) != palette)
        Pixel_in_screen_layered_with_opt_preview(startx+x2, starty+y2, palette | (col & col_mask), preview);
    }
  }
}

/// Paint a pixel with C64 MultiColor constraints
///
/// Only 4 colors in a 4x8 block, including the background color
/// which is common for all blocks.
///
/// @todo support for any background color (fixed to 0 now)
static void Pixel_in_screen_c64multi_with_opt_preview(word x,word y,byte color,int preview)
{
  word startx = x & ~3;
  word starty = y & ~7;
  word x2, y2;
  byte col, old_color;
  byte c[4] = { 0, 0, 0, 0 };  // palette of 4 colors for the block
  int i, n;

  old_color = Read_pixel_from_current_layer(x, y);
  if (old_color == color)
    return; // nothing to do if the color doesn't change !

  c[0] = 0; // assume background is 0
  n = 1;  // counted colors
  for (y2 = 0; y2 < 8; y2++)
  {
    for (x2 = 0; x2 < 4; x2++)
    {
      col = Read_pixel_from_current_layer(startx+x2, starty+y2);
      // search color in our mini 4 colors palette
      for (i = 0; i < n; i++)
      {
        if (col == c[i])
          break;  // found
      }
      if (i == n) // not found
      {
        if (n < 4)
          c[n++] = col; // set color in palette
        else  // already more than 3 colors (+ background) in the block. Fix it
          Pixel_in_screen_layered_with_opt_preview(startx+x2,starty+y2,color,preview);
      }
    }
  }
  if (n < 4)
  {
    // there is less than 4 colors in the block : nothing special to do
    Pixel_in_screen_layered_with_opt_preview(x,y,color,preview);
    return;
  }
  for (i = 0; i < n; i++)
    if (color == c[i])
    {
      // The new color is already in the palette, nothing special to do
      Pixel_in_screen_layered_with_opt_preview(x,y,color,preview);
      return;
    }
  // The execution reaches this point only if plotting the new color
  // would violate the constraints.
  // replace old_color with color, except if old_color is the background.
  // replace the last color of the palette instead.
  if (old_color == c[0])  // background
    old_color = c[3];
  for (y2 = 0; y2 < 8; y2++)
  {
    for (x2 = 0; x2 < 4; x2++)
    {
      col = Read_pixel_from_current_layer(startx+x2, starty+y2);
      if (col == old_color)
        Pixel_in_screen_layered_with_opt_preview(startx+x2,starty+y2,color,preview);
    }
  }
}


/// Paint in the background or Color RAM layer of C64 FLI
///
/// Update the bitmap layer pixel if needed
static void Pixel_in_screen_c64fli_underlay_with_opt_preview(word x,word y,byte color,int preview)
{
  byte oldcolor = Read_pixel_from_current_layer(x, y);

  // does it changes the upper layer (3) ?
  if (oldcolor != Read_pixel_from_layer(Main.current_layer ^ 1, x, y)
      && oldcolor == Read_pixel_from_layer(2, x, y))
  {
    Pixel_in_layer(2, x, y, color);
    if (((1 << 2) & Main.layers_visible) != 0
      && (Main_visible_image_depth_buffer.Image[+x+y*Main.image_width] <= 2))
    {
      Main_screen[x+y*Main.image_width]=color;
      if (preview)
        Pixel_preview(x,y,color);
    }
  }

  Pixel_in_screen_layered_with_opt_preview(x, y, color, preview);
}

/// Paint in the bitmap layer of C64 FLI
///
/// enforce C64 FLI mode constraints.
static void Pixel_in_screen_c64fli_bitmap_with_opt_preview(word x,word y,byte color,int preview)
{
  word startx = x & ~3;
  word x2;
  byte c[4];
  byte oldcolor = Read_pixel_from_current_layer(x, y);

  if (oldcolor == color)
    return; // nothing to do !
  c[0] = Read_pixel_from_layer(0, x, y);
  c[1] = c[0];
  c[2] = c[0];
  c[3] = Read_pixel_from_layer(1, x, y);

  Pixel_in_screen_layered_with_opt_preview(x,y,color,preview);

  // if the color is the background or the color RAM color,
  // no clash is possible !
  if (color == c[0] || color == c[1])
    return;

  // check the number of color used
  for (x2 = startx; x2 < startx+4; x2++)
  {
    int i;
    byte col = Read_pixel_from_current_layer(x2, y);
    // look for the color in the 4 color "palette"
    for (i = 0; i < 4; i++)
    {
      if (col == c[i])
        break;
    }
    if (i >= 4)
    {
      if (c[1] == c[0])
        c[1] = col; // assign 1st color of Screen RAM
      else if (c[2] == c[0])
        c[2] = col; // assign 2nd color of Screen RAM
      else // color clash !
      {
        if (oldcolor == c[0] || oldcolor == c[3])
          oldcolor = c[2];  // pick another one !
        for (x2 = startx; x2 < startx+4; x2++)
        {
          col = Read_pixel_from_current_layer(x2, y);
          if (col == oldcolor)
            Pixel_in_screen_layered_with_opt_preview(x2,y,color,preview);
        }
        return;
      }
    }
  }
}

/// Paint a a single pixel in image and optionnaly on screen : in a layer under one that acts as a layer-selector (mode 5).
static void Pixel_in_screen_underlay_with_opt_preview(word x,word y,byte color,int preview)
{
  byte depth;

  // Paste in layer
  Pixel_in_current_layer(x, y, color);
  // Search depth
  depth = Read_pixel_from_layer(4, x, y);

  if ( depth == Main.current_layer)
  {
    // Draw that color on the visible image buffer
    *(x+y*Main.image_width+Main_screen)=color;

    if (preview)
      Pixel_preview(x,y,color);
  }
}

/// Paint a single pixel in the layer 5 of CPC rasterized modes
///
/// used when @ref IMAGE_MODE_MODE5 or @ref IMAGE_MODE_RASTER is active.
///
/// The layer 5 acts as a layer-selector.
/// it contains INK 0 to 3 which select one of the layer from 1 to 4, the
/// display color is retrieved from this one.
static void Pixel_in_screen_overlay_with_opt_preview(word x,word y,byte color,int preview)
{
  byte ink; // 0 to 3

  if (color < 4)
    ink = color;  // the argument was the ink!
  else
  {
    // search for this color in the 4 defined inks
    for (ink = 0; ink < 4; ink++)
    {
      if (color == Read_pixel_from_layer(ink, x, y))
        break;
    }
    if (ink >= 4)
      return; // not found ? do nothing
  }

  // Paste ink in layer 5
  Pixel_in_current_layer(x, y, ink);
  // Paste in depth buffer
  *(Main_visible_image_depth_buffer.Image+x+y*Main.image_width) = ink;
  // Fetch pixel color from the target raster layer
  if (Main.layers_visible & (1 << ink))
    color = Read_pixel_from_layer(ink, x, y);
  else
    color = ink;

  // Draw that color on the visible image buffer
  *(x+y*Main.image_width+Main_screen)=color;

  if (preview)
    Pixel_preview(x,y,color);
}

/// generate color pixels in layer 2 from the monochrome layer 1
///
/// For Apple II HGR mode ::IMAGE_MODE_HGR
void Update_color_hgr_pixel(word x, word y, int preview)
{
  byte b2, b1, b0, pal;

  // read monochrome pixels
  b1 = Read_pixel_from_layer(0, x, y);
  b2 = (x > 0) ? Read_pixel_from_layer(0, x - 1, y) : 0;
  b0 = (x < Main.image_width - 1) ? Read_pixel_from_layer(0, x + 1, y) : 0;
  pal = b1 & 4;
  switch (((b2 & 1) << 2) | ((b1 & 1) << 1) | (b0 & 1))
  {
    case 7: // 111
    case 6: // 110
    case 3: // 011
      Pixel_in_layer_with_opt_preview(1, x, y, pal + 3, preview); // white
      break;
    case 0: // 000
    case 1: // 001
    case 4: // 100
      Pixel_in_layer_with_opt_preview(1, x, y, pal, preview); // black
      break;
    default:  // 010 or 101
      Pixel_in_layer_with_opt_preview(1, x, y, pal + 1 + ((x & 1) ^ (b1 & 1)), preview); // black
  }
}

/// Paint a pixel in HGR mode in the monochrome layer
///
/// - Layer 1 is the monochrome screen. Pixels are either black or white. 2 different values to reflect high bit
/// - Layer 2 is the screen seen as color
static void Pixel_in_screen_hgr_mono_with_opt_preview(word x,word y,byte color,int preview)
{
  byte oldcolor;
  word column;
  int x2;

  if (color >= 8)
    return;
  if ((color & 3) != 0)
    color |= 3; // force black or white.

  // put pixel
  oldcolor = Read_pixel_from_layer(0, x, y);
  if (color == oldcolor)
    return; // nothing to do !
  Pixel_in_layer_with_opt_preview(0, x, y, color, preview);

  if ((color & 4) == (oldcolor & 4))
  { // no palette change
    if (x > 0)
      Update_color_hgr_pixel(x - 1, y, preview);
    Update_color_hgr_pixel(x, y, preview);
    if (x < Main.image_width - 1)
      Update_color_hgr_pixel(x + 1, y, preview);
    return;
  }

  column = x / 7;
  // update the palette bit of the whole column (byte)
  for (x2 = column * 7; x2 < column * 7 + 7; x2++)
  {
    byte pixel = Read_pixel_from_layer(0, x2, y);
    Pixel_in_layer_with_opt_preview(0, x2, y, (color & 4) | (pixel & 3), preview);
  }
  // update color pixels !
  for (x2 = MAX(0, (column * 7) - 1); x2 < column * 7 + 8; x2++)
  {
    if (x2 >= Main.image_width)
      break;
    Update_color_hgr_pixel((word)x2, y, preview);
  }
}

/// Paint in the color layer of HGR.
///
/// - For B&W the 1st press change the pixel,
/// the second one change the other pixel of the pair.
/// - For colors, change both monochrome pixels
static void Pixel_in_screen_hgr_color_with_opt_preview(word x,word y,byte color,int preview)
{
  byte oldcolor;
  switch (color & 3)
  {
    case 0: // black
    case 3: // white
      oldcolor = Read_pixel_from_layer(0, x, y);
      if (oldcolor == color)
        Pixel_in_screen_hgr_mono_with_opt_preview(x ^ 1, y, color, preview);
      else
        Pixel_in_screen_hgr_mono_with_opt_preview(x, y, color, preview);
      break;
    default:
      x &= ~1;
      Pixel_in_screen_hgr_mono_with_opt_preview(x, y, color & 6, preview);      // palette bit(4) + upper bit(2)
      Pixel_in_screen_hgr_mono_with_opt_preview(x + 1, y, color & 5, preview);  // palette bit(4) + lower bit(1)
  }
}

/// Update the color layer of DHGR according to the monochrome layer
///
/// Emulate \"Le Chat Mauve\" mode 3 (mixed mode).
void Update_color_dhgr_pixel(word x, word y, int preview)
{
  byte b3, b2, b1, b0, color;

  x &= ~3;
  // read monochrome pixels
  b3 = Read_pixel_from_layer(0, x, y);
  b2 = Read_pixel_from_layer(0, x + 1, y);
  b1 = Read_pixel_from_layer(0, x + 2, y);
  b0 = Read_pixel_from_layer(0, x + 3, y);
  if (b3 & 16)
  {
    // monochrome pixel
    Pixel_in_layer_with_opt_preview(1, x, y, b3, preview);
    Pixel_in_layer_with_opt_preview(1, x + 1, y, b2, preview);
    Pixel_in_layer_with_opt_preview(1, x + 2, y, b1, preview);
    Pixel_in_layer_with_opt_preview(1, x + 3, y, b0, preview);
  }
  else
  {
    // color pixel
    color = (b3 & 8) | (b2 & 4) | (b1 & 2) | (b0 & 1);
    Pixel_in_layer_with_opt_preview(1, x, y, color, preview);
    Pixel_in_layer_with_opt_preview(1, x + 1, y, color, preview);
    Pixel_in_layer_with_opt_preview(1, x + 2, y, color, preview);
    Pixel_in_layer_with_opt_preview(1, x + 3, y, color, preview);
  }
}


/// Paint in the monochrome layer of DHGR
///
/// also update the color pixels.
static void Pixel_in_screen_dhgr_mono_with_opt_preview(word x,word y,byte color,int preview)
{
  byte oldcolor;

  if (color >= 32)
    return;
  if ((color & 15) != 0)
    color |= 15; // force black or white.

  // put pixel
  oldcolor = Read_pixel_from_layer(0, x, y);
  if (color == oldcolor)
    return; // nothing to do !
  Pixel_in_layer_with_opt_preview(0, x, y, color, preview);
  Update_color_dhgr_pixel(x, y, preview);

  // change bit7 if needed.
  if ((color & 16) != (oldcolor & 16))
  {
    int i;
    x -= (x % 7);
    for (i = 0; i < 7; i++)
    {
      oldcolor = Read_pixel_from_layer(0, x, y);
      if ((oldcolor & 16) != (color & 16))
      {
        Pixel_in_layer_with_opt_preview(0, x, y, (color & 16) | (oldcolor & 15), preview);
        Update_color_dhgr_pixel(x, y, preview);
      }
      x++;
    }
  }
}

/// Paint in the color layer of DHGR
///
/// use of color 16-31 forces the cell to be monochrome.
static void Pixel_in_screen_dhgr_color_with_opt_preview(word x,word y,byte color,int preview)
{
  if (color & 16)
  {
    // monochrome pixel
    Pixel_in_screen_dhgr_mono_with_opt_preview(x, y, color, preview);
    // force monochrome for this cell
    if ((x & 3) != 0)
      Pixel_in_screen_dhgr_mono_with_opt_preview(x & ~3, y, Read_pixel_from_layer(0, x & ~3, y) | 16, preview);
  }
  else
  {
    // color pixel
    x &= ~3;
    Pixel_in_screen_dhgr_mono_with_opt_preview(x, y, color & 8, preview); // also set this cell in color mode
    Pixel_in_screen_dhgr_mono_with_opt_preview(x + 1, y, color & 4, preview);
    Pixel_in_screen_dhgr_mono_with_opt_preview(x + 2, y, color & 2, preview);
    Pixel_in_screen_dhgr_mono_with_opt_preview(x + 3, y, color & 1, preview);
  }
}

// end of constraints group
/// @}

Func_pixel_opt_preview Pixel_in_current_screen_with_opt_preview=Pixel_in_screen_direct_with_opt_preview;

/**
 * Put a pixel in the current layer of a "Document"
 *
 * @param doc pointer to either @ref Main or @ref Spare
 * @param x x coordinate of the pixel to put
 * @param y y coordinate of the pixel to put
 * @param color the new color for the pixel
 */
void Pixel_in_document_current_layer(T_Document * doc, word x, word y, byte color)
{
  doc->backups->Pages->Image[doc->current_layer].Pixels[x + y*doc->image_width] = color;
}

void Pixel_in_spare(word x,word y, byte color)
{
  Pixel_in_document_current_layer(&Spare, x, y, color);
}

void Pixel_in_current_layer(word x,word y, byte color)
{
  Pixel_in_document_current_layer(&Main, x, y, color);
}

/// put a pixel in a specific layer of Main Page
void Pixel_in_layer(int layer, word x,word y, byte color)
{
  T_Document * doc = &Main;
  doc->backups->Pages->Image[layer].Pixels[x + y*doc->image_width] = color;
}

byte Read_pixel_from_current_layer(word x,word y)
{
  return Read_pixel_from_layer(Main.current_layer, x, y);
}

/// Read a pixel from a specific layer of Main Page
byte Read_pixel_from_layer(int layer, word x,word y)
{
  return Main.backups->Pages->Image[layer].Pixels[x + y*Main.image_width];
}

void Update_pixel_renderer(void)
{
  switch (Main.backups->Pages->Image_mode)
  {
  case IMAGE_MODE_ANIMATION:
    // direct
    Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_direct_with_opt_preview;
    break;
  case IMAGE_MODE_LAYERED:
    // layered
    Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_layered_with_opt_preview;
    break;
  case IMAGE_MODE_EGX:
  case IMAGE_MODE_EGX2:
    // special "EGX" mode
    Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_egx_with_opt_preview;
    break;
  case IMAGE_MODE_THOMSON:
  case IMAGE_MODE_TMS9918G2:
    Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_thomson_with_opt_preview;
    break;
  case IMAGE_MODE_GBC:
  case IMAGE_MODE_MEGADRIVE:
    Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_gbc_with_opt_preview;
    break;
  case IMAGE_MODE_C64HIRES:
  case IMAGE_MODE_ZX:
    Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_zx_with_opt_preview;
    break;
  case IMAGE_MODE_C64MULTI:
    Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_c64multi_with_opt_preview;
    break;
  case IMAGE_MODE_MODE5:
  case IMAGE_MODE_RASTER:
    if ( Main.current_layer == 4)                                     // overlay
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_overlay_with_opt_preview;
    else if (Main.current_layer<4 && (Main.layers_visible & (1<<4)))  // underlay
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_underlay_with_opt_preview;
    else                              // layered (again, for layers > 4 in MODE5 and RASTER)
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_layered_with_opt_preview;
    break;
  case IMAGE_MODE_C64FLI:
    if (Main.current_layer < 2)
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_c64fli_underlay_with_opt_preview;
    else if (Main.current_layer == 2)
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_c64fli_bitmap_with_opt_preview;
    else
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_layered_with_opt_preview;
    break;
  case IMAGE_MODE_HGR:
    if (Main.current_layer == 0)  // monochrome layer
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_hgr_mono_with_opt_preview;
    else if (Main.current_layer == 1)  // color layer
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_hgr_color_with_opt_preview;
    else
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_layered_with_opt_preview;
    break;
  case IMAGE_MODE_DHGR:
    if (Main.current_layer == 0)  // monochrome layer
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_dhgr_mono_with_opt_preview;
    else if (Main.current_layer == 1)  // color layer
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_dhgr_color_with_opt_preview;
    else
      Pixel_in_current_screen_with_opt_preview = Pixel_in_screen_layered_with_opt_preview;
  }
}
