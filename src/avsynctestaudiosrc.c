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

#include "avsynctestaudiosrc.h"

/* pad templates */
static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("audio/x-raw,format=S16LE")
);

GST_DEBUG_CATEGORY_STATIC (gst_avsynctestaudiosrc_debug);
#define GST_CAT_DEFAULT gst_avsynctestaudiosrc_debug

/* signals */
enum
{
  SIGNAL_SYNC_POINT,
  LAST_SIGNAL
};

static guint gst_av_sync_test_audio_src_signals[LAST_SIGNAL] = { 0, };


/* properties */
enum
{
  PROP_0,
  PROP_FREQ,
};

/* property defaults */
#define PROP_FREQ_DEFAULT (0.0)


/* parent class */
#define gst_avsynctestaudiosrc_parent_class parent_class
G_DEFINE_TYPE (GstAvSyncTestAudioSrc, gst_avsynctestaudiosrc, GST_TYPE_PUSH_SRC);

/* GObject member methods */
static void gst_avsynctestaudiosrc_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_avsynctestaudiosrc_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_avsynctestaudiosrc_finalize (GObject * obj);

/* GstPushSrc member methods */
static gboolean gst_avsynctestaudiosrc_set_caps (GstBaseSrc * base, GstCaps * caps);
static GstFlowReturn gst_avsynctestaudiosrc_fill (GstPushSrc * base, GstBuffer *buffer);

static void
gst_avsynctestaudiosrc_class_init (GstAvSyncTestAudioSrcClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = gst_avsynctestaudiosrc_set_property;
  gobject_class->get_property = gst_avsynctestaudiosrc_get_property;
  gobject_class->finalize = gst_avsynctestaudiosrc_finalize;

  g_object_class_install_property (gobject_class, PROP_FREQ,
      g_param_spec_double ("freq", "Freq",
          "Frequency of test signal.",
          0.0, 1.0,
          PROP_FREQ_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | GST_PARAM_CONTROLLABLE));


  gst_av_sync_test_audio_src_signals[SIGNAL_SYNC_POINT] = g_signal_new (
    /* signal_name */ "sync-point",
    /* itype */ G_TYPE_FROM_CLASS (klass),
    /* signal_flags */ G_SIGNAL_RUN_LAST,
    /* class_offset */ G_STRUCT_OFFSET (GstAvSyncTestAudioSrcClass, sync_point),
    /* accumulator */ NULL,
    /* accu_data */ NULL,
    /* c_marshaller */ NULL,
    /* return_type */   G_TYPE_NONE,
    /* n_params */ 0
    /* varargs: param types */);


  GstBaseSrcClass *base_src_class = GST_BASE_SRC_CLASS (klass);
  base_src_class->set_caps = GST_DEBUG_FUNCPTR (gst_avsynctestaudiosrc_set_caps);

  GstPushSrcClass *src_class = GST_PUSH_SRC_CLASS (klass);
  src_class->fill = GST_DEBUG_FUNCPTR (gst_avsynctestaudiosrc_fill);

  GST_DEBUG_CATEGORY_INIT (gst_avsynctestaudiosrc_debug, "avsynctestaudiosrc", 0, "AV Sync-Test Audio Src");

  gst_element_class_add_static_pad_template (element_class, &srctemplate);

  gst_element_class_set_static_metadata (element_class, "AV Sync-Test Audio Src",
      "Source/Audio/Debug",
      "Generates the Audio-Portion of the AV Sync-Test Signal.",
      "Peter Körner <peter@mazdermind.de>");
}

static void
gst_avsynctestaudiosrc_init (GstAvSyncTestAudioSrc * avsynctestaudiosrc)
{
  GST_DEBUG_OBJECT (avsynctestaudiosrc, "init");

    avsynctestaudiosrc->freq = PROP_FREQ_DEFAULT;
}

void
gst_avsynctestaudiosrc_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec)
{
  GstAvSyncTestAudioSrc *avsynctestaudiosrc = GST_AV_SYNC_TEST_AUDIO_SRC (object);

  switch (property_id) {
    case PROP_FREQ:
      avsynctestaudiosrc->freq = g_value_get_double(value);
      break;


    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (avsynctestaudiosrc, property_id, pspec);
      break;
  }
}

void
gst_avsynctestaudiosrc_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec)
{
  GstAvSyncTestAudioSrc *avsynctestaudiosrc = GST_AV_SYNC_TEST_AUDIO_SRC (object);

  switch (property_id) {
    case PROP_FREQ:
      g_value_set_double (value, avsynctestaudiosrc->freq);
      break;


    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (avsynctestaudiosrc, property_id, pspec);
      break;
  }
}

void
gst_avsynctestaudiosrc_finalize (GObject * object)
{
  GstAvSyncTestAudioSrc *avsynctestaudiosrc = GST_AV_SYNC_TEST_AUDIO_SRC (object);
  GST_DEBUG_OBJECT (avsynctestaudiosrc, "finalize");


  G_OBJECT_CLASS (gst_avsynctestaudiosrc_parent_class)->finalize (object);
}

static gboolean
gst_avsynctestaudiosrc_set_caps (GstBaseSrc * base, GstCaps * caps)
{
  GstAvSyncTestAudioSrc *avsynctestaudiosrc = GST_AV_SYNC_TEST_AUDIO_SRC (base);
  GST_DEBUG_OBJECT (avsynctestaudiosrc, "set_caps caps=%" GST_PTR_FORMAT, caps);

  gst_audio_info_from_caps (&avsynctestaudiosrc->audio_info, caps);

  return TRUE;
}

static GstFlowReturn
gst_avsynctestaudiosrc_fill (GstPushSrc * base, GstBuffer *buffer)
{
  GstAvSyncTestAudioSrc *avsynctestaudiosrc = GST_AV_SYNC_TEST_AUDIO_SRC (base);
  GST_DEBUG_OBJECT (avsynctestaudiosrc, "fill");

  GstMapInfo map;
  gst_buffer_map (buffer, &map, GST_MAP_WRITE);

  // assuming S16LE / 1 Channel
  guint num_samples = map.size / sizeof(gint16);
  for(guint sample_idx = 0; sample_idx < num_samples; sample_idx++) {
    // pointer to sample
    gint16 *sample_ptr = ((gint16*) map.data) + sample_idx;

    // fill with sawtooth ramp (multiply gint8 to gint16 by bit-shifting)
    *sample_ptr = (avsynctestaudiosrc->counter << 8);
    avsynctestaudiosrc->counter++;
  }

  gst_buffer_unmap (buffer, &map);

  return GST_FLOW_OK;
}
