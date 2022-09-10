/* layoutinline.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "layoutinline.h"

typedef struct
{
    int padding;
} TextLayoutInlinePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (TextLayoutInline, text_layout_inline, TEXT_TYPE_LAYOUT_BOX)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_layout_inline_new:
 *
 * Create a new #TextLayoutInline.
 *
 * Returns: (transfer full): a newly created #TextLayoutInline
 */
TextLayoutInline *
text_layout_inline_new (void)
{
    return g_object_new (TEXT_TYPE_LAYOUT_INLINE, NULL);
}

static void
text_layout_inline_finalize (GObject *object)
{
    TextLayoutInline *self = (TextLayoutInline *)object;
    TextLayoutInlinePrivate *priv = text_layout_inline_get_instance_private (self);

    // TODO: Dispose of children

    G_OBJECT_CLASS (text_layout_inline_parent_class)->finalize (object);
}

static void
text_layout_inline_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    TextLayoutInline *self = TEXT_LAYOUT_INLINE (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_layout_inline_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    TextLayoutInline *self = TEXT_LAYOUT_INLINE (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

void
text_layout_inline_layout (TextLayoutBox *self,
                           PangoContext  *context,
                           int            width,
                           int            offset_x,
                           int            offset_y)
{
    TextLayoutInlinePrivate *priv;
    TextDimensions *bbox;

    g_return_if_fail (TEXT_IS_LAYOUT_INLINE (self));

    priv = text_layout_inline_get_instance_private (TEXT_LAYOUT_INLINE (self));
    bbox = text_layout_box_get_mutable_bbox (self);

    bbox->x = offset_x;
    bbox->y = offset_y;
    bbox->width = 100;
    bbox->height = 100;
}

static void
text_layout_inline_class_init (TextLayoutInlineClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_layout_inline_finalize;
    object_class->get_property = text_layout_inline_get_property;
    object_class->set_property = text_layout_inline_set_property;

    TextLayoutBoxClass *layout_box_class = TEXT_LAYOUT_BOX_CLASS (klass);

    layout_box_class->layout = text_layout_inline_layout;
}

static void
text_layout_inline_init (TextLayoutInline *self)
{
    TextLayoutInlinePrivate *priv = text_layout_inline_get_instance_private (self);
}
