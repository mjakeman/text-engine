/* editor.c
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

#include "editor.h"

#include "../model/paragraph.h"

struct _TextEditor
{
    GObject parent_instance;

    TextDocument *document;
};

G_DEFINE_FINAL_TYPE (TextEditor, text_editor, G_TYPE_OBJECT)

enum {
    PROP_0,
    PROP_DOCUMENT,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextEditor *
text_editor_new (TextDocument *document)
{
    return g_object_new (TEXT_TYPE_EDITOR,
                         "document", document,
                         NULL);
}

static void
text_editor_finalize (GObject *object)
{
    TextEditor *self = (TextEditor *)object;

    G_OBJECT_CLASS (text_editor_parent_class)->finalize (object);
}

static void
text_editor_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    TextEditor *self = TEXT_EDITOR (object);

    switch (prop_id)
    {
    case PROP_DOCUMENT:
        g_value_set_object (value, self->document);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_editor_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    TextEditor *self = TEXT_EDITOR (object);

    switch (prop_id)
    {
    case PROP_DOCUMENT:
        self->document = g_value_get_object (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_editor_class_init (TextEditorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_editor_finalize;
    object_class->get_property = text_editor_get_property;
    object_class->set_property = text_editor_set_property;

    properties [PROP_DOCUMENT]
        = g_param_spec_object ("document",
                               "Document",
                               "Document",
                               TEXT_TYPE_DOCUMENT,
                               G_PARAM_READWRITE);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static TextItem *
go_up (TextItem *item,
       gboolean  forwards)
{
    TextNode *parent;
    TextNode *sibling;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    printf ("item: %s\n", g_type_name_from_instance ((GTypeInstance *) item));

    parent = text_node_get_parent (TEXT_NODE (item));

    printf ("parent: %s\n", g_type_name_from_instance ((GTypeInstance *) parent));

    if (parent && TEXT_IS_ITEM (parent))
    {
        sibling = forwards
            ? text_node_get_next (parent)
            : text_node_get_previous (parent);

        printf ("sibling: %s\n", g_type_name_from_instance ((GTypeInstance *) sibling));

        if (sibling && TEXT_IS_ITEM (sibling))
        {
            printf("sibling\n");
            return TEXT_ITEM (sibling);
        }
        else
        {
            printf("recurse\n");
            return go_up (TEXT_ITEM (parent), forwards);
        }
    }

    return NULL;
}

static TextRun *
walk_until_previous_run (TextItem *item)
{
    TextNode *child;
    TextNode *sibling;
    TextItem *parent;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    child = text_node_get_last_child (TEXT_NODE (item));

    if (child && TEXT_IS_ITEM (child)) {
        if (TEXT_IS_RUN (child)) {
            return TEXT_RUN (child);
        }

        return walk_until_previous_run (TEXT_ITEM (child));
    }

    sibling = text_node_get_previous (TEXT_NODE (item));

    if (sibling && TEXT_IS_ITEM (sibling)) {
        if (TEXT_IS_RUN (sibling)) {
            return TEXT_RUN (sibling);
        }

        return walk_until_previous_run (TEXT_ITEM (sibling));
    }

    parent = go_up (item, FALSE);

    if (parent) {
        if (TEXT_IS_RUN (parent)) {
            return TEXT_RUN (parent);
        }

        return walk_until_previous_run (parent);
    }

    return NULL;
}

static TextRun *
walk_until_next_run (TextItem *item)
{
    TextNode *child;
    TextNode *sibling;
    TextItem *parent;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    child = text_node_get_first_child (TEXT_NODE (item));

    if (child && TEXT_IS_ITEM (child)) {
        if (TEXT_IS_RUN (child)) {
            return TEXT_RUN (child);
        }

        return walk_until_next_run (TEXT_ITEM (child));
    }

    sibling = text_node_get_next (TEXT_NODE (item));

    if (sibling && TEXT_IS_ITEM (sibling)) {
        if (TEXT_IS_RUN (sibling)) {
            return TEXT_RUN (sibling);
        }

        return walk_until_next_run (TEXT_ITEM (sibling));
    }

    parent = go_up (item, TRUE);

    if (parent) {
        if (TEXT_IS_RUN (parent)) {
            return TEXT_RUN (parent);
        }

        return walk_until_next_run (parent);
    }

    return NULL;
}

void
text_editor_move_left (TextEditor *self)
{
    TextRun *prev;
    TextMark *cursor;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    cursor = self->document->cursor;

    if (cursor->index - 1 < 0) {
        prev = walk_until_previous_run (TEXT_ITEM (cursor->parent));

        if (prev) {
            int length;
            char *text;

            g_assert (TEXT_IS_RUN (prev));
            g_object_get (prev, "text", &text, NULL);
            length = strlen (text);

            cursor->parent = prev;
            cursor->index = length;
            return;
        }

        // do not move at all
        printf("Cannot move left!\n");
        return;
    }

    cursor->index--;
}

void
text_editor_move_right (TextEditor *self)
{
    int length;
    char *text;
    TextRun *next;
    TextMark *cursor;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    cursor = self->document->cursor;

    if (!cursor->parent)
        return;

    g_object_get (cursor->parent, "text", &text, NULL);
    length = strlen (text);

    if (cursor->index + 1 > length) {
        next = walk_until_next_run (TEXT_ITEM (cursor->parent));

        if (next) {
            cursor->parent = next;
            cursor->index = 0;
            return;
        }

        // do not move at all
        printf("Cannot move right!\n");
        return;
    }

    cursor->index++;
}

void
text_editor_move_first (TextEditor *self)
{
    TextMark *cursor;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    cursor = self->document->cursor;

    cursor->parent = walk_until_next_run (TEXT_ITEM (self->document->frame));
    cursor->index = 0;
}

void
text_editor_move_last (TextEditor *self)
{
    TextMark *cursor;
    char *text;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    cursor = self->document->cursor;

    cursor->parent = walk_until_previous_run (TEXT_ITEM (self->document->frame));
    cursor->index = 0;

    if (cursor->parent)
    {
        g_object_get (cursor->parent, "text", &text, NULL);
        cursor->index = strlen (text);
    }
}

static void
_ensure_paragraph (TextEditor *self)
{
    TextFrame *document_frame;
    TextParagraph *paragraph;

    document_frame = self->document->frame;

    // Add paragraph with run if none exists
    if (!walk_until_next_run (TEXT_ITEM (document_frame)))
    {
        paragraph = text_paragraph_new ();
        text_frame_append_block (document_frame, TEXT_BLOCK (paragraph));
        text_paragraph_append_run (paragraph, text_run_new (""));
    }
}

void
text_editor_delete (TextEditor *self)
{
    char *text;
    int length;
    GString *modified;
    TextMark *cursor;
    TextNode *node;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    cursor = self->document->cursor;

    if (!TEXT_IS_RUN (cursor->parent)) {
        cursor->parent = walk_until_next_run (TEXT_ITEM (self->document->frame));
    }

    // TODO: Replace with hybrid tree/piece-table structure?
    // Textual data is stored in buffers and indexed by the tree
    g_object_get (cursor->parent, "text", &text, NULL);
    length = strlen (text);

    // Where the run length is 1
    if (length - 1 <= 0)
    {
        node = text_node_get_parent (TEXT_NODE (cursor->parent));
        if (TEXT_IS_PARAGRAPH (node) &&
            text_node_get_num_children (node) == 1)
        {
            // Delete paragraph if we are the last run
            cursor->parent = walk_until_previous_run (TEXT_ITEM (cursor->parent));
            text_node_delete (&node);

            // Ensure there is a paragraph remaining
            _ensure_paragraph (self);

            return;
        }

        // Delete run
        node = TEXT_NODE (cursor->parent);
        cursor->parent = walk_until_previous_run (TEXT_ITEM (cursor->parent));
        text_node_delete (&node);

        return;
    }

    // Where the cursor is at the end of the run
    if (cursor->index == length)
    {
        // End of run
        /*node = TEXT_NODE (walk_until_next_run (cursor->parent));
        text_editor_delete ()*/
        g_print ("Cannot delete at end of run\n");
        return;
    }

    modified = g_string_new (text);
    modified = g_string_erase (modified, cursor->index, 1);
    g_object_set (cursor->parent, "text", modified->str, NULL);
    g_string_free (modified, TRUE);

    // TODO: Update/invalidate all marks

    // Move left/right by one
    // TODO: Move by the amount changed
    // This should be handled by an auxiliary anchor object which
    // remains fixed at a given point in the text no matter how
    // the text changes (i.e. a cursor).
    // text_editor_move_right (self);
}

void
text_editor_insert (TextEditor *self,
                    gchar      *str)
{
    // Encapsulates insertion inside an editor module/object.
    // This should accept user input in the form of Operational
    // Transformation commands. This will aid with undo/redo.

    char *text;
    GString *modified;
    TextMark *cursor;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    cursor = self->document->cursor;

    if (!TEXT_IS_RUN (cursor->parent)) {
        cursor->parent = walk_until_next_run (TEXT_ITEM (self->document->frame));
    }

    // TODO: Replace with hybrid tree/piece-table structure?
    // Textual data is stored in buffers and indexed by the tree
    g_object_get (cursor->parent, "text", &text, NULL);
    modified = g_string_new (text);
    modified = g_string_insert (modified, cursor->index, str);
    g_object_set (cursor->parent, "text", modified->str, NULL);
    g_string_free (modified, TRUE);

    // TODO: Update all marks

    // Move left/right by one
    // TODO: Move by the amount changed
    // This should be handled by an auxiliary anchor object which
    // remains fixed at a given point in the text no matter how
    // the text changes (i.e. a cursor).
    text_editor_move_right (self);
}

static void
text_editor_init (TextEditor *self)
{
}
