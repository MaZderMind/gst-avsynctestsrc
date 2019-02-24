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

/* pad templates */
static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("video/x-raw,format=RGBx")
);

GST_DEBUG_CATEGORY_STATIC (gst_avsynctestvideosrc_debug);
#define GST_CAT_DEFAULT gst_avsynctestvideosrc_debug

/* signals */
enum
{
  SIGNAL_SYNC_POINT,
  LAST_SIGNAL
};

static guint gst_av_sync_test_video_src_signals[LAST_SIGNAL] = { 0, };


/* properties */
enum
{
  PROP_0,
  PROP_FOREGROUND_COLOR,
  PROP_BACKGROUND_COLOR,
};

/* property defaults */
#define PROP_FOREGROUND_COLOR_DEFAULT (0xFFFFFFFF)
#define PROP_BACKGROUND_COLOR_DEFAULT (0xFF000000)


/* parent class */
#define gst_avsynctestvideosrc_parent_class parent_class
G_DEFINE_TYPE (GstAvSyncTestVideoSrc, gst_avsynctestvideosrc, GST_TYPE_PUSH_SRC);

/* GObject member methods */
static void gst_avsynctestvideosrc_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_avsynctestvideosrc_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);
static void gst_avsynctestvideosrc_finalize (GObject * obj);

/* GstPushSrc member methods */
static gboolean gst_avsynctestvideosrc_set_caps (GstBaseSrc * base, GstCaps * caps);
static GstFlowReturn gst_avsynctestvideosrc_fill (GstPushSrc * base, GstBuffer *buffer);

static void
gst_avsynctestvideosrc_class_init (GstAvSyncTestVideoSrcClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = gst_avsynctestvideosrc_set_property;
  gobject_class->get_property = gst_avsynctestvideosrc_get_property;
  gobject_class->finalize = gst_avsynctestvideosrc_finalize;

  g_object_class_install_property (gobject_class, PROP_FOREGROUND_COLOR,
      g_param_spec_uint ("foreground-color", "Foreground Color",
          "Foreground Color of the generated Test-Image.",
          0, G_MAXUINT,
          PROP_FOREGROUND_COLOR_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | GST_PARAM_CONTROLLABLE));

  g_object_class_install_property (gobject_class, PROP_BACKGROUND_COLOR,
      g_param_spec_uint ("background-color", "Background-Color",
          "Background Color of the generated Test-Image.",
          0, G_MAXUINT,
          PROP_BACKGROUND_COLOR_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | GST_PARAM_CONTROLLABLE));


  gst_av_sync_test_video_src_signals[SIGNAL_SYNC_POINT] = g_signal_new (
    /* signal_name */ "sync-point",
    /* itype */ G_TYPE_FROM_CLASS (klass),
    /* signal_flags */ G_SIGNAL_RUN_LAST,
    /* class_offset */ G_STRUCT_OFFSET (GstAvSyncTestVideoSrcClass, sync_point),
    /* accumulator */ NULL,
    /* accu_data */ NULL,
    /* c_marshaller */ NULL,
    /* return_type */   G_TYPE_NONE,
    /* n_params */ 0
    /* varargs: param types */);


  GstBaseSrcClass *base_src_class = GST_BASE_SRC_CLASS (klass);
  base_src_class->set_caps = GST_DEBUG_FUNCPTR (gst_avsynctestvideosrc_set_caps);

  GstPushSrcClass *src_class = GST_PUSH_SRC_CLASS (klass);
  src_class->fill = GST_DEBUG_FUNCPTR (gst_avsynctestvideosrc_fill);

  GST_DEBUG_CATEGORY_INIT (gst_avsynctestvideosrc_debug, "avsynctestvideosrc", 0, "AV Sync-Test Video Src");

  gst_element_class_add_static_pad_template (element_class, &srctemplate);

  gst_element_class_set_static_metadata (element_class, "AV Sync-Test Video Src",
      "Source/Video/Debug",
      "Generates the Video-Portion of the AV Sync-Test Signal.",
      "Peter Körner <peter@mazdermind.de>");
}

static void
gst_avsynctestvideosrc_init (GstAvSyncTestVideoSrc * avsynctestvideosrc)
{
  GST_DEBUG_OBJECT (avsynctestvideosrc, "init");

    avsynctestvideosrc->foreground_color = PROP_FOREGROUND_COLOR_DEFAULT;
    avsynctestvideosrc->background_color = PROP_BACKGROUND_COLOR_DEFAULT;
}

void
gst_avsynctestvideosrc_set_property (GObject * object, guint property_id, const GValue * value, GParamSpec * pspec)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (object);

  switch (property_id) {
    case PROP_FOREGROUND_COLOR:
      avsynctestvideosrc->foreground_color = g_value_get_uint(value);
      break;

    case PROP_BACKGROUND_COLOR:
      avsynctestvideosrc->background_color = g_value_get_uint(value);
      break;


    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (avsynctestvideosrc, property_id, pspec);
      break;
  }
}

void
gst_avsynctestvideosrc_get_property (GObject * object, guint property_id, GValue * value, GParamSpec * pspec)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (object);

  switch (property_id) {
    case PROP_FOREGROUND_COLOR:
      g_value_set_uint (value, avsynctestvideosrc->foreground_color);
      break;

    case PROP_BACKGROUND_COLOR:
      g_value_set_uint (value, avsynctestvideosrc->background_color);
      break;


    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (avsynctestvideosrc, property_id, pspec);
      break;
  }
}

void
gst_avsynctestvideosrc_finalize (GObject * object)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (object);
  GST_DEBUG_OBJECT (avsynctestvideosrc, "finalize");


  G_OBJECT_CLASS (gst_avsynctestvideosrc_parent_class)->finalize (object);
}

static gboolean
gst_avsynctestvideosrc_set_caps (GstBaseSrc * base, GstCaps * caps)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (base);
  GST_DEBUG_OBJECT (avsynctestvideosrc, "set_caps caps=%" GST_PTR_FORMAT, caps);

  gst_video_info_from_caps (&avsynctestvideosrc->video_info, caps);

  return TRUE;
}

static GstFlowReturn
gst_avsynctestvideosrc_fill (GstPushSrc * base, GstBuffer *buffer)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (base);
  GST_DEBUG_OBJECT (avsynctestvideosrc, "fill");

  GstVideoFrame frame;
  gst_video_frame_map (&frame, &avsynctestvideosrc->video_info, buffer, GST_MAP_WRITE);

  // fill &frame with video-data here (example given for RGBx, you probably need to check the caps before)
  guint8 *pixels = GST_VIDEO_FRAME_PLANE_DATA (&frame, 0);
  guint stride = GST_VIDEO_FRAME_PLANE_STRIDE (&frame, 0);
  guint pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (&frame, 0);

  for (guint h = 0; h < avsynctestvideosrc->video_info.height; ++h) {
    for (guint w = 0; w < avsynctestvideosrc->video_info.width; ++w) {
      guint8 *pixel = pixels + h * stride + w * pixel_stride;

      *(pixel+0) = 255; // R
      *(pixel+1) = 0;   // G
      *(pixel+2) = 0;   // B
    }
  }

  gst_video_frame_unmap (&frame);

  return GST_FLOW_OK;
}
