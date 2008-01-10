
#include <glib.h>
#include <libastro-desktop/astro.h>
#include <libastro-desktop/astro-application.h>

#include "astro-example.h"


AstroApplication *
astro_application_factory_init ()
{
  AstroApplication *app;
  GdkPixbuf *pixbuf;
 
  pixbuf = gdk_pixbuf_new_from_file (PKGDATADIR "/icons/exec.png", NULL);

  app = astro_example2_new ("Example Application", pixbuf);

  g_debug ("Example applet loaded\n");

  return app;
}
