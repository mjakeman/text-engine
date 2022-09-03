/* editor.h
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#pragma once

#include <glib-object.h>

#include "../model/document.h"

G_BEGIN_DECLS

#define TEXT_TYPE_EDITOR (text_editor_get_type())

G_DECLARE_FINAL_TYPE (TextEditor, text_editor, TEXT, EDITOR, GObject)

typedef enum
{
    TEXT_EDITOR_CURSOR,
    TEXT_EDITOR_SELECTION
} TextEditorMarkType;

TextEditor *text_editor_new        (TextDocument *document);

void        text_editor_move_mark_first (TextEditor *self, TextMark *mark);
void        text_editor_move_mark_last  (TextEditor *self, TextMark *mark);
void        text_editor_move_mark_right (TextEditor *self, TextMark *mark, int amount);
void        text_editor_move_mark_left  (TextEditor *self, TextMark *mark, int amount);
void        text_editor_insert_at_mark  (TextEditor *self, TextMark *start, gchar *str);
void        text_editor_delete_at_mark  (TextEditor *self, TextMark *start, int length);
void        text_editor_replace_at_mark (TextEditor *self, TextMark *start, TextMark *end, gchar *text);
void        text_editor_split_at_mark   (TextEditor *self, TextMark *mark);

void        text_editor_move_first      (TextEditor *self, TextEditorMarkType type);
void        text_editor_move_last       (TextEditor *self, TextEditorMarkType type);
void        text_editor_move_right      (TextEditor *self, TextEditorMarkType type, int amount);
void        text_editor_move_left       (TextEditor *self, TextEditorMarkType type, int amount);
void        text_editor_insert          (TextEditor *self, TextEditorMarkType type, gchar *str);
void        text_editor_delete          (TextEditor *self, TextEditorMarkType type, int length);
void        text_editor_replace         (TextEditor *self, TextEditorMarkType start_type, TextEditorMarkType end_type, gchar *text);
void        text_editor_split           (TextEditor *self, TextEditorMarkType type);

TextRun    *text_editor_get_run         (TextEditor *self, TextEditorMarkType type);
TextRun    *text_editor_get_run_at_mark (TextEditor *self, TextMark *mark);

gchar      *text_editor_dump_plain_text (TextEditor *self);

// Traversal Helpers
// TODO: These shouldn't really be part of the editor
void            text_editor_sort_marks          (TextMark *mark1, TextMark *mark2, TextMark **first, TextMark **last);
TextParagraph  *text_editor_next_paragraph      (TextParagraph *paragraph);
TextParagraph  *text_editor_previous_paragraph  (TextParagraph *paragraph);

// Format Helpers
// TODO: Make this more abstract
void            text_editor_apply_format_bold   (TextEditor *self, TextMark *start, TextMark *end, gboolean is_bold);

G_END_DECLS
