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
#ifndef _GST_AV_SYNC_TEST_VIDEO_SRC_H_
#define _GST_AV_SYNC_TEST_VIDEO_SRC_H_

#include <gst/base/gstpushsrc.h>
#include <gst/video/video.h>

#include <cairo.h>

G_BEGIN_DECLS
#define GST_TYPE_AV_SYNC_TEST_VIDEO_SRC           (gst_avsynctestvideosrc_get_type())
#define GST_AV_SYNC_TEST_VIDEO_SRC(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_AV_SYNC_TEST_VIDEO_SRC, GstAvSyncTestVideoSrc))
#define GST_AV_SYNC_TEST_VIDEO_SRC_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),  GST_TYPE_AV_SYNC_TEST_VIDEO_SRC, GstAvSyncTestVideoSrcClass))
#define GST_IS_AV_SYNC_TEST_VIDEO_SRC(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_AV_SYNC_TEST_VIDEO_SRC))
#define GST_IS_AV_SYNC_TEST_VIDEO_SRC_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE((klass),  GST_TYPE_AV_SYNC_TEST_VIDEO_SRC))
typedef struct _GstAvSyncTestVideoSrc GstAvSyncTestVideoSrc;
typedef struct _GstAvSyncTestVideoSrcClass GstAvSyncTestVideoSrcClass;


struct _GstAvSyncTestVideoSrc
{
  GstPushSrc base_avsynctestvideosrc;
  GstVideoInfo video_info;

  guint foreground_color;
  guint background_color;

  cairo_surface_t *surface;
  cairo_t *cairo;
};

struct _GstAvSyncTestVideoSrcClass
{
  GstPushSrcClass base_avsynctestvideosrc_class;

  void (*sync_point) (GstElement * element);
};

GType gst_avsynctestvideosrc_get_type (void);

G_END_DECLS
#endif // _GST_AV_SYNC_TEST_VIDEO_SRC_H_
