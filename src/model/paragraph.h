/* paragraph.h
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

#include "item.h"
#include "block.h"
#include "run.h"

G_BEGIN_DECLS

#define TEXT_TYPE_PARAGRAPH (text_paragraph_get_type())

G_DECLARE_FINAL_TYPE (TextParagraph, text_paragraph, TEXT, PARAGRAPH, TextBlock)

TextParagraph * text_paragraph_new                   (void);
void            text_paragraph_append_inline         (TextParagraph *self, TextInline *element);
TextInline     *text_paragraph_get_item_at_index     (TextParagraph *self, int index, int *starting_index);
int             text_paragraph_get_length            (TextParagraph *self);
char           *text_paragraph_get_text              (TextParagraph *self);

G_END_DECLS
