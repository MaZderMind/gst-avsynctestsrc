plugin_LTLIBRARIES = libgstavsynctestsrc.la

libgstavsynctestsrc_la_SOURCES = \
        avsynctestvideosrc.c \
        avsynctestvideosrc.h \
        avsynctestaudiosrc.c \
        avsynctestaudiosrc.h \
        avsynctestsrc-plugin.c


# compiler and linker flags used to compile this plugin, set in configure.ac
libgstavsynctestsrc_la_CFLAGS = $(GST_CFLAGS)
libgstavsynctestsrc_la_LIBADD = \
        $(GST_LIBS) \
        $(GST_PLUGINS_BASE_LIBS) \
        -lgstvideo-1.0 \
        -lgstaudio-1.0
libgstavsynctestsrc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstavsynctestsrc_la_LIBTOOLFLAGS = --tag=disable-static
