#!/bin/sh
GST_PLUGIN_PATH=`dirname $0`/../src/.libs/ GST_DEBUG="*:2,avsynctestvideosrc:5" gst-launch-1.0 \
	avsynctestvideosrc ! video/x-raw,width=320,height=240,framerate=25/1 ! autovideosink
