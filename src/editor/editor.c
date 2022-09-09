/* editor.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "editor.h"

#include "../model/paragraph.h"
#include "../model/opaque.h"

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

static int
_get_offset (TextParagraph *paragraph,
             int            byte_index)
{
    // NOTE: byte_index must be within bounds!
    const char *text;
    text = text_paragraph_get_text (paragraph);
    return (int) g_utf8_pointer_to_offset (text, text + byte_index);
}

static TextItem *
go_up (TextItem *item,
       gboolean  forwards)
{
    TextNode *parent;
    TextNode *sibling;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    parent = text_node_get_parent (TEXT_NODE (item));

    if (parent && TEXT_IS_ITEM (parent))
    {
        sibling = forwards
            ? text_node_get_next (parent)
            : text_node_get_previous (parent);

        if (sibling && TEXT_IS_ITEM (sibling))
            return TEXT_ITEM (sibling);
        else
            return go_up (TEXT_ITEM (parent), forwards);
    }

    return NULL;
}

static TextFragment *
walk_until_previous_fragment (TextItem *item)
{
    TextNode *child;
    TextNode *sibling;
    TextItem *parent;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    child = text_node_get_last_child (TEXT_NODE (item));

    if (child && TEXT_IS_ITEM (child)) {
        if (TEXT_IS_FRAGMENT (child)) {
            return TEXT_FRAGMENT (child);
        }

        return walk_until_previous_fragment(TEXT_ITEM(child));
    }

    sibling = text_node_get_previous (TEXT_NODE (item));

    if (sibling && TEXT_IS_ITEM (sibling)) {
        if (TEXT_IS_FRAGMENT (sibling)) {
            return TEXT_FRAGMENT (sibling);
        }

        return walk_until_previous_fragment(TEXT_ITEM(sibling));
    }

    parent = go_up (item, FALSE);

    if (parent) {
        if (TEXT_IS_FRAGMENT (parent)) {
            return TEXT_FRAGMENT (parent);
        }

        return walk_until_previous_fragment(parent);
    }

    return NULL;
}

static TextFragment *
walk_until_next_fragment (TextItem *item)
{
    TextNode *child;
    TextNode *sibling;
    TextItem *parent;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    child = text_node_get_first_child (TEXT_NODE (item));

    if (child && TEXT_IS_ITEM (child)) {
        if (TEXT_IS_FRAGMENT (child)) {
            return TEXT_FRAGMENT (child);
        }

        return walk_until_next_fragment(TEXT_ITEM(child));
    }

    sibling = text_node_get_next (TEXT_NODE (item));

    if (sibling && TEXT_IS_ITEM (sibling)) {
        if (TEXT_IS_FRAGMENT (sibling)) {
            return TEXT_FRAGMENT (sibling);
        }

        return walk_until_next_fragment(TEXT_ITEM(sibling));
    }

    parent = go_up (item, TRUE);

    if (parent) {
        if (TEXT_IS_FRAGMENT (parent)) {
            return TEXT_FRAGMENT (parent);
        }

        return walk_until_next_fragment(parent);
    }

    return NULL;
}

static TextParagraph *
walk_until_previous_paragraph (TextItem *item)
{
    TextNode *child;
    TextNode *sibling;
    TextItem *parent;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    child = text_node_get_last_child (TEXT_NODE (item));

    if (child && TEXT_IS_ITEM (child)) {
        if (TEXT_IS_PARAGRAPH (child)) {
            return TEXT_PARAGRAPH (child);
        }

        return walk_until_previous_paragraph (TEXT_ITEM (child));
    }

    sibling = text_node_get_previous (TEXT_NODE (item));

    if (sibling && TEXT_IS_ITEM (sibling)) {
        if (TEXT_IS_PARAGRAPH (sibling)) {
            return TEXT_PARAGRAPH (sibling);
        }

        return walk_until_previous_paragraph (TEXT_ITEM (sibling));
    }

    parent = go_up (item, FALSE);

    if (parent) {
        if (TEXT_IS_PARAGRAPH (parent)) {
            return TEXT_PARAGRAPH (parent);
        }

        return walk_until_previous_paragraph (parent);
    }

    return NULL;
}

static TextParagraph *
walk_until_next_paragraph (TextItem *item)
{
    TextNode *child;
    TextNode *sibling;
    TextItem *parent;

    g_return_val_if_fail (TEXT_IS_ITEM (item), NULL);

    child = text_node_get_first_child (TEXT_NODE (item));

    if (child && TEXT_IS_ITEM (child)) {
        if (TEXT_IS_PARAGRAPH (child)) {
            return TEXT_PARAGRAPH (child);
        }

        return walk_until_next_paragraph (TEXT_ITEM (child));
    }

    sibling = text_node_get_next (TEXT_NODE (item));

    if (sibling && TEXT_IS_ITEM (sibling)) {
        if (TEXT_IS_PARAGRAPH (sibling)) {
            return TEXT_PARAGRAPH (sibling);
        }

        return walk_until_next_paragraph (TEXT_ITEM (sibling));
    }

    parent = go_up (item, TRUE);

    if (parent) {
        if (TEXT_IS_PARAGRAPH (parent)) {
            return TEXT_PARAGRAPH (parent);
        }

        return walk_until_next_paragraph (parent);
    }

    return NULL;
}

TextFragment *
text_editor_get_item_at_mark (TextEditor *self,
                              TextMark   *mark)
{
    g_return_val_if_fail (TEXT_IS_EDITOR (self), NULL);
    g_return_val_if_fail (mark != NULL, NULL);

    return text_paragraph_get_item_at_index (mark->paragraph,
                                             mark->index,
                                             NULL);
}

static void
move_n_chars_to_byte_index (const gchar *text,
                            int         *byte_index,
                            int          amount)
{
    // NOTE: Not bounds checked! Make sure to check the
    // string length with g_utf8_strlen() or similar before
    // calling.

    const char *new_ptr;

    g_return_if_fail (text != NULL);
    g_return_if_fail (byte_index != NULL);

    // Go 'amount' characters forwards and find address
    new_ptr = g_utf8_offset_to_pointer (text + *byte_index, amount);

    // Calculate new byte offset
    *byte_index = (int)(new_ptr - text);
}

/**
 * _try_move_mark_left:
 *
 * Attempt to move the mark left by the given amount. If the mark
 * cannot be moved the full amount, return the remaining distance.
 *
 * @self: `TextEditor` instance
 * @mark: the `TextMark` to move
 * @amount: Amount to move by (in characters)
 *
 * Returns: The distance remaining or -1 if an error occurred.
 */
int
_try_move_mark_left (TextMark   *mark,
                     int         amount)
{
    TextParagraph *iter;
    const gchar *text;
    int amount_moved;
    int mark_char_offset;

    g_return_val_if_fail (mark != NULL, -1);
    g_return_val_if_fail (amount >= 0, -1);

    if (amount == 0)
        return 0;

    iter = mark->paragraph;
    amount_moved = 0;

    text = text_paragraph_get_text (iter);

    // Calculate how many characters into the paragraph the mark is
    mark_char_offset = (int) g_utf8_strlen (text, mark->index);

    // Simple case: The movement is contained entirely
    // within the current paragraph.
    if (mark_char_offset - amount >= 0)
    {
        move_n_chars_to_byte_index (text, &mark->index, -amount);
        return 0;
    }

    // Crossing one or more paragraphs
    amount_moved += mark_char_offset;

    while (amount_moved < amount)
    {
        int num_indices;

        // Go to previous paragraph
        iter = walk_until_previous_paragraph (TEXT_ITEM (iter));

        // Check if NULL (e.g. start of document)
        if (!TEXT_IS_ITEM (iter))
            break;

        // Num of indices is the paragraph length
        // plus the final index
        num_indices = text_paragraph_get_length (iter) + 1;

        // Paragraph is entirely contained within amount to move
        if (amount_moved + num_indices < amount)
        {
            // We are now at index 0 of the previous paragraph -> repeat
            amount_moved += num_indices;
            continue;
        }

        // Move partially through the paragraph by
        // the amount to move remaining
        // TODO: Check if this actually works
        text = text_paragraph_get_text (iter);
        mark->index = (int) (g_utf8_offset_to_pointer (text, num_indices - (amount - amount_moved)) - text);
        mark->paragraph = iter;
        return 0;
    }

    // Reached start of document
    text_editor_move_mark_first (mark);
    return amount - amount_moved;
}

/**
 * text_editor_move_mark_left:
 *
 * Move the mark left by the given amount. If the mark
 * cannot be moved the full amount, it will stop at the
 * left-most valid index.
 *
 * @self: `TextEditor` instance
 * @mark: the `TextMark` to move
 * @amount: Amount to move by
 */
void
text_editor_move_mark_left (TextMark   *mark,
                            int         amount)
{
    _try_move_mark_left (mark, amount);
}

/**
 * _try_move_mark_right:
 *
 * Attempt to move the mark right by the given amount. If the mark
 * cannot be moved the full amount, return the remaining distance.
 *
 * @self: `TextEditor` instance
 * @mark: the `TextMark` to move
 * @amount: Amount to move by
 *
 * Returns: The distance remaining or -1 if an error occurred.
 */
int
_try_move_mark_right (TextMark   *mark,
                      int         amount)
{
    TextParagraph *iter;
    gchar *text;
    int amount_moved;
    int last_index;
    int mark_char_offset;

    g_return_val_if_fail (mark != NULL, -1);
    g_return_val_if_fail (amount >= 0, -1);

    if (amount == 0)
        return 0;

    iter = mark->paragraph;
    amount_moved = 0;

    text = text_paragraph_get_text (iter);

    // Calculate how many characters into the paragraph the mark is
    mark_char_offset = (int) g_utf8_strlen (text, mark->index);

    // Simple case: The movement is contained entirely
    // within the current paragraph.
    last_index = text_paragraph_get_length (iter);
    if (mark_char_offset + amount <= last_index)
    {
        move_n_chars_to_byte_index (text, &mark->index, amount);
        return 0;
    }

    // Crossing one or more paragraphs
    amount_moved += (last_index - mark_char_offset);

    while (amount_moved < amount)
    {
        int num_indices;

        // Go to next paragraph
        iter = walk_until_next_paragraph (TEXT_ITEM (iter));

        // Check if NULL (end of document)
        if (!TEXT_IS_ITEM (iter))
            break;

        // Num of indices is the paragraph length
        // plus the final index
        num_indices = text_paragraph_get_length (iter) + 1;

        // Paragraph is entirely contained within amount to move
        if (amount_moved + num_indices < amount)
        {
            // We are now at the last index of the next paragraph -> repeat
            amount_moved += num_indices;
            continue;
        }

        // Move partially through the paragraph by
        // the amount to move remaining
        // TODO: Check if this actually works
        text = text_paragraph_get_text (iter);
        mark->index = (int) (g_utf8_offset_to_pointer (text, (amount - amount_moved) - 1) - text);
        mark->paragraph = iter;
        return 0;
    }

    // Reached end of document
    text_editor_move_mark_last (mark);
    return amount - amount_moved;
}

/**
 * text_editor_move_mark_right:
 *
 * Move the mark right by the given amount. If the mark
 * cannot be moved the full amount, it will stop at the
 * right-most valid index.
 *
 * @self: `TextEditor` instance
 * @mark: the `TextMark` to move
 * @amount: Amount to move by
 */
void
text_editor_move_mark_right (TextMark   *mark,
                             int         amount)
{
    _try_move_mark_right (mark, amount);
}

/**
 * text_editor_move_mark_first:
 *
 * Move the mark to the beginning of the document.
 *
 * @mark: The `TextMark` to move
 */
void
text_editor_move_mark_first (TextMark *mark)
{
    g_return_if_fail (mark != NULL);
    g_return_if_fail (TEXT_IS_DOCUMENT (mark->document));

    mark->paragraph = walk_until_next_paragraph (TEXT_ITEM (mark->document->frame));
    mark->index = 0;
}

/**
 * text_editor_move_mark_last:
 *
 * Move the mark to the end of the document.
 *
 * @mark: The `TextMark` to move
 */
void
text_editor_move_mark_last (TextMark *mark)
{
    g_return_if_fail (mark != NULL);
    g_return_if_fail (TEXT_IS_DOCUMENT (mark->document));

    mark->paragraph = walk_until_previous_paragraph (TEXT_ITEM (mark->document->frame));
    mark->index = 0;

    if (mark->paragraph)
    {
        mark->index = text_paragraph_get_size_bytes (mark->paragraph);
    }
}

static void
_ensure_paragraph (TextEditor *self)
{
    TextFrame *document_frame;
    TextParagraph *paragraph;

    document_frame = self->document->frame;

    // Add paragraph with run if none exists
    if (!walk_until_next_fragment(TEXT_ITEM(document_frame)))
    {
        paragraph = text_paragraph_new ();
        text_frame_append_block (document_frame, TEXT_BLOCK (paragraph));
        text_paragraph_append_fragment(paragraph, text_run_new(""));
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
        text_node_clear (&parent);

        // Ensure there is a paragraph remaining
        _ensure_paragraph (self);

        return;
    }

    // Delete run
    text_node_delete (TEXT_NODE (run));

    return;
}

static void
_erase_text (TextRun *run,
             int      index,
             int      num_chars,
             int     *bytes_deleted)
{
    GString *modified;
    const char *text;
    int length_bytes;

    text = text_fragment_get_text (TEXT_FRAGMENT (run));

    // Calculate length (in bytes) to erase
    length_bytes = (int) (g_utf8_offset_to_pointer (text + index, num_chars) - (text + index));

    // Assumes index and length are within range
    modified = g_string_new (text);
    modified = g_string_erase (modified, index, length_bytes);
    g_object_set (run, "text", modified->str, NULL);
    g_string_free (modified, TRUE);

    if (bytes_deleted)
        *bytes_deleted = length_bytes;
}

static void
_erase_content (TextFragment *item,
                int index,
                int num_chars,
                int *bytes_deleted)
{
    // Initialise to zero
    if (bytes_deleted)
        *bytes_deleted = 0;

    if (TEXT_IS_RUN (item))
    {
        _erase_text (TEXT_RUN (item), index, num_chars, bytes_deleted);
    }
    else if (TEXT_IS_OPAQUE (item))
    {
        TextParagraph *parent;

        if (bytes_deleted)
            *bytes_deleted = text_fragment_get_size_bytes (item);

        parent = TEXT_PARAGRAPH (text_node_get_parent (TEXT_NODE (item)));
        text_node_insert_child_before (TEXT_NODE (parent), TEXT_NODE (text_run_new ("")), TEXT_NODE (item));
        text_node_delete (TEXT_NODE (item));
    }
    else
    {
        g_print ("Cannot delete a non-opaque inline element");
    }
}

static void
_join_paragraphs (TextParagraph *start,
                  TextParagraph **end)
{
    TextNode *iter;

    // Check start and end are siblings
    // TODO: Support more complex joining?
    g_return_if_fail (text_node_get_next (TEXT_NODE (start)) == TEXT_NODE (*end));

    iter = text_node_get_first_child (TEXT_NODE (*end));

    while (iter != NULL)
    {
        TextNode *swap;

        swap = iter;
        g_assert (TEXT_IS_FRAGMENT (swap));

        iter = text_node_get_next (iter);

        // Perform swap
        text_node_unparent (TEXT_NODE (swap));
        text_node_append_child (TEXT_NODE (start), swap);
    }

    text_node_delete (TEXT_NODE (*end));
    *end = NULL;
}

/**
 * _delete_within_paragraph:
 *
 * @paragraph: Paragraph the deletion will be performed on
 * @start_index: Starting byte index of the deletion. Note
 * this is a byte index, not a unicode character offset
 * @deletion_length: The number of unicode characters to
 * be deleted.
 *
 * Returns: `TRUE` if the paragraph was deleted
 */
static gboolean
_delete_within_paragraph (TextParagraph *paragraph,
                          int            start_index,
                          int            deletion_length,
                          int           *bytes_deleted)
{
    TextFragment *start;
    int paragraph_length;
    int paragraph_size;
    int end_index;
    int start_run_offset;
    gboolean is_only;
    const char *text;

    if (deletion_length == 0)
        return FALSE;

    text = text_paragraph_get_text (paragraph);
    paragraph_length = text_paragraph_get_length (paragraph);
    paragraph_size = text_paragraph_get_size_bytes(paragraph);

    is_only = text_node_get_num_children (TEXT_NODE (paragraph)) == 1;

    // Check run immediately after the start_index
    // (as start_index may be the final index of a run)
    start = text_paragraph_get_item_at_index (paragraph, start_index + 1, &start_run_offset);
    end_index = (int) (g_utf8_offset_to_pointer (text + start_index, deletion_length) - text);

    g_return_val_if_fail (bytes_deleted != NULL, -1);
    g_assert (0 <= start_index && start_index <= paragraph_size);
    g_assert (0 <= end_index && end_index <= paragraph_size);
    g_assert (start_index <= end_index);

    // Case 1: The whole paragraph is to be deleted
    if (start_index == 0 && deletion_length == paragraph_length + 1)
    {
        text_node_delete (TEXT_NODE (paragraph));
        *bytes_deleted = 0;
        return TRUE;
    }

    // Case 2: The paragraph only contains one run
    if (is_only)
    {
        // The paragraph should not be deleted (handled
        // by above condition) so erase contents
        _erase_content (start, start_index, deletion_length, bytes_deleted);
        return FALSE;
    }

    // Case 3: The paragraph contains multiple runs
    {
        int run_length;
        int run_bytes_deleted;
        TextFragment *iter;
        int cur_deleted;
        int index_within_run;

        run_length = text_fragment_get_length (TEXT_FRAGMENT (start));
        index_within_run = start_index - start_run_offset;
        cur_deleted = 0;

        iter = start;

        *bytes_deleted = 0;

        // If only part of the first run should be erased, handle it here.
        if (index_within_run != 0 || deletion_length != run_length)
        {
            int to_delete;
            to_delete = MIN (deletion_length, run_length - _get_offset (paragraph, index_within_run));

            cur_deleted += to_delete;
            iter = walk_until_next_fragment(TEXT_ITEM(start));

            // Delete part of run
            _erase_content (start, index_within_run, to_delete, &run_bytes_deleted);
            *bytes_deleted += run_bytes_deleted;
        }

        // Iterate over the remaining runs
        while (cur_deleted < deletion_length)
        {
            g_assert (iter != NULL);

            run_bytes_deleted = text_fragment_get_size_bytes (TEXT_FRAGMENT (iter));
            run_length = text_fragment_get_length (TEXT_FRAGMENT (iter));

            // Check if the run is entirely contained within the deletion
            if (cur_deleted + run_length <= deletion_length)
            {
                TextFragment *next;

                next = walk_until_next_fragment(TEXT_ITEM(iter));
                text_node_delete (TEXT_NODE (iter));

                cur_deleted += run_length;
                *bytes_deleted += run_bytes_deleted;
                iter = next;

                continue;
            }

            // Handle last element
            _erase_content (iter, 0, deletion_length - cur_deleted, &run_bytes_deleted);
            *bytes_deleted += run_bytes_deleted;
            break;
        }
    }

    return FALSE;
}

/**
 * _distribute_mark:
 *
 * Distributes a mark to either a starting or ending
 * text position depending on the mark's gravity.
 *
 * @mark: The mark to recalculate
 * @start_para: The #TextParagraph to use if %TEXT_GRAVITY_LEFT
 * @start_index: The index to use if %TEXT_GRAVITY_LEFT
 * @end_para: The #TextParagraph to use if %TEXT_GRAVITY_RIGHT
 * @end_index: The index to use if %TEXT_GRAVITY_RIGHT
 */
static void
_distribute_mark (TextMark      *mark,
                  TextParagraph *start_para,
                  int            start_index,
                  TextParagraph *end_para,
                  int            end_index)
{
    if (mark->gravity == TEXT_GRAVITY_LEFT)
    {
        mark->paragraph = start_para;
        mark->index = start_index;
        return;
    }

    mark->paragraph = end_para;
    mark->index = end_index;
}

static void
_offset_mark (TextMark *mark,
              int       byte_offset)
{
    mark->index += byte_offset;
}

void
text_editor_delete_at_mark (TextEditor *self,
                            TextMark   *start,
                            int         length)
{
    TextParagraph *paragraph;
    int num_indices;
    int start_char_offset;
    const char *text;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (TEXT_IS_PARAGRAPH (start->paragraph));
    g_return_if_fail (start != NULL);

    paragraph = start->paragraph;

    if (length < 0)
    {
        int remaining;

        // Handles case where the cursor cannot be moved by the
        // full '-length' because the start of document is reached
        // TODO: Should this refer to cursor directly?
        remaining = _try_move_mark_left  (self->document->cursor, -length);
        text_editor_delete_at_mark (self, start, (-length - remaining));
        return;
    }

    text = text_paragraph_get_text (start->paragraph);

    // Calculate how many characters into the paragraph the mark is
    start_char_offset = (int) g_utf8_strlen (text, start->index);

    // Account for final index at the end of a paragraph
    num_indices = text_paragraph_get_length (start->paragraph) + 1;

    // Case 1: Deletion affects a single paragraph
    // Any portion of a paragraph up to the entire thing
    if (start_char_offset + length < num_indices)
    {
        int new_index;
        int bytes_deleted;
        TextParagraph *new_para;
        TextParagraph *prev;

        prev = walk_until_previous_paragraph (TEXT_ITEM (start->paragraph));
        new_para = start->paragraph;
        new_index = start->index;

        // True if paragraph was deleted
        if (_delete_within_paragraph (start->paragraph, start->index, length, &bytes_deleted))
        {
            if (prev)
            {
                new_para = prev;
                new_index = text_paragraph_get_size_bytes (prev);
            }
            else
            {
                _ensure_paragraph (self);
                new_para = walk_until_next_paragraph (TEXT_ITEM (self->document->frame));
                new_index = 0;
            }
        }

        // Adjust marks - we can ignore gravity as
        // marks will converge on the same point
        GSList *marks;
        marks = text_document_get_all_marks (self->document);

        for (GSList *mark_iter = marks;
             mark_iter != NULL;
             mark_iter = mark_iter->next)
        {
            TextMark *mark;

            mark = (TextMark *)mark_iter->data;

            // Marks within affected area
            if (mark->paragraph == start->paragraph &&
                start->index <= mark->index &&
                mark->index <= start->index + bytes_deleted)
            {
                _distribute_mark (mark, new_para, new_index,
                                  new_para, new_index);
            }

            // Marks after affected area
            else if (mark->paragraph == start->paragraph &&
                start->index + bytes_deleted < mark->index)
            {
                _offset_mark (mark, -bytes_deleted);
            }
        }

        g_slist_free (marks);

        return;
    }

    // Case 2: Deletion affects multiple paragraphs
    {
        GSList *dirty;

        TextParagraph *iter;
        int cur_deleted;
        int paragraph_length;
        int bytes_deleted;
        int to_delete;
        int remaining;

        iter = paragraph;
        cur_deleted = 0;
        dirty = NULL;

        paragraph_length = text_paragraph_get_length (iter);

        // Handle first paragraph
        // Erase part or all of the first paragraph but do not delete it
        to_delete = paragraph_length - start_char_offset;
        _delete_within_paragraph (iter, start->index, to_delete, &bytes_deleted);
        cur_deleted += to_delete;

        // As the deletion affects multiple paragraphs, we also
        // account for the paragraph boundary.
        cur_deleted += 1;
        iter = walk_until_next_paragraph (TEXT_ITEM (iter));

        // Loop over all paragraphs between start and end
        while (cur_deleted < length)
        {
            num_indices = text_paragraph_get_length (iter) + 1;

            // Entire paragraph is contained within deletion
            if (cur_deleted + num_indices <= length)
            {
                // Mark paragraph for deletion
                dirty = g_slist_append (dirty, iter);
                iter = walk_until_next_paragraph (TEXT_ITEM (iter));
                cur_deleted += num_indices;
                continue;
            }

            // Delete a portion of the end paragraph
            remaining = length - cur_deleted;
            _delete_within_paragraph (iter, 0, remaining, &bytes_deleted);
            break;
        }

        // Adjust marks - we can ignore gravity as
        // marks will converge on the same point
        GSList *marks;
        marks = text_document_get_all_marks (self->document);

        for (GSList *mark_iter = marks;
             mark_iter != NULL;
             mark_iter = mark_iter->next)
        {
            TextMark *mark;
            gboolean affected;

            mark = (TextMark *)mark_iter->data;
            affected = FALSE;

            // In starting paragraph
            if (mark->paragraph == paragraph &&
                start->index <= mark->index)
                affected = TRUE;

            // In dirty paragraphs
            else if (g_slist_find (dirty, mark->paragraph))
                affected = TRUE;

            // In ending paragraph, if exists
            else if (iter &&
                mark->paragraph == iter &&
                mark->index <= remaining)
                affected = TRUE;

            // Mark within affected area
            if (affected)
            {
                _distribute_mark (mark, paragraph, start->index,
                                  paragraph, start->index);
            }

            // Mark after affected area
            else if (iter &&
                mark->paragraph == iter &&
                remaining < mark->index)
            {
                int offset;

                // Will now index into the starting paragraph
                // as we join them (see below)
                mark->paragraph = start->paragraph;

                // TODO: Does this actually work?
                offset = text_paragraph_get_size_bytes (start->paragraph);
                _offset_mark (mark, offset - bytes_deleted);
            }
        }

        g_slist_free (marks);

        // Perform lazy deletion
        g_slist_free_full (dirty, (GDestroyNotify)text_node_delete);

        // Join start and end paragraphs
        if (iter != NULL)
            _join_paragraphs (start->paragraph, &iter);
    }
}

/**
 * _relate_nodes:
 *
 * Relate two nodes to each other to find the lowest common
 * ancestor. Additionally indicates which node comes first
 * in the tree when using a depth-first traversal.
 *
 * The ordering check may be omitted by passing %NULL to the
 * @in_order parameter, which may improve performance somewhat.
 *
 * @start: Starting node
 * @end: Ending node
 *
 * Returns: The lowest common ancestor of the two nodes
 *
 */
TextNode *
_relate_nodes (TextNode *start,
               TextNode *end,
               gboolean *in_order)
{
    GSList *ancestors;
    TextNode *iter;

    g_return_val_if_fail (start, NULL);
    g_return_val_if_fail (end, NULL);

    ancestors = NULL;
    iter = start;

    while ((iter = text_node_get_parent (iter)) != NULL)
        ancestors = g_slist_append (ancestors, iter);

    iter = end;

    while (iter != NULL)
    {
        TextNode *parent;

        parent = text_node_get_parent (iter);

        if (g_slist_find (ancestors, parent))
        {
            TextNode *child;

            // Return the lowest common ancestor
            if (in_order == NULL)
                return parent;

            // Otherwise find which node comes first
            for (child = text_node_get_first_child (parent);
                 child != NULL;
                 child = text_node_get_next (child))
            {
                if (child == start) {
                    *in_order = TRUE;
                    return parent;
                }

                if (child == end) {
                    *in_order = FALSE;
                    return parent;
                }
            }

            // Should not be reached
            g_assert ("Tree structure is malformed");
        }

        iter = parent;
    }

    return NULL;
}

int
_length_between_marks (TextMark *start,
                       TextMark *end)
{
    TextParagraph *iter;
    const char *text;
    int length;

    g_return_val_if_fail (start != NULL, 0);
    g_return_val_if_fail (end != NULL, 0);

    if (start->paragraph == end->paragraph)
    {
        text = text_paragraph_get_text (start->paragraph);
        return (int) (g_utf8_pointer_to_offset (text + start->index, text + end->index));
    }

    iter = start->paragraph;
    length = text_paragraph_get_length (iter) + 1 - start->index;

    while ((iter = walk_until_next_paragraph (TEXT_ITEM (iter))) != NULL)
    {
        if (iter == end->paragraph)
        {
            text = text_paragraph_get_text (end->paragraph);
            length += (int) g_utf8_strlen (text, end->index);
            break;
        }

        length += text_paragraph_get_length (iter) + 1;
    }

    return length;
}

void
_ensure_ordered (TextMark **start,
                 TextMark **end)
{
    gboolean in_order;

    g_return_if_fail (start != NULL);
    g_return_if_fail (end != NULL);
    g_return_if_fail (*start != NULL);
    g_return_if_fail (*end != NULL);

    if ((*start)->paragraph == (*end)->paragraph)
    {
        in_order = (*start)->index < (*end)->index;
    }
    else
    {
        _relate_nodes (TEXT_NODE ((*start)->paragraph),
                       TEXT_NODE ((*end)->paragraph),
                       &in_order);
    }

    // Swap if in wrong order
    if (!in_order)
    {
        TextMark *temp;
        temp = *end;
        *end = *start;
        *start = temp;
    }
}

// TODO: All usages must use offsets
void
split_run_at_offset (TextRun  *run,
                     TextRun **new,
                     int offset)
{
    char *first_text;
    char *second_text;

    g_return_if_fail (TEXT_IS_RUN (run));
    g_return_if_fail (new != NULL);

    g_object_get (run, "text", &first_text, NULL);

    second_text = g_utf8_substring (first_text, offset, -1);
    first_text = g_utf8_substring (first_text, 0, offset);

    g_object_set (run, "text", first_text, NULL);
    *new = text_run_new (second_text);

    // Copy formatting
    text_run_set_style_bold (*new, text_run_get_style_bold (run));
    text_run_set_style_italic (*new, text_run_get_style_italic (run));
    text_run_set_style_underline (*new, text_run_get_style_underline (run));
}

// TODO: All usages must use offsets
void
split_run_in_place (TextRun *run,
                    TextRun **new,
                    int offset)
{
    TextParagraph *parent;

    parent = TEXT_PARAGRAPH (text_node_get_parent (TEXT_NODE (run)));
    split_run_at_offset (run, new, offset);
    text_node_insert_child_after (TEXT_NODE (parent),
                                  TEXT_NODE (*new),
                                  TEXT_NODE (run));
}

void
text_editor_split_at_mark (TextEditor *self,
                           TextMark   *split)

{
    TextParagraph *new;
    TextParagraph *current;
    TextNode *parent;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (TEXT_IS_PARAGRAPH (split->paragraph));

    current = split->paragraph;

    // Case 1: Split is happening on the last index
    if (split->index == text_paragraph_get_size_bytes (current))
    {
        // Append a new paragraph with empty run
        new = text_paragraph_new ();
        text_paragraph_append_fragment(new, TEXT_FRAGMENT (text_run_new("")));

        parent = text_node_get_parent (TEXT_NODE (current));
        text_node_insert_child_after (parent, TEXT_NODE (new), TEXT_NODE (current));
    }

    // Case 2: Split happens mid-paragraph
    else
    {
        TextFragment *start;
        TextNode *iter;
        int index_within_run;

        new = text_paragraph_new ();
        start = text_paragraph_get_item_at_index (current, split->index, &index_within_run);

        iter = TEXT_NODE (start);

        // Split first run if index is not at beginning
        if (split->index != index_within_run)
        {
            int split_index_within_run;
            TextRun *new_run;

            // TODO: Make supported
            if (!TEXT_IS_RUN (start))
            {
                g_print ("Unsupported!\n");
            }
            else
            {
                split_index_within_run = split->index - index_within_run;
                split_run_at_offset (TEXT_RUN (start), &new_run, _get_offset (split->paragraph, split_index_within_run));
                text_paragraph_append_fragment(new, TEXT_FRAGMENT (new_run));
            }

            // Move to next run
            iter = text_node_get_next (iter);
        }

        // Iterate over remaining runs and move them to the new paragraph
        while (iter != NULL)
        {
            TextNode *next;

            g_assert (TEXT_IS_FRAGMENT (iter));

            next = text_node_get_next (iter);

            text_node_unparent (iter);
            text_paragraph_append_fragment(new, TEXT_FRAGMENT(iter));

            iter = next;
        }

        // Ensure the original paragraph has at least one run (all runs may be
        // moved when the split index is at the start of the paragraph)
        if (text_node_get_num_children (TEXT_NODE (current)) == 0)
        {
            // Add empty run
            text_paragraph_append_fragment(current, TEXT_FRAGMENT (text_run_new("")));
        }

        // Append paragraph to document tree
        parent = text_node_get_parent (TEXT_NODE (current));
        text_node_insert_child_after (parent, TEXT_NODE (new), TEXT_NODE (current));
    }

    // Adjust marks according to gravity
    GSList *marks;
    marks = text_document_get_all_marks (self->document);

    for (GSList *mark_iter = marks;
         mark_iter != NULL;
         mark_iter = mark_iter->next)
    {
        TextMark *mark;

        mark = (TextMark *)mark_iter->data;

        // Mark is on split point
        if (mark->paragraph == current &&
            mark->index == split->index)
        {
            _distribute_mark (mark, current, split->index, new, 0);
            continue;
        }

        // Mark is after split point
        if (mark->paragraph == current &&
            mark->index > split->index)
        {
            mark->paragraph = new;
            _offset_mark (mark, -split->index);
        }
    }

    g_slist_free (marks);
}

void
text_editor_replace_at_mark (TextEditor *self,
                             TextMark   *start,
                             TextMark   *end,
                             gchar      *text)
{
    int length;

    g_return_if_fail (TEXT_IS_PARAGRAPH (start->paragraph));
    g_return_if_fail (TEXT_IS_PARAGRAPH (end->paragraph));

    _ensure_ordered (&start, &end);

    length = _length_between_marks (start, end);
    text_editor_delete_at_mark (self, start, length);
    text_editor_insert_text_at_mark(self, start, text);
}

void
text_editor_insert_text_at_mark (TextEditor *self,
                                 TextMark   *start,
                                 gchar      *str)
{
    // Encapsulates insertion inside an editor module/object.
    // This should accept user input in the form of Operational
    // Transformation commands. This will aid with undo/redo.

    GSList *marks;
    char *text;
    GString *modified;
    TextFragment *item;
    TextRun *run;
    int run_start_index;
    int index_within_run;
    int length;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (TEXT_IS_PARAGRAPH (start->paragraph));

    item = text_paragraph_get_item_at_index (start->paragraph, start->index, &run_start_index);

    index_within_run = start->index - run_start_index;

    if (!TEXT_IS_RUN (item))
    {
        if (index_within_run == 0)
        {
            run = text_run_new ("");
            text_node_insert_child_before (TEXT_NODE (start->paragraph), TEXT_NODE (run), TEXT_NODE (item));
        }
        else if (index_within_run == text_fragment_get_size_bytes(item))
        {
            run = text_run_new ("");
            text_node_insert_child_after (TEXT_NODE (start->paragraph), TEXT_NODE (run), TEXT_NODE (item));
            index_within_run = 0;
        }
        else
        {
            g_print ("Not supported: Inserting into non-text run\n");
            return;
        }
    }
    else
    {
        run = TEXT_RUN (item);
    }

    // TODO: Replace with hybrid tree/piece-table structure?
    // Textual data is stored in buffers and indexed by the tree
    g_object_get (run, "text", &text, NULL);
    modified = g_string_new (text);
    modified = g_string_insert (modified, index_within_run, str);
    g_object_set (run, "text", modified->str, NULL);
    g_string_free (modified, TRUE);

    length = (int) strlen (str);

    // Adjust marks according to gravity
    marks = text_document_get_all_marks (self->document);

    for (GSList *mark_iter = marks;
         mark_iter != NULL;
         mark_iter = mark_iter->next)
    {
        TextMark *mark;

        mark = (TextMark *)mark_iter->data;

        // Mark is on insertion point
        if (mark->paragraph == start->paragraph &&
            mark->index == start->index)
        {
            _distribute_mark (mark,
                              start->paragraph, start->index,
                              start->paragraph, start->index + length);
            continue;
        }

        // Mark is after insertion point
        if (mark->paragraph == start->paragraph &&
            mark->index > start->index)
        {
            _offset_mark (mark, length);
        }
    }

    g_slist_free (marks);
}

void
text_editor_insert_fragment_at_mark (TextEditor   *self,
                                     TextMark     *start,
                                     TextFragment *fragment)
{
    // Encapsulates insertion inside an editor module/object.
    // This should accept user input in the form of Operational
    // Transformation commands. This will aid with undo/redo.

    GSList *marks;
    TextFragment *item;
    int run_start_index;
    int index_within_run;
    int size;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (TEXT_IS_PARAGRAPH (start->paragraph));

    item = text_paragraph_get_item_at_index (start->paragraph, start->index, &run_start_index);

    index_within_run = start->index - run_start_index;

    if (index_within_run == 0)
    {
        text_node_insert_child_before (TEXT_NODE (start->paragraph), TEXT_NODE (fragment), TEXT_NODE (item));
    }
    else if (index_within_run == text_fragment_get_size_bytes(item))
    {
        text_node_insert_child_after (TEXT_NODE (start->paragraph), TEXT_NODE (fragment), TEXT_NODE (item));
    }
    else if (TEXT_IS_RUN (item))
    {
        TextRun *new_run;
        const char *text;
        int offset_within_run;

        text = text_fragment_get_text (TEXT_FRAGMENT (item));
        offset_within_run = (int) g_utf8_pointer_to_offset (text, text + index_within_run);
        split_run_in_place (TEXT_RUN (item), &new_run, offset_within_run);
        text_node_insert_child_before (TEXT_NODE (start->paragraph), TEXT_NODE (fragment), TEXT_NODE (new_run));
    }
    else
    {
        g_print ("Cannot split opaque inline element!\n");
        return;
    }

    size = text_fragment_get_size_bytes (fragment);

    // Adjust marks according to gravity
    marks = text_document_get_all_marks (self->document);

    for (GSList *mark_iter = marks;
         mark_iter != NULL;
         mark_iter = mark_iter->next)
    {
        TextMark *mark;

        mark = (TextMark *)mark_iter->data;

        // Mark is on insertion point
        if (mark->paragraph == start->paragraph &&
            mark->index == start->index)
        {
            _distribute_mark (mark,
                              start->paragraph, start->index,
                              start->paragraph, start->index + size);
            continue;
        }

        // Mark is after insertion point
        if (mark->paragraph == start->paragraph &&
            mark->index > start->index)
        {
            _offset_mark (mark, size);
        }
    }

    g_slist_free (marks);
}

// TODO: Decouple format from run when we introduce the stylesheet
typedef enum
{
    FORMAT_BOLD,
    FORMAT_ITALIC,
    FORMAT_UNDERLINE
} Format;

static void
set_run_format (TextRun *run,
                Format   format,
                gboolean in_use)
{
    switch (format)
    {
        case FORMAT_BOLD:
            text_run_set_style_bold (run, in_use);
            break;
        case FORMAT_ITALIC:
            text_run_set_style_italic (run, in_use);
            break;
        case FORMAT_UNDERLINE:
            text_run_set_style_underline (run, in_use);
            break;
    }
}

static void
text_editor_apply_format (TextEditor *self,
                          TextMark   *start,
                          TextMark   *end,
                          Format      format,
                          gboolean    in_use)
{
    TextFragment *iter;
    TextFragment *last;
    int start_run_index;
    int end_run_index;

    _ensure_ordered (&start, &end);

    iter = text_paragraph_get_item_at_index (start->paragraph, start->index, &start_run_index);
    last = text_paragraph_get_item_at_index (end->paragraph, end->index, &end_run_index);

    // Check if start and end indices are in the same run
    if (iter == last)
    {
        TextFragment *first_split;
        TextFragment *second_split;
        int start_index_offset;
        int end_index_offset;

        g_assert (start_run_index == end_run_index);

        start_index_offset = start->index - start_run_index;
        end_index_offset = end->index - end_run_index;

        // Split first run
        split_run_in_place (iter, &first_split, start_index_offset);

        // Calculate offset into new run and split again
        end_index_offset -= start_index_offset;
        split_run_in_place (first_split, &second_split, end_index_offset);

        // Apply format to middle run
        set_run_format (first_split, format, in_use);
        return;
    }

    // Check if we need to split the first run
    if (start->index - start_run_index != 0)
    {
        TextFragment *new_run;
        split_run_in_place (iter, &new_run, start->index - start_run_index);

        // Apply format to new run
        set_run_format (new_run, format, in_use);
        iter = new_run;
    }

    // Check if we need to split the last run
    if (end->index - end_run_index != 0)
    {
        TextRun *new_run;
        split_run_in_place (last, &new_run, end->index - end_run_index);

        // Apply format to old run
        set_run_format (last, format, in_use);
    }

    while (iter != NULL)
    {
        if (iter == last)
            break;

        set_run_format (iter, format, in_use);

        iter = walk_until_next_fragment(TEXT_ITEM(iter));
    }
}

void
text_editor_apply_format_bold (TextEditor *self,
                               TextMark   *start,
                               TextMark   *end,
                               gboolean    is_bold)
{
    text_editor_apply_format (self, start, end, FORMAT_BOLD, is_bold);
}

void
text_editor_apply_format_italic (TextEditor *self,
                                 TextMark   *start,
                                 TextMark   *end,
                                 gboolean    is_italic)
{
    text_editor_apply_format (self, start, end, FORMAT_ITALIC, is_italic);
}

void
text_editor_apply_format_underline (TextEditor *self,
                                    TextMark   *start,
                                    TextMark   *end,
                                    gboolean    is_underline)
{
    text_editor_apply_format (self, start, end, FORMAT_UNDERLINE, is_underline);
}

gboolean
text_editor_get_format_bold_at_mark (TextEditor *self,
                                     TextMark   *mark)
{
    TextFragment *run;

    run = text_editor_get_item_at_mark (self, mark);
    if (TEXT_IS_RUN (run))
        return text_run_get_style_bold (TEXT_RUN (run));

    return FALSE;
}

gboolean
text_editor_get_format_italic_at_mark (TextEditor *self,
                                       TextMark   *mark)
{
    TextFragment *run;

    run = text_editor_get_item_at_mark (self, mark);
    if (TEXT_IS_RUN (run))
        return text_run_get_style_italic (TEXT_RUN (run));

    return FALSE;
}

gboolean
text_editor_get_format_underline_at_mark (TextEditor *self,
                                          TextMark   *mark)
{
    TextFragment *run;

    run = text_editor_get_item_at_mark (self, mark);
    if (TEXT_IS_RUN (run))
        return text_run_get_style_underline (TEXT_RUN (run));

    return FALSE;
}

TextMark *
_get_mark (TextEditor         *self,
           TextEditorMarkType  type)
{
    g_return_val_if_fail (TEXT_IS_EDITOR (self), NULL);
    g_return_val_if_fail (TEXT_IS_DOCUMENT (self->document), NULL);

    return (type == TEXT_EDITOR_CURSOR)
        ? self->document->cursor
        : self->document->selection;
}

TextParagraph *
text_editor_next_paragraph (TextParagraph *paragraph)
{
    return walk_until_next_paragraph (TEXT_ITEM (paragraph));
}

TextParagraph *
text_editor_previous_paragraph (TextParagraph *paragraph)
{
    return walk_until_previous_paragraph (TEXT_ITEM (paragraph));
}

void
text_editor_sort_marks (TextMark   *mark1,
                        TextMark   *mark2,
                        TextMark **first,
                        TextMark **last)
{
    g_return_if_fail (mark1 != NULL);
    g_return_if_fail (mark2 != NULL);

    _ensure_ordered (&mark1,&mark2);

    if (first)
        *first = mark1;
    if (last)
        *last = mark2;
}

void
text_editor_move_first (TextEditor         *self,
                        TextEditorMarkType  type)
{
    text_editor_move_mark_first (_get_mark (self, type));
}

void
text_editor_move_last (TextEditor         *self,
                       TextEditorMarkType  type)
{
    text_editor_move_mark_last (_get_mark (self, type));
}

void
text_editor_move_right (TextEditor         *self,
                        TextEditorMarkType  type,
                        int                 amount)
{
    text_editor_move_mark_right (_get_mark (self, type), amount);
}

void
text_editor_move_left (TextEditor         *self,
                       TextEditorMarkType  type,
                       int                 amount)
{
    text_editor_move_mark_left (_get_mark (self, type), amount);
}

void
text_editor_insert_text (TextEditor         *self,
                         TextEditorMarkType  type,
                         gchar              *str)
{
    text_editor_insert_text_at_mark (self, _get_mark(self, type), str);
}

void
text_editor_insert_fragment (TextEditor         *self,
                             TextEditorMarkType  type,
                             TextFragment         *fragment)
{
    text_editor_insert_fragment_at_mark (self, _get_mark(self, type), fragment);
}

void
text_editor_delete (TextEditor         *self,
                    TextEditorMarkType  type,
                    int                 length)
{
    text_editor_delete_at_mark (self, _get_mark (self, type), length);
}

void
text_editor_replace (TextEditor         *self,
                     TextEditorMarkType  start_type,
                     TextEditorMarkType  end_type,
                     gchar              *text)
{
    text_editor_replace_at_mark (self, _get_mark (self, start_type), _get_mark (self, end_type), text);
}

void
text_editor_split (TextEditor         *self,
                   TextEditorMarkType  type)
{
    text_editor_split_at_mark (self, _get_mark (self, type));
}

TextFragment *
text_editor_get_item (TextEditor         *self,
                      TextEditorMarkType  type)
{
    return text_editor_get_item_at_mark (self, _get_mark (self, type));
}

gchar *
text_editor_dump_plain_text (TextEditor *self)
{
    TextItem *iter;
    GString *string_builder;

    g_return_val_if_fail (TEXT_IS_EDITOR (self), NULL);
    g_return_val_if_fail (TEXT_IS_DOCUMENT (self->document), NULL);
    g_return_val_if_fail (TEXT_IS_FRAME (self->document->frame), NULL);

    string_builder = g_string_new (NULL);
    iter = TEXT_ITEM (self->document->frame);

    while ((iter = TEXT_ITEM (walk_until_next_paragraph (iter))) != NULL)
    {
        char *text;

        g_assert (TEXT_IS_ITEM (iter));
        text = text_paragraph_get_text (TEXT_PARAGRAPH (iter));
        g_string_append (string_builder, text);
        g_string_append (string_builder, "\n");
        g_free (text);
    }

    return g_string_free (string_builder, FALSE);
}

static void
text_editor_init (TextEditor *self)
{
}
