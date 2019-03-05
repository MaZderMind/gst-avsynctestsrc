#!/bin/sh
GST_PLUGIN_PATH=`dirname $0`/../src/.libs/ GST_DEBUG="*:2,basesrc:7,avsynctestvideosrc:5" gst-launch-1.0 \
	avsynctestvideosrc ! video/x-raw,framerate=5/1,width=1024,height=576 ! timeoverlay ! ximagesink


# ximagesink - BGRx
# glimagesinkelement - RGBA
# xvimagesink - YV12