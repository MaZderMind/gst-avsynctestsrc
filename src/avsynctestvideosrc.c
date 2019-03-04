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
    GST_STATIC_CAPS("video/x-raw,format=BGRx,interlace-mode=progressive,multiview-mide=mono,pixel-aspect-ratio=1/1")
);

GST_DEBUG_CATEGORY_STATIC (gst_avsynctestvideosrc_debug);
#define GST_CAT_DEFAULT gst_avsynctestvideosrc_debug

#define COLOR_R(x) ((double)((x & 0x00FF0000) >> 16) / 0xFF)
#define COLOR_G(x) ((double)((x & 0x0000FF00) >>  8) / 0xFF)
#define COLOR_B(x) ((double)((x & 0x000000FF) >>  0) / 0xFF)

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

/* basic geom types */
typedef struct rectangle
{
  double top;
  double left;
  double width;
  double height;
} double_rectangle_t;

/* geometry of test-card */
static const double_rectangle_t flash_rectangle = {0.166, 0.070, 0.437, 0.283};

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

/* GstBaseSrc member methods */
static gboolean gst_avsynctestvideosrc_set_caps (GstBaseSrc * base, GstCaps * caps);
static GstCaps *gst_avsynctestvideosrc_fixate (GstBaseSrc * base, GstCaps * caps);
static void gst_avsynctestvideosrc_get_times (GstBaseSrc * base, GstBuffer * buffer, GstClockTime * start, GstClockTime * end);

/* GstPushSrc member methods */
static GstFlowReturn gst_avsynctestvideosrc_fill (GstPushSrc * base, GstBuffer *buffer);

/* GstAvSyncTestVideoSrc member methods */
static void gst_avsynctestvideosrc_destory_cairo (GstAvSyncTestVideoSrc * avsynctestvideosrc);
static void gst_avsynctestvideosrc_paint_background (GstAvSyncTestVideoSrc * avsynctestvideosrc);

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
          "Foreground Color of the generated Test-Image. (big-endian ARGB)",
          0, G_MAXUINT,
          PROP_FOREGROUND_COLOR_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | GST_PARAM_CONTROLLABLE));

  g_object_class_install_property (gobject_class, PROP_BACKGROUND_COLOR,
      g_param_spec_uint ("background-color", "Background-Color",
          "Background Color of the generated Test-Image. (big-endian ARGB)",
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
  base_src_class->fixate = GST_DEBUG_FUNCPTR (gst_avsynctestvideosrc_fixate);
  base_src_class->get_times = GST_DEBUG_FUNCPTR (gst_avsynctestvideosrc_get_times);

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

  gst_base_src_set_live(GST_BASE_SRC(avsynctestvideosrc), TRUE);
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

  //gst_avsynctestvideosrc_destory_cairo(avsynctestvideosrc);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

void
gst_avsynctestvideosrc_destory_cairo (GstAvSyncTestVideoSrc * avsynctestvideosrc)
{
  if (avsynctestvideosrc->cairo != NULL) {
    GST_DEBUG_OBJECT (avsynctestvideosrc, "destroying cairo context");
    cairo_destroy(avsynctestvideosrc->cairo);
  }

  if (avsynctestvideosrc->surface != NULL) {
    GST_DEBUG_OBJECT (avsynctestvideosrc, "destroying cairo surface");
    cairo_surface_destroy(avsynctestvideosrc->surface);
  }
}

static gboolean
gst_avsynctestvideosrc_set_caps (GstBaseSrc * base, GstCaps * caps)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (base);
  GST_DEBUG_OBJECT (avsynctestvideosrc, "set_caps caps=%" GST_PTR_FORMAT, caps);

  gst_video_info_from_caps (&avsynctestvideosrc->video_info, caps);
  gst_avsynctestvideosrc_destory_cairo(avsynctestvideosrc);

  GST_DEBUG_OBJECT (avsynctestvideosrc, "creating cairo surface xRGB");
  avsynctestvideosrc->surface = cairo_image_surface_create (
    CAIRO_FORMAT_RGB24,
    avsynctestvideosrc->video_info.width,
    avsynctestvideosrc->video_info.height);

  GST_DEBUG_OBJECT (avsynctestvideosrc, "creating cairo surface");
  avsynctestvideosrc->cairo = cairo_create (avsynctestvideosrc->surface);

  gst_avsynctestvideosrc_paint_background(avsynctestvideosrc);

  return TRUE;
}

static GstCaps *gst_avsynctestvideosrc_fixate (GstBaseSrc * base, GstCaps * caps)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (base);
  GST_DEBUG_OBJECT (avsynctestvideosrc, "fixate in=%" GST_PTR_FORMAT, caps);

  caps = gst_caps_make_writable (caps);
  GstStructure *structure = gst_caps_get_structure (caps, 0);

  gst_structure_fixate_field_nearest_int (structure, "width", 320);
  gst_structure_fixate_field_nearest_int (structure, "height", 240);
  gst_structure_fixate_field_nearest_fraction (structure, "framerate", 30, 1);

  caps = GST_BASE_SRC_CLASS (parent_class)->fixate (base, caps);

  GST_DEBUG_OBJECT (avsynctestvideosrc, "fixate out=%" GST_PTR_FORMAT, caps);
  return caps;
}

void
gst_avsynctestvideosrc_get_times (GstBaseSrc * base, GstBuffer * buffer, GstClockTime * start, GstClockTime * end)
{
  GstAvSyncTestVideoSrc *avsynctestvideosrc = GST_AV_SYNC_TEST_VIDEO_SRC (base);
  GST_DEBUG_OBJECT (avsynctestvideosrc, "get_times pts=%" GST_TIME_FORMAT " duration=%" GST_TIME_FORMAT,
    GST_TIME_ARGS(GST_BUFFER_PTS (buffer)),
    GST_TIME_ARGS(GST_BUFFER_DURATION (buffer)));

  GstClockTime timestamp = GST_BUFFER_PTS (buffer);

  if (GST_CLOCK_TIME_IS_VALID (timestamp)) {
    /* get duration to calculate end time */
    GstClockTime duration = GST_BUFFER_DURATION (buffer);

    if (GST_CLOCK_TIME_IS_VALID (duration)) {
      *end = timestamp + duration;
    }
    *start = timestamp;
  }
}

void
gst_avsynctestvideosrc_paint_background (GstAvSyncTestVideoSrc * avsynctestvideosrc)
{
  cairo_t *cr = avsynctestvideosrc->cairo;
  double width = cairo_image_surface_get_width (avsynctestvideosrc->surface);
  double height = cairo_image_surface_get_height (avsynctestvideosrc->surface);

  // fill background with background_color
  cairo_rectangle (cr, 0, 0, width, height);
  cairo_set_source_rgb (cr,
    COLOR_R(avsynctestvideosrc->background_color),
    COLOR_G(avsynctestvideosrc->background_color),
    COLOR_B(avsynctestvideosrc->background_color));
  cairo_fill (cr);

  // continue painting in foreground_color
  cairo_set_source_rgb (cr,
    COLOR_R(avsynctestvideosrc->foreground_color),
    COLOR_G(avsynctestvideosrc->foreground_color),
    COLOR_B(avsynctestvideosrc->foreground_color));

  // draw flash-rectangle outline
  {
    double_rectangle_t r = {
      flash_rectangle.left   * width,
      flash_rectangle.top    * height,
      flash_rectangle.width  * width,
      flash_rectangle.height * height
    };

    cairo_move_to (cr, r.left, r.top);
    cairo_line_to (cr, r.left + r.width, r.top);
    cairo_line_to (cr, r.left + r.width, r.top + r.height);
    cairo_line_to (cr, r.left, r.top + r.height);
    cairo_line_to (cr, r.left, r.top);
    cairo_stroke (cr);
  }
}

static GstFlowReturn
gst_avsynctestvideosrc_fill (GstPushSrc * base, GstBuffer *buffer)
{
  GstAvSyncTestVideoSrc *src = GST_AV_SYNC_TEST_VIDEO_SRC (base);

  /* 0 framerate and we are at the second frame, eos */
  if (G_UNLIKELY (src->video_info.fps_n == 0 && src->n_frames == 1)) {
    goto eos;
  }

  GST_BUFFER_PTS (buffer) = gst_util_uint64_scale (
    src->n_frames,
    src->video_info.fps_d * GST_SECOND,
    src->video_info.fps_n);

  GST_BUFFER_DTS (buffer) = GST_CLOCK_TIME_NONE;
  GST_BUFFER_DURATION (buffer) = gst_util_uint64_scale (
    GST_SECOND,
    src->video_info.fps_d,
    src->video_info.fps_n);

  src->n_frames++;

  GstVideoFrame frame;
  gst_video_frame_map (&frame, &src->video_info, buffer, GST_MAP_WRITE);

  cairo_surface_t * surface = src->surface;
  cairo_surface_flush(surface);
  unsigned char * cairo_pixels = cairo_image_surface_get_data(surface);
  int cairo_width = cairo_image_surface_get_width (surface);
  int cairo_height = cairo_image_surface_get_height (surface);
  int cairo_stride = cairo_image_surface_get_stride (surface);

  // fill &frame with video-data here (example given for RGBx, you probably need to check the caps before)
  unsigned char * gst_pixels = GST_VIDEO_FRAME_PLANE_DATA (&frame, 0);
  gint gst_width = GST_VIDEO_FRAME_WIDTH (&frame);
  gint gst_height = GST_VIDEO_FRAME_HEIGHT (&frame);
  gint gst_stride = GST_VIDEO_FRAME_COMP_STRIDE (&frame, 0);

  if (G_UNLIKELY (cairo_width != gst_width)) {
    GST_ERROR_OBJECT(src, "cairo width %d != gst width %d", cairo_width, gst_width);
    goto incompatible_formats;
  }

  if (G_UNLIKELY (cairo_height != gst_height)) {
    GST_ERROR_OBJECT(src, "cairo height %d != gst height %d", cairo_height, gst_height);
    goto incompatible_formats;
  }

  if (G_UNLIKELY (cairo_stride != gst_stride)) {
    GST_ERROR_OBJECT(src, "cairo stride %d != gst stride %d", cairo_stride, gst_stride);
    goto incompatible_formats;
  }

  gint64 num_bytes = gst_height * gst_stride;
  //GST_DEBUG_OBJECT (src, "memcpy %" G_GINT64_FORMAT " bytes from cairo to gst-buffer", num_bytes);
  memcpy(gst_pixels, cairo_pixels, num_bytes);

  gst_video_frame_unmap (&frame);

  return GST_FLOW_OK;

incompatible_formats:
  gst_video_frame_unmap (&frame);
  return GST_FLOW_ERROR;

eos:
  {
    GST_DEBUG_OBJECT (src, "eos: 0 framerate, frame %d", (gint) src->n_frames);
    return GST_FLOW_EOS;
  }
}
