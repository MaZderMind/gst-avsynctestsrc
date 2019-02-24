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
#ifndef _GST_AV_SYNC_TEST_AUDIO_SRC_H_
#define _GST_AV_SYNC_TEST_AUDIO_SRC_H_

#include <gst/base/gstpushsrc.h>
  #include <gst/audio/audio.h>

G_BEGIN_DECLS
#define GST_TYPE_AV_SYNC_TEST_AUDIO_SRC           (gst_avsynctestaudiosrc_get_type())
#define GST_AV_SYNC_TEST_AUDIO_SRC(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_AV_SYNC_TEST_AUDIO_SRC, GstAvSyncTestAudioSrc))
#define GST_AV_SYNC_TEST_AUDIO_SRC_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),  GST_TYPE_AV_SYNC_TEST_AUDIO_SRC, GstAvSyncTestAudioSrcClass))
#define GST_IS_AV_SYNC_TEST_AUDIO_SRC(obj)        (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_AV_SYNC_TEST_AUDIO_SRC))
#define GST_IS_AV_SYNC_TEST_AUDIO_SRC_CLASS(obj)  (G_TYPE_CHECK_CLASS_TYPE((klass),  GST_TYPE_AV_SYNC_TEST_AUDIO_SRC))
typedef struct _GstAvSyncTestAudioSrc GstAvSyncTestAudioSrc;
typedef struct _GstAvSyncTestAudioSrcClass GstAvSyncTestAudioSrcClass;


struct _GstAvSyncTestAudioSrc
{
  GstPushSrc base_avsynctestaudiosrc;
  GstAudioInfo audio_info;
  gint8 counter;

  gdouble freq;
};

struct _GstAvSyncTestAudioSrcClass
{
  GstPushSrcClass base_avsynctestaudiosrc_class;

  void (*sync_point) (GstElement * element);
};

GType gst_avsynctestaudiosrc_get_type (void);

G_END_DECLS
#endif // _GST_AV_SYNC_TEST_AUDIO_SRC_H_
