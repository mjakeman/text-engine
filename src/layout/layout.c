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

typedef struct
{
    int padding;
} TextLayoutPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (TextLayout, text_layout, G_TYPE_OBJECT)

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

TextLayoutBox *
build_layout_tree_recursive (TextLayout    *self,
                             PangoContext  *context,
                             TextItem      *item)
{
    TextLayoutBox *box;
    TextNode *iter;

    g_return_val_if_fail (TEXT_IS_LAYOUT (self), NULL);
    g_return_val_if_fail (PANGO_IS_CONTEXT (context), NULL);
    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    // Construct a layout item for this node using the item factory
    // Subclasses can override this to add and use their own items
    box = TEXT_LAYOUT_GET_CLASS (self)->item_factory (TEXT_ITEM (item));

    // For now, if a node does not provide a LayoutBox then we assume
    // it and its children are invisible. Perhaps we want to introduce
    // some kind of LayoutAnonymousBox which is transparently skipped by
    // the layout engine.
    if (!TEXT_IS_LAYOUT_BOX (box))
        return NULL;

    // Setup Box
    text_layout_box_set_item (box, item);
    text_item_detach (TEXT_ITEM (item)); // TODO: Move to a 'cleanup_tree' function?
    text_item_attach (TEXT_ITEM (item), TEXT_NODE (box));

    // Append children (in future, recursively)
    for (iter = text_node_get_first_child (TEXT_NODE (item));
         iter != NULL;
         iter = text_node_get_next (iter))
    {
        TextLayoutBox *child_box;

        g_assert (TEXT_IS_ITEM(iter));

        child_box = build_layout_tree_recursive (self, context, TEXT_ITEM (iter));

        if (TEXT_IS_LAYOUT_BOX (child_box))
        {
            text_node_append_child (TEXT_NODE (box), TEXT_NODE (child_box));
        }
    }

    return box;
}

TextLayoutBox *
text_layout_default_item_factory (TextItem *item)
{
    GType type;
    type = G_TYPE_FROM_INSTANCE (item);

    // Go from most specific to least specific, otherwise
    // we could accidentally create the wrong layout box
    // by taking the base class instead of the subclass

    // Images
    if (type == TEXT_TYPE_IMAGE)
        return NULL;

    // Paragraphs
    if (type == TEXT_TYPE_PARAGRAPH)
        return TEXT_LAYOUT_BOX (text_layout_block_new ());

    // Text Runs
    if (type == TEXT_TYPE_RUN)
        return NULL;

    // Frames
    if (type == TEXT_TYPE_FRAME)
        return TEXT_LAYOUT_BOX (text_layout_block_new ());

    // It is an error to provide a type for which no layout
    // item exists - we cannot display it.
    g_critical ("Cannot create layout item for type '%s'.",
                g_type_name (type));

    return NULL;
}

TextLayoutBox *
text_layout_build_layout_tree (TextLayout   *self,
                               PangoContext *context,
                               TextFrame    *frame,
                               int           width)
{
    TextLayoutBox *root;

    g_return_val_if_fail (TEXT_IS_LAYOUT (self), NULL);
    g_return_val_if_fail (TEXT_IS_FRAME (frame), NULL);

    root = build_layout_tree_recursive (self, context, TEXT_ITEM (frame));
    text_layout_box_layout (root, context, width, 0, 0);
    return root;
}

TextLayoutBox *
text_layout_find_above (TextLayoutBox *item)
{
    g_return_val_if_fail (TEXT_IS_LAYOUT_BOX (item), NULL);
    return TEXT_LAYOUT_BOX (text_node_get_previous (TEXT_NODE (item)));
}

TextLayoutBox *
text_layout_find_below (TextLayoutBox *item)
{
    g_return_val_if_fail (TEXT_IS_LAYOUT_BOX (item), NULL);
    return TEXT_LAYOUT_BOX (text_node_get_next (TEXT_NODE (item)));
}

TextLayoutBox *
text_layout_pick_internal (TextLayoutBox  *root,
                           double          x,
                           double          y,
                           double         *min_y_distance,
                           TextLayoutBox **min_y_layout)
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
        const TextDimensions *bbox;
        double dist_to_y;

        layout_item = TEXT_LAYOUT_BOX (child);
        bbox = text_layout_box_get_bbox (layout_item);

        // Recursively check child layouts first
        found = TEXT_NODE (text_layout_pick_internal (layout_item, x - bbox->x, y - bbox->y, min_y_distance, min_y_layout));

        if (found) {
            return TEXT_LAYOUT_BOX (found);
        }

        // Check if the cursor is fully within the bounding box
        if (x >= bbox->x &&
            y >= bbox->y &&
            x <= bbox->x + bbox->width &&
            y <= bbox->y + bbox->height)
        {
            return layout_item;
        }

        // Check if the cursor is vertically within the bounding box
        if (y >= bbox->y &&
            y <= bbox->y + bbox->height)
        {
            *min_y_distance = 0;
            *min_y_layout = layout_item;
            continue;
        }

        // Finally check the vertical offset from the cursor to the
        // closest edge of the bounding box
        dist_to_y = (y < bbox->y)
                ? bbox->y - y
                : y - bbox->y;

        if (dist_to_y < *min_y_distance) {
            *min_y_distance = dist_to_y;
            *min_y_layout = layout_item;
        }
    }

    return NULL;
}

TextLayoutBox *
text_layout_pick (TextLayoutBox *root,
                  int            x,
                  int            y)
{
    double min_y_distance = G_MAXDOUBLE;
    TextLayoutBox *min_y_layout = NULL;
    TextLayoutBox *result;

    result = text_layout_pick_internal(root, x, y, &min_y_distance, &min_y_layout);

    if (result) {
        return result;
    }

    // Otherwise return nearest layout
    return min_y_layout;
}

static void
text_layout_class_init (TextLayoutClass *klass)
{
    klass->item_factory = text_layout_default_item_factory;

    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_layout_finalize;
    object_class->get_property = text_layout_get_property;
    object_class->set_property = text_layout_set_property;
}

static void
text_layout_init (TextLayout *self)
{
}
