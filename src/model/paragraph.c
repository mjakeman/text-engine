/* paragraph.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "paragraph.h"

struct _TextParagraph
{
    TextBlock parent_instance;
};

G_DEFINE_FINAL_TYPE (TextParagraph, text_paragraph, TEXT_TYPE_BLOCK)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextParagraph *
text_paragraph_new (void)
{
    return g_object_new (TEXT_TYPE_PARAGRAPH, NULL);
}

static void
text_paragraph_finalize (GObject *object)
{
    TextParagraph *self = (TextParagraph *)object;

    G_OBJECT_CLASS (text_paragraph_parent_class)->finalize (object);
}

static void
text_paragraph_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    TextParagraph *self = TEXT_PARAGRAPH (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

static void
text_paragraph_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    TextParagraph *self = TEXT_PARAGRAPH (object);

    switch (prop_id)
      {
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      }
}

void
text_paragraph_append_fragment (TextParagraph *self,
                                TextFragment  *fragment)
{
    g_return_if_fail (TEXT_IS_PARAGRAPH (self));
    g_return_if_fail (TEXT_IS_FRAGMENT (fragment));

    text_node_append_child (TEXT_NODE (self), TEXT_NODE (fragment));
}

/**
 * text_paragraph_get_text:
 *
 * Returns a duplicate string containing the contents of
 * this paragraph. The contents is valid as of the instant
 * the method is called and will not be updated. It is the
 * caller's responsibility to free the returned string.
 *
 * @self: The `TextParagraph` instance.
 *
 * Returns: A pointer to the text content of this paragraph
 */
char *
text_paragraph_get_text (TextParagraph *self)
{
    TextNode *child;
    GString *str;

    g_return_val_if_fail (TEXT_IS_PARAGRAPH (self), NULL);

    str = g_string_new ("");

    for (child = text_node_get_first_child (TEXT_NODE (self));
         child != NULL;
         child = text_node_get_next (child))
    {
        const gchar *run_text;

        g_assert (TEXT_IS_FRAGMENT (child));

        run_text = text_fragment_get_text (TEXT_FRAGMENT (child));
        g_string_append (str, run_text);
    }

    return g_string_free (str, FALSE);
}

int
text_paragraph_get_length (TextParagraph *self)
{
    TextNode *child;
    int length;

    g_return_val_if_fail (TEXT_IS_PARAGRAPH (self), -1);

    length = 0;

    for (child = text_node_get_first_child (TEXT_NODE (self));
         child != NULL;
         child = text_node_get_next (child))
    {
        length += text_fragment_get_length (TEXT_FRAGMENT (child));
    }

    return length;
}

int
text_paragraph_get_size_bytes (TextParagraph *self)
{
    TextNode *child;
    int length;

    g_return_val_if_fail (TEXT_IS_PARAGRAPH (self), -1);

    length = 0;

    for (child = text_node_get_first_child (TEXT_NODE (self));
         child != NULL;
         child = text_node_get_next (child))
    {
        length += (int) strlen (text_fragment_get_text ((TEXT_FRAGMENT (child))));
    }

    return length;
}

TextFragment *
text_paragraph_get_item_at_index (TextParagraph *self,
                                  int            byte_index,
                                  int           *starting_index)
{
    TextNode *child;
    int length;

    length = 0;

    g_return_val_if_fail (TEXT_IS_PARAGRAPH (self), NULL);

    if (byte_index == 0)
    {
        TextNode *first;
        first = text_node_get_first_child (TEXT_NODE (self));

        if (starting_index)
            *starting_index = 0;
        return TEXT_FRAGMENT (first);
    }

    for (child = text_node_get_first_child (TEXT_NODE (self));
         child != NULL;
         child = text_node_get_next (child))
    {
        int delta_size;
        g_assert (TEXT_IS_FRAGMENT (child));
        delta_size = text_fragment_get_size_bytes (TEXT_FRAGMENT (child));

        // Index is considered part of a run if it is immediately
        // after the last character in the run. For example:
        // There is a cursor position at the end of a run
        //
        //     `Once upon a time there was a little dog, `
        //                                               ^
        //                 this index is part of the run /
        //
        if (length < byte_index && byte_index <= length + delta_size)
        {
            if (starting_index)
                *starting_index = length;
            return TEXT_FRAGMENT (child);
        }

        length += delta_size;
    }

    g_critical ("Invalid index: %d passed to text_paragraph_get_item_at_index ()\n", byte_index);

    if (starting_index)
        *starting_index = -1;
    return NULL;
}

static void
text_paragraph_class_init (TextParagraphClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_paragraph_finalize;
    object_class->get_property = text_paragraph_get_property;
    object_class->set_property = text_paragraph_set_property;
}

static void
text_paragraph_init (TextParagraph *self)
{
}
