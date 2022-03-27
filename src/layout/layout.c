/* layout.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
                               TextFrame    *frame,
                               int           width)
{
    g_return_if_fail (TEXT_IS_LAYOUT (self));
    g_return_if_fail (TEXT_IS_FRAME (frame));

    TextLayoutBox *root = text_layout_box_new ();
    do_layout_recursive (self, root, context, TEXT_ITEM (frame), width);
    return root;
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
