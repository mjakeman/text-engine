/* paragraph.c
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
text_paragraph_append_run (TextParagraph *self,
                           TextRun       *run)
{
    g_return_if_fail (TEXT_IS_PARAGRAPH (self));
    g_return_if_fail (TEXT_IS_RUN (run));

    text_node_append_child (TEXT_NODE (self), TEXT_NODE (run));
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
        g_assert (TEXT_IS_RUN (child));
        length += text_run_get_length (TEXT_RUN (child));
    }

    return length;
}

TextRun *
text_paragraph_get_run_at_index (TextParagraph *self,
                                 int            index,
                                 int           *starting_index)
{
    TextNode *child;
    int length;

    length = 0;

    g_return_val_if_fail (TEXT_IS_PARAGRAPH (self), NULL);

    if (index == 0)
    {
        TextNode *first;
        first = text_node_get_first_child (TEXT_NODE (self));

        if (starting_index)
            *starting_index = 0;
        return TEXT_RUN (first);
    }

    for (child = text_node_get_first_child (TEXT_NODE (self));
         child != NULL;
         child = text_node_get_next (child))
    {
        int delta_length;
        g_assert (TEXT_IS_RUN (child));
        delta_length = text_run_get_length (TEXT_RUN (child));

        // Index is considered part of a run if it is immediately
        // after the last character in the run. For example:
        // There is a cursor position at the end of a run
        //
        //     `Once upon a time there was a little dog, `
        //                                               ^
        //                 this index is part of the run /
        //
        if (length < index && index <= length + delta_length)
        {
            if (starting_index)
                *starting_index = length;
            return TEXT_RUN (child);
        }

        length += delta_length;
    }

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
