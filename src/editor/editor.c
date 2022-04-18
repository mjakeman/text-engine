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
text_editor_move_left (TextEditor *self,
                       TextMark   *mark,
                       int         amount)
{
    // TODO: Why does this overstep by 1 index?

    TextRun *iter;
    int amount_moved;
    int iter_length;
    gboolean first;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (amount > 0);

    iter = mark->parent;
    amount_moved = 0;
    first = TRUE;

    // Handle first run
    if (mark->index - amount >= 0)
    {
        mark->index -= amount;
        return;
    }

    amount_moved += mark->index;
    iter = walk_until_previous_run (TEXT_ITEM (iter));
    iter_length = text_run_get_length (iter);

    while (amount_moved < amount)
    {
        // Could be NULL (e.g. start of document)
        if (!TEXT_IS_ITEM (iter))
            return;

        iter_length = text_run_get_length (iter);

        // Run is entirely contained within amount to move
        if (amount_moved + iter_length < amount)
        {
            iter = walk_until_previous_run (TEXT_ITEM (iter));
            amount_moved += iter_length;
            continue;
        }

        break;
    }

    mark->index = iter_length - (amount - amount_moved);
    mark->parent = iter;
}

void
text_editor_move_right (TextEditor *self,
                        TextMark   *mark,
                        int         amount)
{
    // TODO: Why does this overstep by 1 index?

    TextRun *iter;
    int amount_moved;
    gboolean first;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (amount > 0);

    iter = mark->parent;
    amount_moved = 0;
    first = TRUE;

    // Handle first run
    if (mark->index + amount <= text_run_get_length (iter))
    {
        mark->index += amount;
        return;
    }

    amount_moved += (text_run_get_length (iter) - mark->index);
    iter = walk_until_next_run (TEXT_ITEM (iter));

    while (amount_moved < amount)
    {
        int iter_length;

        // Could be NULL (e.g. end of document)
        if (!TEXT_IS_ITEM (iter))
            return;

        iter_length = text_run_get_length (iter);

        // Run is entirely contained within amount to move
        if (amount_moved + iter_length < amount)
        {
            iter = walk_until_next_run (TEXT_ITEM (iter));
            amount_moved += iter_length;
            continue;
        }

        break;
    }

    mark->index = amount - amount_moved;
    mark->parent = iter;
}

void
text_editor_move_first (TextEditor *self,
                        TextMark   *mark)
{
    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    mark->parent = walk_until_next_run (TEXT_ITEM (self->document->frame));
    mark->index = 0;
}

void
text_editor_move_last (TextEditor *self,
                       TextMark   *mark)
{
    char *text;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    mark->parent = walk_until_previous_run (TEXT_ITEM (self->document->frame));
    mark->index = 0;

    if (mark->parent)
    {
        g_object_get (mark->parent, "text", &text, NULL);
        mark->index = strlen (text);
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
_delete_run (TextEditor *self,
             TextRun    *run)
{
    TextNode *parent;

    parent = text_node_get_parent (TEXT_NODE (run));

    if (TEXT_IS_PARAGRAPH (parent) &&
        text_node_get_num_children (parent) == 1)
    {
        // Delete paragraph if we are the last run
        text_node_delete (&parent);

        // Ensure there is a paragraph remaining
        _ensure_paragraph (self);

        return;
    }

    // Delete run
    parent = TEXT_NODE (run);
    text_node_delete (&parent);

    return;
}

static void
_erase_text (TextRun *run,
             int      index,
             int      length)
{
    GString *modified;
    char *text;

    g_object_get (run, "text", &text, NULL);

    // Assumes index and length are within range
    modified = g_string_new (text);
    modified = g_string_erase (modified, index, length);
    g_object_set (run, "text", modified->str, NULL);
    g_string_free (modified, TRUE);
}

void
text_editor_delete (TextEditor *self,
                    TextMark   *start,
                    int         length)
{
    // TODO: Rework to use an Iter instead of a Mark
    // TODO: Update all marks associated with a document

    TextMark *cursor;
    TextRun *run;
    TextRun *iter;
    int run_length;
    int cur_deleted;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (start != NULL);

    if (length < 0)
    {
        text_editor_move_left (self, self->document->cursor, -length);
        text_editor_delete (self, start, -length);
        return;
    }

    run = start->parent;
    run_length = text_run_get_length (run);
    cursor = self->document->cursor;

    // Case 1: Deleting a single run
    if (start->index == 0 &&
        length == run_length)
    {
        cursor->parent = walk_until_previous_run (TEXT_ITEM (cursor->parent));
        _delete_run (self, run);
        return;
    }

    // Case 2: Deletion is contained within current run
    if (start->index + length <= text_run_get_length (run))
    {
        _erase_text (run, start->index, length);
        return;
    }

    // Case 3: Deletion spans multiple runs
    cur_deleted = 0;
    iter = run;
    gboolean first = TRUE;

    while (cur_deleted < length)
    {
        int delta_length;

        delta_length = text_run_get_length (iter);

        // Handle first element
        if (first)
        {
            _erase_text (iter, start->index, delta_length - start->index);
            iter = walk_until_next_run (TEXT_ITEM (iter));
            first = FALSE;
            continue;
        }

        // Check if the run is entirely contained within the deletion
        if (cur_deleted + delta_length < length)
        {
            TextRun *next;

            next = walk_until_next_run (TEXT_ITEM (iter));
            _delete_run (self, iter);
            cur_deleted += delta_length;
            iter = next;
            continue;
        }

        // Handle last element
        _erase_text (iter, 0, length - cur_deleted);

        return;
    }
}

int
_length_between_marks (TextMark *start,
                       TextMark *end)
{
    TextRun *iter;
    int length = 0;

    // TODO: Find a way to determine whether start is before end?

    iter = start->parent;
    while ((iter = walk_until_next_run (TEXT_ITEM (iter))) != NULL)
    {
        if (iter == end->parent)
        {
            length += end->index;
            break;
        }

        length += text_run_get_length (iter);
    }

    return length;
}

void
text_editor_replace (TextEditor *self,
                     TextMark   *start,
                     TextMark   *end,
                     gchar      *text)
{
    // TODO: Find a way to determine whether start is before end?

    int length;
    length = _length_between_marks (start, end);
    text_editor_delete (self, start, length);
    text_editor_insert (self, start, text);
}

void
text_editor_insert (TextEditor *self,
                    TextMark   *start,
                    gchar      *str)
{
    // Encapsulates insertion inside an editor module/object.
    // This should accept user input in the form of Operational
    // Transformation commands. This will aid with undo/redo.

    char *text;
    GString *modified;
    TextRun *run;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (TEXT_IS_RUN (start->parent));

    run = start->parent;

    // TODO: Replace with hybrid tree/piece-table structure?
    // Textual data is stored in buffers and indexed by the tree
    g_object_get (run, "text", &text, NULL);
    modified = g_string_new (text);
    modified = g_string_insert (modified, start->index, str);
    g_object_set (run, "text", modified->str, NULL);
    g_string_free (modified, TRUE);

    // TODO: Update all marks

    // Move cursor left/right by amount changed
    // This should be handled by an auxiliary anchor object which
    // remains fixed at a given point in the text no matter how
    // the text changes (i.e. a cursor).
    text_editor_move_right (self, self->document->cursor, strlen (str));
}

static void
text_editor_init (TextEditor *self)
{
}
