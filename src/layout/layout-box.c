/* layout-box.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "layout-box.h"

#include "../model/paragraph.h"
#include "../model/image.h"

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
_set_attributes (TextParagraph *paragraph,
                 PangoLayout   *pango_layout)
{
    TextNode *run;
    PangoAttrList *list;

    int start_index;

    list = pango_attr_list_new();

    g_return_if_fail (TEXT_IS_PARAGRAPH (paragraph));

    start_index = 0;

    for (run = text_node_get_first_child (TEXT_NODE (paragraph));
         run != NULL;
         run = text_node_get_next (run))
    {
        gboolean is_bold, is_italic, is_underline;
        PangoAttribute *attr;
        int run_length;

        if (!TEXT_IS_RUN (run))
            continue;

        run_length = text_fragment_get_size_bytes (TEXT_FRAGMENT (run));

        // Get Style Properties
        is_bold = text_run_get_style_bold (TEXT_RUN (run));
        is_italic = text_run_get_style_italic (TEXT_RUN (run));
        is_underline = text_run_get_style_underline (TEXT_RUN (run));

        // Attribute: Bold
        if (is_bold)
        {
            attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
            attr->start_index = start_index;
            attr->end_index = start_index + run_length;
            pango_attr_list_insert (list, attr);
        }

        // Attribute: Italic
        if (is_italic)
        {
            attr = pango_attr_style_new (PANGO_STYLE_ITALIC);
            attr->start_index = start_index;
            attr->end_index = start_index + run_length;
            pango_attr_list_insert (list, attr);
        }

        // Attribute: Underline
        if (is_underline)
        {
            attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
            attr->start_index = start_index;
            attr->end_index = start_index + run_length;
            pango_attr_list_insert (list, attr);
        }

        start_index += run_length;
    }

    pango_layout_set_attributes (pango_layout, list);
}

void
text_layout_box_layout (TextLayoutBox *self,
                        PangoContext  *context,
                        int            width,
                        int            offset_x,
                        int            offset_y)
{
    g_return_if_fail (TEXT_IS_LAYOUT_BOX (self));

    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);

    int height = 0;

    g_debug ("Starting for %s\n", g_type_name_from_instance (self));

    g_debug ("Has item: %d\n", priv->item != NULL);
    g_debug ("Has paragraph: %d\n", TEXT_IS_PARAGRAPH (priv->item));

    if (priv->item && TEXT_IS_PARAGRAPH (priv->item))
    {
        gchar *text;
        text = text_paragraph_get_text (TEXT_PARAGRAPH (priv->item));
        g_debug (" - String %s\n", text);

        if (!priv->layout)
            priv->layout = pango_layout_new (context);

        pango_layout_set_text (priv->layout, text, -1);
        pango_layout_set_wrap (priv->layout, PANGO_WRAP_WORD_CHAR);
        pango_layout_set_width (priv->layout, PANGO_SCALE * width);
        pango_layout_get_pixel_size (priv->layout, NULL, &height);
        g_debug (" - Height %d\n", height);

        // Set style information
        // TODO: Matching from ruleset
        _set_attributes (TEXT_PARAGRAPH (priv->item), priv->layout);

        g_free (text);
    }
    else if (priv->item && TEXT_IS_IMAGE (priv->item))
    {
        width = 100;
        height = 100;
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

    priv->bbox.x = offset_x;
    priv->bbox.y = offset_y;
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
text_layout_box_get_item (TextLayoutBox *self)
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
    TextLayoutBoxPrivate *priv = text_layout_box_get_instance_private (self);
}
