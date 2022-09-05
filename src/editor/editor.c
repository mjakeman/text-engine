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

TextInline *
text_editor_get_item_at_mark (TextEditor *self,
                              TextMark   *mark)
{
    g_return_val_if_fail (TEXT_IS_EDITOR (self), NULL);
    g_return_val_if_fail (mark != NULL, NULL);

    return text_paragraph_get_item_at_index (mark->paragraph,
                                             mark->index,
                                             NULL);
}

/**
 * _try_move_mark_left:
 *
 * Attempt to move the mark left by the given amount. If the mark
 * cannot be moved the full amount, return the remaining distance.
 *
 * @self: `TextEditor` instance
 * @mark: the `TextMark` to move
 * @amount: Amount to move by
 *
 * Returns: The distance remaining or -1 if an error occurred.
 */
int
_try_move_mark_left (TextEditor *self,
                     TextMark   *mark,
                     int         amount)
{
    TextParagraph *iter;
    int amount_moved;

    g_return_val_if_fail (TEXT_IS_EDITOR (self), -1);
    g_return_val_if_fail (TEXT_IS_DOCUMENT (self->document), -1);
    g_return_val_if_fail (amount >= 0, -1);
    g_return_val_if_fail (mark != NULL, -1);

    if (amount == 0)
        return 0;

    iter = mark->paragraph;
    amount_moved = 0;

    // Simple case: The movement is contained entirely
    // within the current paragraph.
    if (mark->index - amount >= 0)
    {
        mark->index -= amount;
        return 0;
    }

    // Crossing one or more paragraphs
    amount_moved += mark->index;

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
        mark->index = num_indices - (amount - amount_moved);
        mark->paragraph = iter;
        return 0;
    }

    // Reached start of document
    text_editor_move_mark_first (self, mark);
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
text_editor_move_mark_left (TextEditor *self,
                            TextMark   *mark,
                            int         amount)
{
    _try_move_mark_left (self, mark, amount);
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
_try_move_mark_right (TextEditor *self,
                      TextMark   *mark,
                      int         amount)
{
    TextParagraph *iter;
    int amount_moved;
    int last_index;

    g_return_val_if_fail (TEXT_IS_EDITOR (self), -1);
    g_return_val_if_fail (TEXT_IS_DOCUMENT (self->document), -1);
    g_return_val_if_fail (amount >= 0, -1);
    g_return_val_if_fail (mark != NULL, -1);

    if (amount == 0)
        return 0;

    iter = mark->paragraph;
    amount_moved = 0;

    // Simple case: The movement is contained entirely
    // within the current paragraph.
    last_index = text_paragraph_get_length (iter);
    if (mark->index + amount <= last_index)
    {
        mark->index += amount;
        return 0;
    }

    // Crossing one or more paragraphs
    amount_moved += (last_index - mark->index);

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
        mark->index = (amount - amount_moved) - 1;
        mark->paragraph = iter;
        return 0;
    }

    // Reached end of document
    text_editor_move_mark_last (self, mark);
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
text_editor_move_mark_right (TextEditor *self,
                             TextMark   *mark,
                             int         amount)
{
    _try_move_mark_right (self, mark, amount);
}

/**
 * text_editor_move_mark_first:
 *
 * Move the mark to the beginning of the document.
 *
 * @self: `TextEditor` instance
 * @mark: The `TextMark` to move
 */
void
text_editor_move_mark_first (TextEditor *self,
                             TextMark   *mark)
{
    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    mark->paragraph = walk_until_next_paragraph (TEXT_ITEM (self->document->frame));
    mark->index = 0;
}

/**
 * text_editor_move_mark_last:
 *
 * Move the mark to the end of the document.
 *
 * @self: `TextEditor` instance
 * @mark: The `TextMark` to move
 */
void
text_editor_move_mark_last (TextEditor *self,
                            TextMark   *mark)
{
    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));

    mark->paragraph = walk_until_previous_paragraph (TEXT_ITEM (self->document->frame));
    mark->index = 0;

    if (mark->paragraph)
    {
        mark->index = text_paragraph_get_length (mark->paragraph);
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
        text_paragraph_append_inline(paragraph, text_run_new(""));
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
        g_assert (TEXT_IS_INLINE (swap));

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
 * @start_index: Starting index of the deletion. It is the caller's
 * responsibility to ensure this is in-range.
 * @deletion_length: The number of characters to be erased.
 *
 * Returns: `TRUE` if the paragraph was deleted
 */
static gboolean
_delete_within_paragraph (TextParagraph *paragraph,
                          int            start_index,
                          int            deletion_length)
{
    TextRun *start;
    TextRun *end;
    int paragraph_length;
    int end_index;
    int start_run_offset;
    gboolean is_only;

    if (deletion_length == 0)
        return FALSE;

    paragraph_length = text_paragraph_get_length (paragraph);
    end_index = start_index + deletion_length;

    is_only = text_node_get_num_children (TEXT_NODE (paragraph)) == 1;

    // Check run immediately after the start_index
    // (as start_index may be the final index of a run)
    start = text_paragraph_get_item_at_index (paragraph, start_index + 1, &start_run_offset);
    end = text_paragraph_get_item_at_index (paragraph, end_index, NULL);

    g_assert (0 <= start_index && start_index <= paragraph_length);
    g_assert (0 <= end_index && end_index <= paragraph_length);
    g_assert (start_index <= end_index);

    // Case 1: The whole paragraph is to be deleted
    if (start_index == 0 && end_index == paragraph_length + 1)
    {
        text_node_delete (TEXT_NODE (paragraph));
        return TRUE;
    }

    // Case 2: The paragraph only contains one run
    if (is_only)
    {
        // The paragraph should not be deleted (handled
        // by above condition) so erase contents
        _erase_text (start, start_index, deletion_length);
        return FALSE;
    }

    // Case 3: The paragraph contains multiple runs
    {
        int run_length;
        TextRun *iter;
        int cur_deleted;
        int offset_within_run;

        run_length = text_inline_get_length (TEXT_INLINE (start));
        offset_within_run = start_index - start_run_offset;
        cur_deleted = 0;

        iter = start;

        // If only part of the first run should be erased, handle it here.
        if (offset_within_run != 0 || deletion_length != run_length)
        {
            int to_delete;
            to_delete = MIN (deletion_length, run_length - offset_within_run);

            cur_deleted += to_delete;
            iter = walk_until_next_run (TEXT_ITEM (start));

            // Delete part of run
            _erase_text (start, offset_within_run, to_delete);
        }

        // Iterate over the remaining runs
        while (cur_deleted < deletion_length)
        {
            g_assert (iter != NULL);

            run_length = text_inline_get_length (TEXT_INLINE (iter));

            // Check if the run is entirely contained within the deletion
            if (cur_deleted + run_length <= deletion_length)
            {
                TextRun *next;

                next = walk_until_next_run (TEXT_ITEM (iter));
                text_node_delete (TEXT_NODE (iter));

                cur_deleted += run_length;
                iter = next;

                continue;
            }

            // Handle last element
            _erase_text (iter, 0, deletion_length - cur_deleted);
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
              int       offset)
{
    mark->index += offset;
}

void
text_editor_delete_at_mark (TextEditor *self,
                            TextMark   *start,
                            int         length)
{
    TextParagraph *paragraph;
    int num_indices;

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
        remaining = _try_move_mark_left  (self, self->document->cursor, -length);
        text_editor_delete_at_mark (self, start, (-length - remaining));
        return;
    }

    // Account for final index at the end of a paragraph
    num_indices = text_paragraph_get_length (start->paragraph) + 1;

    // Case 1: Deletion affects a single paragraph
    // Any portion of a paragraph up to the entire thing
    if (start->index + length < num_indices)
    {
        int new_index;
        TextParagraph *new_para;
        TextParagraph *prev;

        prev = walk_until_previous_paragraph (TEXT_ITEM (start->paragraph));
        new_para = start->paragraph;
        new_index = start->index;

        // True if paragraph was deleted
        if (_delete_within_paragraph (start->paragraph, start->index, length))
        {
            if (prev)
            {
                new_para = prev;
                new_index = text_paragraph_get_length (prev);
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
                mark->index <= start->index + length)
            {
                _distribute_mark (mark, new_para, new_index,
                                  new_para, new_index);
            }

            // Marks after affected area
            else if (mark->paragraph == start->paragraph &&
                start->index + length < mark->index)
            {
                _offset_mark (mark, -length);
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
        int to_delete;
        int remaining;

        iter = paragraph;
        cur_deleted = 0;
        dirty = NULL;

        paragraph_length = text_paragraph_get_length (iter);

        // Handle first paragraph
        // Erase part or all of the first paragraph but do not delete it
        to_delete = paragraph_length - start->index;
        _delete_within_paragraph (iter, start->index, to_delete);
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
            _delete_within_paragraph (iter, 0, remaining);
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

                offset = text_paragraph_get_length (start->paragraph);
                _offset_mark (mark, offset - remaining);
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
    int length;

    g_return_val_if_fail (start != NULL, 0);
    g_return_val_if_fail (end != NULL, 0);

    if (start->paragraph == end->paragraph)
        return end->index - start->index;

    iter = start->paragraph;
    length = text_paragraph_get_length (iter) + 1 - start->index;

    while ((iter = walk_until_next_paragraph (TEXT_ITEM (iter))) != NULL)
    {
        if (iter == end->paragraph)
        {
            length += end->index;
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
    if (split->index == text_paragraph_get_length (current))
    {
        // Append a new paragraph with empty run
        new = text_paragraph_new ();
        text_paragraph_append_inline(new, text_run_new(""));

        parent = text_node_get_parent (TEXT_NODE (current));
        text_node_insert_child_after (parent, TEXT_NODE (new), TEXT_NODE (current));
    }

    // Case 2: Split happens mid-paragraph
    else
    {
        TextInline *start;
        TextNode *iter;
        int run_offset;

        new = text_paragraph_new ();
        start = text_paragraph_get_item_at_index (current, split->index, &run_offset);

        iter = TEXT_NODE (start);

        // Split first run if run offset is not at beginning
        if (split->index != run_offset)
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
                split_index_within_run = split->index - run_offset;
                split_run_at_offset (start, &new_run, split_index_within_run);
                text_paragraph_append_inline(new, new_run);
            }

            // Move to next run
            iter = text_node_get_next (iter);
        }

        // Iterate over remaining runs and move them to the new paragraph
        while (iter != NULL)
        {
            TextNode *next;

            g_assert (TEXT_IS_INLINE (iter));

            next = text_node_get_next (iter);

            text_node_unparent (iter);
            text_paragraph_append_inline(new, TEXT_INLINE (iter));

            iter = next;
        }

        // Ensure the original paragraph has at least one run (all runs may be
        // moved when the split index is at the start of the paragraph)
        if (text_node_get_num_children (TEXT_PARAGRAPH (current)) == 0)
        {
            // Add empty run
            text_paragraph_append_inline(current, text_run_new(""));
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
    text_editor_insert_at_mark (self, start, text);
}

void
text_editor_insert_at_mark (TextEditor *self,
                            TextMark   *start,
                            gchar      *str)
{
    // Encapsulates insertion inside an editor module/object.
    // This should accept user input in the form of Operational
    // Transformation commands. This will aid with undo/redo.

    GSList *marks;
    char *text;
    GString *modified;
    TextInline *item;
    TextRun *run;
    int run_start_index;
    int run_offset;
    int length;

    g_return_if_fail (TEXT_IS_EDITOR (self));
    g_return_if_fail (TEXT_IS_DOCUMENT (self->document));
    g_return_if_fail (TEXT_IS_PARAGRAPH (start->paragraph));

    item = text_paragraph_get_item_at_index (start->paragraph, start->index, &run_start_index);

    if (!TEXT_IS_RUN (item))
    {
        g_print ("Not supported: Inserting into non-text run\n");
        return;
    }

    run = TEXT_RUN (item);

    run_offset = start->index - run_start_index;

    // TODO: Replace with hybrid tree/piece-table structure?
    // Textual data is stored in buffers and indexed by the tree
    g_object_get (run, "text", &text, NULL);
    modified = g_string_new (text);
    modified = g_string_insert (modified, run_offset, str);
    g_object_set (run, "text", modified->str, NULL);
    g_string_free (modified, TRUE);

    length = strlen (str);

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
    TextInline *iter;
    TextInline *last;
    int start_run_index;
    int end_run_index;

    _ensure_ordered (&start, &end);

    iter = text_paragraph_get_item_at_index (start->paragraph, start->index, &start_run_index);
    last = text_paragraph_get_item_at_index (end->paragraph, end->index, &end_run_index);

    // Check if start and end indices are in the same run
    if (iter == last)
    {
        TextInline *first_split;
        TextInline *second_split;
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
        TextInline *new_run;
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

        iter = walk_until_next_run (TEXT_ITEM (iter));
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
    TextInline *run;

    run = text_editor_get_item_at_mark (self, mark);
    if (TEXT_IS_RUN (run))
        return text_run_get_style_bold (TEXT_RUN (run));

    return FALSE;
}

gboolean
text_editor_get_format_italic_at_mark (TextEditor *self,
                                       TextMark   *mark)
{
    TextInline *run;

    run = text_editor_get_item_at_mark (self, mark);
    if (TEXT_IS_RUN (run))
        return text_run_get_style_italic (TEXT_RUN (run));

    return FALSE;
}

gboolean
text_editor_get_format_underline_at_mark (TextEditor *self,
                                          TextMark   *mark)
{
    TextInline *run;

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
    text_editor_move_mark_first (self, _get_mark (self, type));
}

void
text_editor_move_last (TextEditor         *self,
                       TextEditorMarkType  type)
{
    text_editor_move_mark_last (self, _get_mark (self, type));
}

void
text_editor_move_right (TextEditor         *self,
                        TextEditorMarkType  type,
                        int                 amount)
{
    text_editor_move_mark_right (self, _get_mark (self, type), amount);
}

void
text_editor_move_left (TextEditor         *self,
                       TextEditorMarkType  type,
                       int                 amount)
{
    text_editor_move_mark_left (self, _get_mark (self, type), amount);
}

void
text_editor_insert (TextEditor         *self,
                    TextEditorMarkType  type,
                    gchar              *str)
{
    text_editor_insert_at_mark (self, _get_mark (self, type), str);
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

TextInline *
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
