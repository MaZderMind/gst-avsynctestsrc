/* GStreamer
 * Copyright (C) 2019 Peter Körner <peter@mazdermind.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "avsynctestvideosrc.h"
#include "avsynctestaudiosrc.h"

static gboolean
plugin_init (GstPlugin * plugin)
{
	gst_element_register (plugin, "avsynctestvideosrc", GST_RANK_NONE,
		GST_TYPE_AV_SYNC_TEST_VIDEO_SRC);
	gst_element_register (plugin, "avsynctestaudiosrc", GST_RANK_NONE,
		GST_TYPE_AV_SYNC_TEST_AUDIO_SRC);

	return TRUE;
}

GST_PLUGIN_DEFINE (
	GST_VERSION_MAJOR,
	GST_VERSION_MINOR,
	avsynctestsrc,
	"Generates an Audio and a Video-Signal that have distinctive features happening at the exact same point in time, useful to detect Problems in the A/V Sync of long running Systems.",
	plugin_init, VERSION, "LGPL", PACKAGE_NAME, "https://github.com/MaZderMind/gst-avsynctestsrc")
