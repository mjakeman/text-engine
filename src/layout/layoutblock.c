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

    TextLayoutBox *inline_box;
    const TextDimensions *bbox;

    inline_box = TEXT_LAYOUT_BOX (text_item_get_attachment (TEXT_ITEM (opaque)));
    bbox = text_layout_box_get_bbox (inline_box);

    g_assert (TEXT_IS_LAYOUT_INLINE (inline_box));

    // Get Style Properties
    rect.width = (int) bbox->width * PANGO_SCALE;
    rect.height = (int) bbox->height * PANGO_SCALE;

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
do_block_layout (TextLayoutBox *self,
                 PangoContext  *context,
                 int            width,
                 int            offset_x,
                 int            offset_y)
{
    TextNode *iter;
    TextDimensions *bbox;

    int child_offset_y;
    int height;

    height = 0;
    child_offset_y = 0;
    bbox = text_layout_box_get_mutable_bbox (self);

    // Recompute child element offset
    for (iter = text_node_get_first_child (TEXT_NODE (self));
         iter != NULL;
         iter = text_node_get_next (TEXT_NODE (iter)))
    {
        const TextDimensions *child_bbox;
        TextLayoutBox *child_box = TEXT_LAYOUT_BOX (iter);

        g_assert (TEXT_IS_LAYOUT_BLOCK (iter));

        text_layout_box_layout (child_box, context, width, offset_x, offset_y + child_offset_y);

        child_bbox = text_layout_box_get_bbox (child_box);
        child_offset_y += (int) child_bbox->height;
    }

    height += child_offset_y;

    bbox->x = offset_x;
    bbox->y = offset_y;
    bbox->width = width;
    bbox->height = height;
}

static void
do_inline_layout (TextLayoutBox *self,
                  PangoContext  *context,
                  int            width,
                  int            offset_x,
                  int            offset_y)
{
    TextNode *iter;
    TextItem *item;
    TextDimensions *bbox;
    TextLayoutBlockPrivate *priv;

    int height;
    int byte_offset;

    item = text_layout_box_get_item (self);
    priv = text_layout_block_get_instance_private (TEXT_LAYOUT_BLOCK (self));
    bbox = text_layout_box_get_mutable_bbox (self);

    // Precompute inline children requested size
    for (iter = text_node_get_first_child (TEXT_NODE (self));
         iter != NULL;
         iter = text_node_get_next (TEXT_NODE (iter)))
    {
        // Get fixed size so we can set pango attributes accordingly
        g_assert (TEXT_IS_LAYOUT_INLINE (iter));
        text_layout_box_layout (TEXT_LAYOUT_BOX (iter), context, 0, 0, 0);
    }

    // Setup pango layout
    if (item && TEXT_IS_PARAGRAPH (item))
    {
        gchar *text;
        text = text_paragraph_get_text (TEXT_PARAGRAPH (item));

        if (!priv->layout)
            priv->layout = pango_layout_new (context);

        // Set style information
        // TODO: Matching from ruleset
        _set_attributes (TEXT_PARAGRAPH (item), priv->layout);

        // Set basic layout properties
        pango_layout_set_text (priv->layout, text, -1);
        pango_layout_set_wrap (priv->layout, PANGO_WRAP_WORD_CHAR);
        pango_layout_set_width (priv->layout, PANGO_SCALE * width);
        pango_layout_get_pixel_size (priv->layout, NULL, &height);
        g_free (text);
    }

    // Recompute x/y offsets of inline children

    byte_offset = 0;
    for (iter = text_node_get_first_child (TEXT_NODE (item));
         iter != NULL;
         iter = text_node_get_next (TEXT_NODE (iter)))
    {
        TextNode *inline_box;
        PangoRectangle rect;

        // Try to get layout item attachment
        inline_box = text_item_get_attachment (TEXT_ITEM (iter));

        if (TEXT_IS_LAYOUT_INLINE (inline_box))
        {
            // Get starting x,y position of run at this index
            pango_layout_index_to_pos (priv->layout, byte_offset, &rect);

            // Re-layout child with new x/y offset
            text_layout_box_layout (TEXT_LAYOUT_BOX (inline_box), context, 0,
                                    rect.x / PANGO_SCALE,
                                    rect.y / PANGO_SCALE);
        }

        // Increase byte offset into the paragraph
        byte_offset += text_fragment_get_size_bytes (TEXT_FRAGMENT (iter));
    }

    bbox->x = offset_x;
    bbox->y = offset_y;
    bbox->width = width;
    bbox->height = height;
}

static void
text_layout_block_layout (TextLayoutBox *self,
                          PangoContext  *context,
                          int            width,
                          int            offset_x,
                          int            offset_y)
{
    TextNode *first_child;

    g_return_if_fail (TEXT_IS_LAYOUT_BLOCK (self));

    // Blocks can only contain all block children OR
    // all inline children, but not a mixture of each.
    first_child = text_node_get_first_child (TEXT_NODE (self));

    if (TEXT_IS_LAYOUT_BLOCK (first_child))
        do_block_layout (self, context, width, offset_x, offset_y);
    else
        do_inline_layout (self, context, width, offset_x, offset_y);
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
