/*
Copyright (c) 2010, Intel Corporation
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

• Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
• Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
• Neither the name of Intel Corporation nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <clutter/clutter.h>
#ifdef USE_CLUTTER_GST
#include <clutter-gst/clutter-gst.h>
#endif
#include <string.h>

#define RESTDEPTH   -9000.0
#define RESTX        4600.0
#define STARTPOS    -3000.0

static ClutterColor white = {0xff,0xff,0xff,0xff};
static ClutterColor black = {0x00,0x00,0x00,0xff};
static ClutterColor red   = {0x00,0x00,0xff,0xff};
static char *font        = "Sans 60px";
static char *default_bg  = "";

typedef struct PinPointPoint
{
  ClutterActor   *background;
  ClutterActor   *text;
  ClutterGravity  position;
} PinPointPoint;

static GList *slides      = NULL; /* list of slide texts */
static GList *slidep      = NULL; /* current slide */

static ClutterActor *stage;       /* stage */
static ClutterActor *background;
static ClutterActor *shading;
static ClutterActor *foreground;

static void leave_slide  (void);
static void show_slide   (void);
static void parse_slides (const char *slide_src);

static void stage_resized (ClutterActor *actor,
                           GParamSpec   *pspec,
                           gpointer      data)
{
  show_slide ();
}

static gboolean key_pressed (ClutterActor *actor,
                             ClutterEvent *event,
                             gpointer      data);

int main (int argc, char **argv)
{
  char   *text = NULL;

  if (!argv[1])
    {
      g_print ("usage: %s <slides>\n", argv[0]);
      text = g_strdup ("--\nusage: pinpoint <slides.txt>\n--");
    }
  else
    {
      if (!g_file_get_contents (argv[1], &text, NULL, NULL))
        {
          g_print ("failed to load slides from %s\n", argv[1]);
          return -1;
        }
    }

#ifdef USE_CLUTTER_GST
  clutter_gst_init (&argc, &argv);
#else
  clutter_init (&argc, &argv);
#endif
  stage = CLUTTER_ACTOR (clutter_stage_new ());

  background = clutter_group_new ();
  foreground = clutter_group_new ();
  shading = clutter_rectangle_new_with_color (&black);
  clutter_actor_set_opacity (shading, 0x77);

  clutter_container_add (CLUTTER_CONTAINER (stage),
                         background, shading, foreground, NULL);

  parse_slides (text);
  g_free (text);

  clutter_actor_show (stage);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &black);
  g_signal_connect (stage, "key-press-event", G_CALLBACK (key_pressed), NULL);
  g_signal_connect (stage, "notify::width", G_CALLBACK (stage_resized), NULL);
  g_signal_connect (stage, "notify::height", G_CALLBACK (stage_resized), NULL);

  clutter_stage_set_user_resizable (CLUTTER_STAGE (stage), TRUE);

  /* start with first slide */
  slidep = slides;

  clutter_main ();

  clutter_actor_destroy (stage);
  g_list_free (slides);

  return 0;
}


static gboolean key_pressed (ClutterActor *actor,
                             ClutterEvent *event,
                             gpointer      data)
{


  if (event) /* There is no event for the first triggering */
  switch (clutter_event_get_key_symbol (event))
    {
      case CLUTTER_Left:
      case CLUTTER_Up:
      case CLUTTER_BackSpace:
        if (slidep->prev)
          {
            leave_slide ();
            slidep = slidep->prev;
            show_slide ();
          }
        break;
      case CLUTTER_Right:
      case CLUTTER_space:
        if (slidep->next)
          {
            leave_slide ();
            slidep = slidep->next;
            show_slide ();
          }
        break;
      case CLUTTER_Escape:
        clutter_main_quit ();
        break;
      case CLUTTER_F11:
        clutter_stage_set_fullscreen (CLUTTER_STAGE (stage),
                 !clutter_stage_get_fullscreen (CLUTTER_STAGE (stage)));
        break;
    }
  return TRUE;
}


static void leave_slide (void)
{
  PinPointPoint *point = slidep->data;
  float resty;

  resty = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (point->text),
                                              "rest-y"));
  clutter_actor_animate (point->text,
                         CLUTTER_LINEAR, 2000,
                         "depth", RESTDEPTH,
                         "scale-x", 1.0,
                         "scale-y", 1.0,
                         "x", RESTX,
                         "y", resty,
                         NULL);
  if (point->background)
    {
      clutter_actor_animate (point->background,
                             CLUTTER_LINEAR, 1000,
                             "opacity", 0x0,
                             NULL);
#ifdef USE_CLUTTER_GST
      if (CLUTTER_GST_IS_VIDEO_TEXTURE (point->background))
        {
          clutter_media_set_playing (CLUTTER_MEDIA (point->background), FALSE);
        }
#endif
    }
}


static void show_slide (void)
{
  PinPointPoint *point = slidep->data;
  if (point->background)
    {
      float w,h;
      float x,y;
      float s = 1.0;
      clutter_actor_get_size (point->background, &w, &h);
      s = clutter_actor_get_width (stage)/w;
      if (s > 1.0 || clutter_actor_get_height (stage) < s * h)
        s = clutter_actor_get_height (stage)/h;
      clutter_actor_set_scale (point->background, s, s);
      x = (clutter_actor_get_width (stage)-w*s)/2;
      y = (clutter_actor_get_height (stage)-h*s)/2;
      clutter_actor_set_position (point->background, x, y);
      clutter_actor_animate (point->background,
                             CLUTTER_LINEAR, 1000,
                             "opacity", 0xff,
                             NULL);

#ifdef USE_CLUTTER_GST
      if (CLUTTER_GST_IS_VIDEO_TEXTURE (point->background))
        {
          clutter_media_set_progress (CLUTTER_MEDIA (point->background), 0.0);
          clutter_media_set_playing (CLUTTER_MEDIA (point->background), TRUE);
          clutter_actor_set_size (point->background, 320, 200);
        }
#endif
    }

  {
    float w, h;
    float x, y;
    float sx = 1.0;
    float sy = 1.0;
    float padding = clutter_actor_get_width (stage) * 0.01;

    clutter_actor_get_size (point->text, &w, &h);

    sx = clutter_actor_get_width (stage) / w * 0.8;
    sy = clutter_actor_get_height (stage) / h * 0.8;

    if (sy < sx)
      sx = sy;
    if (sx > 1.0) /* avoid enlarging text */
      sx = 1.0;

    x = (clutter_actor_get_width (stage)-w*sx)/2;
    y = (clutter_actor_get_height (stage)-h*sx)/2;

    clutter_actor_animate (point->text,
                           CLUTTER_EASE_OUT_QUINT, 1000,
                           "depth", 0.0,
                           "scale-x", sx,
                           "scale-y", sx,
                           "x", x,
                           "y", y,
                           NULL);

    if (clutter_actor_get_width (point->text) > 1.0)
      clutter_actor_animate (shading,
             CLUTTER_LINEAR, 500,
             "x", x - padding,
             "y", y - padding,
             "opacity", 0x88,
             "width", clutter_actor_get_width (point->text) * sx + padding*2,
             "height", clutter_actor_get_height (point->text) * sx + padding*2,
             NULL);
    else /* no text, fade out shading */
      clutter_actor_animate (shading,
             CLUTTER_LINEAR, 500,
             "opacity", 0,
             "x", x - padding,
             "y", y - padding,
             "width", clutter_actor_get_width (point->text) * sx + padding*2,
             "height", clutter_actor_get_height (point->text) * sx + padding*2,
             NULL);
  }
}

static void
parse_config (const char *config)
{
  GString *str2 = g_string_new ("");
  const char *p;
  for (p = config; *p; p++)
    {
      switch (*p)
        {
          case '\n':
            g_print ("{%s}\n", str2->str);
            if (g_str_has_prefix (str2->str, "font"))
              {
                font = g_strdup (strrchr (str2->str, '=') + 1);
              }
            else if (g_str_has_prefix (str2->str, "background"))
              {
                default_bg = g_strdup (strrchr (str2->str, '=') + 1);
              }
            g_string_truncate (str2, 0);
            break;
          default:
            g_string_append_c (str2, *p);
            break;
        }
    }
  g_string_free (str2, TRUE);
}

static void
parse_slides (const char *slide_src)
{
  const char *p;
  int      y = STARTPOS;
  gboolean startofline = TRUE;
  gboolean gotconfig = FALSE;
  GString *str = g_string_new ("");
  GString *wpstr = g_string_new ("");

  /* parse the slides, constructing lists of objects, adding all generated
   * actors to the stage
   */
  for (p = slide_src; *p; p++)
    {
      switch (*p)
      {
        case '\\': /* escape the next char */
          p++;
          startofline = FALSE;
          if (*p)
            g_string_append_c (str, *p);
          break;
        case '\n':
          startofline = TRUE;
          g_string_append_c (str, *p);
          break;
        case '[': /* starting a background for the slide */
          p++;
          g_string_assign (wpstr, "");
          while (*p && *p!='\n' && *p!=']')
           {
              g_string_append_c (wpstr, *p);
              p++;
           }
          break;
        case '-': /* slide seperator */
          if (startofline)
            {
              while (*p && *p!='\n')  /* until newline */
                p++;

              if (!gotconfig)
                {
                  parse_config (str->str);
                  gotconfig = TRUE;
                  g_string_assign (str, "");
                  g_string_assign (wpstr, "");
                }
              else
                {
                  PinPointPoint *point = g_new0 (PinPointPoint, 1);
                  point->position = CLUTTER_GRAVITY_CENTER;
                 
                  if (wpstr->str[0])
                    {
#ifdef USE_CLUTTER_GST
                      if (g_str_has_suffix (wpstr->str, ".avi")
                       || g_str_has_suffix (wpstr->str, ".ogg")
                       || g_str_has_suffix (wpstr->str, ".ogv")
                       || g_str_has_suffix (wpstr->str, ".mpg")
                       || g_str_has_suffix (wpstr->str, ".mov")
                       || g_str_has_suffix (wpstr->str, ".mp4")
                       || g_str_has_suffix (wpstr->str, ".wmv")
                       || g_str_has_suffix (wpstr->str, ".AVI")
                       || g_str_has_suffix (wpstr->str, ".OGG")
                       || g_str_has_suffix (wpstr->str, ".OGV")
                       || g_str_has_suffix (wpstr->str, ".MPG")
                       || g_str_has_suffix (wpstr->str, ".MOV")
                       || g_str_has_suffix (wpstr->str, ".MP4")
                       || g_str_has_suffix (wpstr->str, ".WMV"))
                        {
                          point->background = clutter_gst_video_texture_new ();
                          clutter_media_set_filename (CLUTTER_MEDIA (point->background), wpstr->str);
                          /* should pre-roll the video and set the size */
                          clutter_actor_set_size (point->background, 400, 300);
                        }
                      else
#endif
                        {
                          point->background = g_object_new (
                                  CLUTTER_TYPE_TEXTURE,
                                  "filename", wpstr->str,
                                  "load-data-async", TRUE,
                                  NULL);
                        }
                    }
                  else if (default_bg[0])
                    {
                      /* should reuse clones of a default loaded texture */
                      point->background = g_object_new (CLUTTER_TYPE_TEXTURE,
                              "filename", default_bg,
                              "load-data-async", TRUE,
                              NULL);
                    }

                  if (str->str[strlen(str->str)-1]=='\n')
                      str->str[strlen(str->str)-1]='\0';

                  point->text= clutter_text_new_with_text (font, str->str);
                  clutter_text_set_color (CLUTTER_TEXT (point->text), &white);

                  slides = g_list_append (slides, point);

                  clutter_container_add_actor (CLUTTER_CONTAINER (foreground),
                                               point->text);

                  if (point->background)
                    {
                      clutter_container_add_actor (
                         CLUTTER_CONTAINER (background), point->background);
                      clutter_actor_set_opacity (point->background, 0);
                    }

                  clutter_actor_set_position (point->text, RESTX, y);
                  g_object_set_data (G_OBJECT (point->text), "rest-y",
                                     GINT_TO_POINTER (y));
                  y += clutter_actor_get_height (point->text);
                  clutter_actor_set_depth (point->text, RESTDEPTH);

                  g_string_assign (str, "");
                  g_string_assign (wpstr, "");
                }
            }
          else
            {
              startofline = FALSE;
              g_string_append_c (str, *p);
            }
          break;
        default:
          startofline = FALSE;
          g_string_append_c (str, *p);
          break;
      }
    }
}