#include <clutter/clutter.h>
#include <clutter-gst/clutter-gst.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void alarm_catcher (int sig) 
{
  exit(-1);
}

int
main (int argc, char *argv[])
{
  ClutterActor *video;
  GdkPixbuf    *shot;
  gint          duration;
  struct sigaction sact;

  sigemptyset( &sact.sa_mask );
  sact.sa_flags = 0;
  sact.sa_handler = alarm_catcher;
  sigaction( SIGALRM, &sact, NULL );

  alarm(120); /* If we are left running for more than 2 minutes something
	       * is really wrong and thus attempt to abort. 
	      */

  gst_init (&argc, &argv);
  clutter_init (&argc, &argv);

  if (argc < 3)
    {
      g_print ("Usage: %s <path to movie file> <output png>\n", argv[0]);
      exit(-1);
    }

  video = clutter_gst_video_texture_new ();

  clutter_media_set_filename(CLUTTER_MEDIA(video), argv[1]);
  clutter_media_set_volume (CLUTTER_MEDIA(video), 0);
  clutter_media_set_playing (CLUTTER_MEDIA(video), TRUE);

  do {

    while (g_main_context_pending (NULL))
      g_main_context_iteration (NULL, FALSE);

    duration = clutter_media_get_duration (CLUTTER_MEDIA(video));

  } while (duration == 0);

  clutter_actor_realize (video);

  clutter_media_set_position (CLUTTER_MEDIA(video), duration/3);

  do {

    while (g_main_context_pending (NULL))
      g_main_context_iteration (NULL, FALSE);

  } while (clutter_media_get_position (CLUTTER_MEDIA(video)) <= duration/3);

  shot = clutter_texture_get_pixbuf (CLUTTER_TEXTURE(video));
      
  if (shot)
    {
      GdkPixbuf *thumb, *pic;
      gint       x, y, nw, nh, w, h, size;
      
      size = 128;

      w = clutter_actor_get_width (video);
      h = clutter_actor_get_height (video);
      
      nh = ( h * size) / w;
      
      if (nh <= size)
	{
	  nw = size;
	  x = 0;
	  y = (size - nh) / 2;
	}
      else
	{
	  nw  = ( w * size ) / h;
	  nh = size;
	  x = (size - nw) / 2;
	  y = 0;
	}

      thumb = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, size, size);
      gdk_pixbuf_fill (thumb, 0x000000FF);

      pic = gdk_pixbuf_scale_simple (shot, nw, nh, GDK_INTERP_BILINEAR);
      gdk_pixbuf_copy_area  (pic, 0, 0, nw, nh, thumb, x, y);
      
      if (!gdk_pixbuf_save (thumb, argv[2], "png", NULL, NULL))
	{
	  g_error ("Pixbuf save failed\n", argv[0]);
	  exit(-1);
	}

      g_object_unref (shot);
      g_object_unref (thumb);
      g_object_unref (pic);

      exit(0);
    }

  exit (-1);
}
