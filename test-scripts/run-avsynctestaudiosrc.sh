#!/bin/sh
GST_PLUGIN_PATH=`dirname $0`/../src/.libs/ GST_DEBUG="*:2,avsynctestaudiosrc:5" gst-launch-1.0 \
	avsynctestaudiosrc ! audio/x-raw,rate=48000,channels=1 ! autoaudiosink
