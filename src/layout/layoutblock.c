/* layoutblock.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "layoutblock.h"

#include "../model/paragraph.h"
#include "../model/image.h"

#include "layoutinline.h"

typedef struct
{
    PangoLayout *layout;
} TextLayoutBlockPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (TextLayoutBlock, text_layout_block, TEXT_TYPE_LAYOUT_BOX)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextLayoutBlock *
text_layout_block_new ()
{
    return TEXT_LAYOUT_BLOCK (g_object_new (TEXT_TYPE_LAYOUT_BLOCK, NULL));
}

static void
text_layout_block_finalize (GObject *object)
{
    TextLayoutBlock *self = (TextLayoutBlock *)object;
    TextLayoutBlockPrivate *priv = text_layout_block_get_instance_private (self);

    G_OBJECT_CLASS (text_layout_block_parent_class)->finalize (object);
}

static void
text_layout_block_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    TextLayoutBlock *self = TEXT_LAYOUT_BLOCK (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_layout_block_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    TextLayoutBlock *self = TEXT_LAYOUT_BLOCK (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
_set_inline_attribute (TextOpaque    *opaque,
                       PangoAttrList *list,
                       int            start_index,
                       int            run_length)
{
    PangoAttribute *attr;
    PangoRectangle rect;

    // Get Style Properties
    rect.width = 100 * PANGO_SCALE;
    rect.height = 100 * PANGO_SCALE;

    // Shape Attribute
    attr = pango_attr_shape_new (&rect, &rect);
    attr->start_index = start_index;
    attr->end_index = start_index + run_length;
    pango_attr_list_insert (list, attr);
}

static void
_set_run_attribute (TextRun       *run,
                    PangoAttrList *list,
                    int            start_index,
                    int            run_length)
{
    gboolean is_bold, is_italic, is_underline;
    PangoAttribute *attr;

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
}

void
_set_attributes (TextParagraph *paragraph,
                 PangoLayout   *pango_layout)
{
    TextNode *fragment;
    PangoAttrList *list;

    int start_index;

    list = pango_attr_list_new();

    g_return_if_fail (TEXT_IS_PARAGRAPH (paragraph));

    start_index = 0;

    for (fragment = text_node_get_first_child (TEXT_NODE (paragraph));
         fragment != NULL;
         fragment = text_node_get_next (fragment))
    {
        int run_length;

        run_length = text_fragment_get_size_bytes (TEXT_FRAGMENT (fragment));

        if (TEXT_IS_RUN (fragment))
            _set_run_attribute (TEXT_RUN (fragment), list, start_index, run_length);
        else if (TEXT_IS_OPAQUE (fragment))
            _set_inline_attribute (TEXT_OPAQUE (fragment), list, start_index, run_length);

        start_index += run_length;
    }

    pango_layout_set_attributes (pango_layout, list);
}

static void
text_layout_block_layout (TextLayoutBox *self,
                          PangoContext  *context,
                          int            width,
                          int            offset_x,
                          int            offset_y)
{
    TextLayoutBlockPrivate *priv;
    TextDimensions *bbox;
    TextItem *item;

    int height;

    TextNode *iter;
    TextNode *fragment_iter;
    int byte_offset;
    int child_offset_y;

    g_return_if_fail (TEXT_IS_LAYOUT_BLOCK (self));

    priv = text_layout_block_get_instance_private (TEXT_LAYOUT_BLOCK (self));
    bbox = text_layout_box_get_mutable_bbox (self);
    item = text_layout_box_get_item (self);

    height = 0;

    if (item && TEXT_IS_PARAGRAPH (item))
    {
        gchar *text;
        text = text_paragraph_get_text (TEXT_PARAGRAPH (item));

        if (!priv->layout)
            priv->layout = pango_layout_new (context);

        // Set style information
        // TODO: Matching from ruleset
        _set_attributes (TEXT_PARAGRAPH (item), priv->layout);

        // Determine height (must be done *after* attributes are set)
        pango_layout_set_text (priv->layout, text, -1);
        pango_layout_set_wrap (priv->layout, PANGO_WRAP_WORD_CHAR);
        pango_layout_set_width (priv->layout, PANGO_SCALE * width);
        pango_layout_get_pixel_size (priv->layout, NULL, &height);

        g_free (text);
    }

    // Account for children

    byte_offset = 0;
    child_offset_y = 0;
    fragment_iter = text_node_get_first_child (TEXT_NODE (item));

    for (iter = text_node_get_first_child (TEXT_NODE (self));
         iter != NULL;
         iter = text_node_get_next (TEXT_NODE (iter)))
    {
        const TextDimensions *child_bbox;
        TextLayoutBox *child_box = TEXT_LAYOUT_BOX (iter);

        // Treat children differently depending on whether they
        // are inline or block elements
        // TODO: Force to choose between inline OR block children
        if (TEXT_IS_LAYOUT_BLOCK (iter))
        {
            text_layout_box_layout (child_box, context, width, offset_x, offset_y + child_offset_y);

            child_bbox = text_layout_box_get_bbox (child_box);
            child_offset_y += (int) child_bbox->height;
        }
        else if (TEXT_IS_LAYOUT_INLINE (iter))
        {
            PangoRectangle rect;
            TextItem *fragment;

            fragment = text_layout_box_get_item (TEXT_LAYOUT_BOX (iter));

            // Check pango layout
            if (!PANGO_IS_LAYOUT (priv->layout))
            {
                g_warning ("%s has inline child but no text layout.\n",
                           g_type_name (text_layout_block_get_type()));
                continue;
            }

            // Get correct byte_index for fragment
            while (TEXT_FRAGMENT (fragment_iter) != TEXT_FRAGMENT (fragment))
            {
                byte_offset += text_fragment_get_size_bytes (TEXT_FRAGMENT (fragment_iter));
                fragment_iter = text_node_get_next (fragment_iter);

                // InlineBox must point to a valid item in the paragraph
                g_assert (fragment_iter != NULL);
            }

            // Get starting x,y position of run at this index
            pango_layout_index_to_pos (priv->layout, byte_offset, &rect);

            text_layout_box_layout (TEXT_LAYOUT_BOX (iter), context, width,
                                    rect.x / PANGO_SCALE,
                                    rect.y / PANGO_SCALE);
        }
    }

    height += child_offset_y;

    bbox->x = offset_x;
    bbox->y = offset_y;
    bbox->width = width;
    bbox->height = height;
}

PangoLayout *
text_layout_block_get_pango_layout (TextLayoutBlock *self)
{
    TextLayoutBlockPrivate *priv = text_layout_block_get_instance_private (self);
    return priv->layout;
}

static void
text_layout_block_class_init (TextLayoutBlockClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_layout_block_finalize;
    object_class->get_property = text_layout_block_get_property;
    object_class->set_property = text_layout_block_set_property;

    TextLayoutBoxClass *layout_box_class = TEXT_LAYOUT_BOX_CLASS (klass);

    layout_box_class->layout = text_layout_block_layout;
}

static void
text_layout_block_init (TextLayoutBlock *self)
{
    TextLayoutBlockPrivate *priv = text_layout_block_get_instance_private (self);
}
