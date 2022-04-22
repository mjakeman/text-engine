/* document.h
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
