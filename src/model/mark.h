/* mark.h
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
#include "paragraph.h"

G_BEGIN_DECLS

#define TEXT_TYPE_MARK (text_mark_get_type ())

typedef enum
{
    TEXT_GRAVITY_LEFT,
    TEXT_GRAVITY_RIGHT,
} TextGravity;

typedef struct _TextMark TextMark;
struct _TextDocument;

struct _TextMark
{
    struct _TextDocument *document;

    TextParagraph *paragraph;
    int index; // byte index (i.e. NOT unicode)
    TextGravity gravity;
};

GType         text_mark_get_type (void) G_GNUC_CONST;
TextMark     *text_mark_new      (struct _TextDocument *document, TextParagraph *paragraph, int index, TextGravity gravity);
TextMark     *text_mark_copy     (TextMark *self);
void          text_mark_free     (TextMark *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (TextMark, text_mark_free)

G_END_DECLS
