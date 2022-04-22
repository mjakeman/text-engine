/* document.h
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

#include "frame.h"
#include "mark.h"

struct _TextDocument
{
    GObject parent_instance;

    TextFrame *frame;
    TextMark *cursor;
    TextMark *selection;
    GSList *marks;
};

G_BEGIN_DECLS

#define TEXT_TYPE_DOCUMENT (text_document_get_type())

G_DECLARE_FINAL_TYPE (TextDocument, text_document, TEXT, DOCUMENT, GObject)

TextDocument *text_document_new         (void);

TextMark     *text_document_create_mark     (TextDocument *doc, TextParagraph *paragraph, int index, TextGravity gravity);
TextMark     *text_document_copy_mark       (TextDocument *doc, TextMark *mark);
void          text_document_delete_mark     (TextDocument *doc, TextMark *mark);
void          text_document_clear_mark      (TextDocument *doc, TextMark **mark);

// TODO: Make private
GSList       *text_document_get_all_marks   (TextDocument *doc);

G_END_DECLS
