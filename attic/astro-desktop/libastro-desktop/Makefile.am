INCLUDES = \
	-I$(srcdir) 				\
	-I$(top_srcdir) 			\
	-DPREFIX=\""$(prefix)"\" 		\
	-DLIBDIR=\""$(libdir)"\" 		\
	-DDATADIR=\""$(datadir)"\"		\
	$(GCC_CFLAGS) 				\
	$(DEPS_CFLAGS) 	

lib_LTLIBRARIES = libastro-desktop.la

libastro_desktop_la_SOURCES = \
	astro-application.c \
	astro-behave.c \
	astro-utils.c \
	astro-window.c \
	tidy-texture-frame.c

libastro_desktop_la_LIBADD = $(DEPS_LIBS)
libastro_desktop_la_LDFLAGS = $(DEPS_LT_LDFLAGS) -version-info 0:1:0
