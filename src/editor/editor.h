/* editor.h
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

void        text_editor_move_first      (TextEditor *self, TextEditorMarkType type);
void        text_editor_move_last       (TextEditor *self, TextEditorMarkType type);
void        text_editor_move_right      (TextEditor *self, TextEditorMarkType type, int amount);
void        text_editor_move_left       (TextEditor *self, TextEditorMarkType type, int amount);
void        text_editor_insert          (TextEditor *self, TextEditorMarkType type, gchar *str);
void        text_editor_delete          (TextEditor *self, TextEditorMarkType type, int length);
void        text_editor_replace         (TextEditor *self, TextEditorMarkType start_type, TextEditorMarkType end_type, gchar *text);

TextRun *   text_editor_get_run         (TextEditor *self, TextEditorMarkType type);
TextRun *   text_editor_get_run_at_mark (TextEditor *self, TextMark *mark);

G_END_DECLS
