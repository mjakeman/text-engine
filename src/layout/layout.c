/* layout.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "layout.h"

struct _TextLayout
{
    GObject parent_instance;
};

G_DEFINE_FINAL_TYPE (TextLayout, text_layout, G_TYPE_OBJECT)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextLayout *
text_layout_new (void)
{
    return g_object_new (TEXT_TYPE_LAYOUT, NULL);
}

static void
text_layout_finalize (GObject *object)
{
    TextLayout *self = (TextLayout *)object;

    G_OBJECT_CLASS (text_layout_parent_class)->finalize (object);
}

static void
text_layout_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    TextLayout *self = TEXT_LAYOUT (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_layout_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    TextLayout *self = TEXT_LAYOUT (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
do_layout_recursive (TextLayout    *self,
                     TextLayoutBox *parent,
                     PangoContext  *context,
                     TextMark      *cursor,
                     TextItem      *item,
                     int            width)
{
    g_return_if_fail (TEXT_IS_LAYOUT (self));
    g_return_if_fail (TEXT_IS_LAYOUT_BOX (parent));
    g_return_if_fail (TEXT_IS_ITEM (item));

    // Append children (in future, recursively)
    for (TextNode *node = text_node_get_first_child (TEXT_NODE (item));
         node != NULL;
         node = text_node_get_next (node))
    {
        g_assert (TEXT_IS_ITEM (node));

        g_debug ("Counting child %s\n", g_type_name_from_instance (node));

        // Let's treat paragraphs opaquely for now. In the future, we need
        // to manually consider each text run in order for inline equations
        // and images.
        if (TEXT_IS_PARAGRAPH (node))
        {
            TextLayoutBox *box = text_layout_box_new ();
            text_layout_box_set_item (box, TEXT_ITEM (node));
            text_item_detach (TEXT_ITEM (node)); // TODO: Don't do this
            text_item_attach (TEXT_ITEM (node), TEXT_NODE (box));

            // TODO: We should handle runs inline here, rather than
            // paragraphs. As we do not consider individual runs, we
            // must check whether the paragraph contains a cursor and
            // then handle layout for the cursor object.
            if (TEXT_PARAGRAPH (node) == cursor->paragraph)
            {
                // Paragraph contains cursor
                text_layout_box_set_cursor (box, cursor->index);
            }

            text_node_append_child (TEXT_NODE (parent), TEXT_NODE (box));
            g_debug ("Added child %s\n", g_type_name_from_instance (node));

            // TODO: This function should be properly recursive in the future,
            // so avoid calling it here. Below should be the only time it is
            // called (i.e. post-order traversal).
            text_layout_box_layout (box, context, width);
        }
    }

    text_layout_box_layout (parent, context, width);
    g_debug ("Layout for %s\n", g_type_name_from_instance (parent));
}

TextLayoutBox *
text_layout_build_layout_tree (TextLayout   *self,
                               PangoContext *context,
                               TextMark     *cursor,
                               TextFrame    *frame,
                               int           width)
{
    g_return_val_if_fail (TEXT_IS_LAYOUT (self), NULL);
    g_return_val_if_fail (TEXT_IS_FRAME (frame), NULL);

    TextLayoutBox *root = text_layout_box_new ();
    do_layout_recursive (self, root, context, cursor, TEXT_ITEM (frame), width);
    return root;
}

TextLayoutBox *
text_layout_pick (TextLayoutBox *root,
                  int            x,
                  int            y)
{
    // Note: 'x' and 'y' are relative to the document origin
    TextNode *child;
    TextNode *found;

    g_return_val_if_fail (TEXT_IS_LAYOUT_BOX (root), NULL);

    for (child = text_node_get_first_child (TEXT_NODE (root));
         child != NULL;
         child = text_node_get_next (child))
    {
        TextLayoutBox *layout_item;
        TextDimensions *bbox;

        layout_item = TEXT_LAYOUT_BOX (child);
        bbox = text_layout_box_get_bbox (layout_item);

        if (x >= bbox->x &&
            y >= bbox->y &&
            x <= bbox->x + bbox->width &&
            y <= bbox->y + bbox->height)
        {
            // Recursively check child layouts first
            found = text_layout_pick (layout_item, x, y);

            if (found) {
                return found;
            }

            return layout_item;
        }
    }

    return NULL;
}

static void
text_layout_class_init (TextLayoutClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_layout_finalize;
    object_class->get_property = text_layout_get_property;
    object_class->set_property = text_layout_set_property;
}

static void
text_layout_init (TextLayout *self)
{
}
