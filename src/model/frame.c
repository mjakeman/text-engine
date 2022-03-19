/* frame.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
