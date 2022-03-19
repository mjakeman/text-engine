/* layout-box.c
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

#include "layout-box.h"

#include "../model/paragraph.h"
#include "../tree/node.h"

typedef struct
{
    TextItem *item;
    PangoLayout *layout;

    TextDimensions bbox;
} TextLayoutBoxPrivate;

G_DEFINE_FINAL_TYPE_WITH_PRIVATE (TextLayoutBox, text_layout_box, TEXT_TYPE_NODE)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

/**
 * text_layout_box_new:
 *
 * Create a new #TextLayoutBox.
 *
 * Returns: (transfer full): a newly created #TextLayoutBox
 */
TextLayoutBox *
text_layout_box_new (void)
{
    return g_object_new (TEXT_TYPE_LAYOUT_BOX, NULL);
}

static void
text_layout_box_finalize (GObject *object)
{
    TextLayoutBox *self = (TextLayoutBox *)object;
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);

    // Dispose of children
    for (TextNode *child = text_node_get_first_child (TEXT_NODE (self));
         child != NULL;
         child = text_node_get_next (TEXT_NODE (self)))
    {
        g_clear_object (&child);
    }

    G_OBJECT_CLASS (text_layout_box_parent_class)->finalize (object);
}

static void
text_layout_box_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    TextLayoutBox *self = TEXT_LAYOUT_BOX (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_layout_box_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    TextLayoutBox *self = TEXT_LAYOUT_BOX (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

void
text_layout_box_layout (TextLayoutBox *self,
                        PangoContext  *context,
                        int            width)
{
    g_return_if_fail (TEXT_IS_LAYOUT_BOX (self));

    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);

    int height = 0;

    g_debug ("Starting for %s\n", g_type_name_from_instance (self));

    g_debug ("Has item: %d\n", priv->item != NULL);
    g_debug ("Has paragraph: %d\n", TEXT_IS_PARAGRAPH (priv->item));

    if (priv->item && TEXT_IS_PARAGRAPH (priv->item))
    {
        GString *str = g_string_new ("");
        for (TextNode *run = text_node_get_first_child (TEXT_NODE (priv->item));
             run != NULL;
             run = text_node_get_next (run))
        {
            const gchar *run_text;
            g_object_get (run, "text", &run_text, NULL);
            g_string_append (str, run_text);
            g_debug (" - Counting run\n");
        }

        gchar *text = g_string_free (str, FALSE);
        g_debug (" - String %s\n", text);

        if (!priv->layout)
            priv->layout = pango_layout_new (context);

        pango_layout_set_text (priv->layout, text, -1);
        pango_layout_set_wrap (priv->layout, PANGO_WRAP_WORD_CHAR);
        pango_layout_set_width (priv->layout, PANGO_SCALE * width);
        pango_layout_get_pixel_size (priv->layout, NULL, &height);
        g_debug (" - Height %d\n", height);

        g_free (text);
    }

    // Account for children (should we force elements to choose between
    // children and text? seems like a sensible simplification)

    for (TextNode *node = text_node_get_first_child (TEXT_NODE (self));
         node != NULL;
         node = text_node_get_next (node))
    {
        TextLayoutBox *child_box = TEXT_LAYOUT_BOX (node);

        g_debug (" - Found child\n");

        // We can assume bbox already exists by now, as the layout() method
        // has been called already in the layout manager.

        TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (child_box);
        height += priv->bbox.height;
        g_debug (" - Child height %d\n", height);
    }

    priv->bbox.x = 0;
    priv->bbox.y = 0;
    priv->bbox.width = width;
    priv->bbox.height = height;
}

void
text_layout_box_set_item (TextLayoutBox *self,
                          TextItem      *item)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
    priv->item = item;
    g_debug ("Set item to non null: %d\n", priv->item != NULL);
}

TextItem *
text_layout_box_get_item (TextLayoutBox *self,
                          TextItem      *item)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
    return priv->item;
}

const TextDimensions *
text_layout_box_get_bbox (TextLayoutBox *self)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
    return &priv->bbox;
}

PangoLayout *
text_layout_box_get_pango_layout (TextLayoutBox *self)
{
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
    return priv->layout;
}

static void
text_layout_box_class_init (TextLayoutBoxClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_layout_box_finalize;
    object_class->get_property = text_layout_box_get_property;
    object_class->set_property = text_layout_box_set_property;
}

static void
text_layout_box_init (TextLayoutBox *self)
{
}
