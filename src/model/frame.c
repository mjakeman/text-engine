/* frame.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "frame.h"

typedef struct
{
    int _padding;
} TextFramePrivate;

G_DEFINE_FINAL_TYPE_WITH_PRIVATE (TextFrame, text_frame, TEXT_TYPE_BLOCK)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_frame_new:
 *
 * Create a new #TextFrame.
 *
 * Returns: (transfer full): a newly created #TextFrame
 */
TextFrame *
text_frame_new (void)
{
    return g_object_new (TEXT_TYPE_FRAME, NULL);
}

static void
text_frame_finalize (GObject *object)
{
    TextFrame *self = (TextFrame *)object;
    TextFramePrivate *priv = text_frame_get_instance_private (self);

    G_OBJECT_CLASS (text_frame_parent_class)->finalize (object);
}

static void
text_frame_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    TextFrame *self = TEXT_FRAME (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
text_frame_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    TextFrame *self = TEXT_FRAME (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

void
text_frame_append_block (TextFrame *self,
                         TextBlock *block)
{
    g_return_if_fail (TEXT_IS_FRAME (self));
    g_return_if_fail (TEXT_IS_BLOCK (block));

    text_node_append_child (TEXT_NODE (self), TEXT_NODE (block));
}

void
text_frame_prepend_block (TextFrame *self,
                          TextBlock *block)
{
    g_return_if_fail (TEXT_IS_FRAME (self));
    g_return_if_fail (TEXT_IS_BLOCK (block));

    text_node_prepend_child (TEXT_NODE (self), TEXT_NODE (block));
}

static void
text_frame_class_init (TextFrameClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_frame_finalize;
    object_class->get_property = text_frame_get_property;
    object_class->set_property = text_frame_set_property;
}

static void
text_frame_init (TextFrame *self)
{
}
