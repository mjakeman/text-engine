/* mark.h
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
#include "paragraph.h"

G_BEGIN_DECLS

#define TEXT_TYPE_MARK (text_mark_get_type ())

typedef enum
{
    TEXT_GRAVITY_LEFT,
    TEXT_GRAVITY_RIGHT,
} TextGravity;

typedef struct _TextMark TextMark;

struct _TextMark
{
    TextParagraph *paragraph;
    int index;
    TextGravity gravity;
};

GType         text_mark_get_type (void) G_GNUC_CONST;
TextMark     *text_mark_new      (TextParagraph *paragraph, int index, TextGravity gravity);
TextMark     *text_mark_copy     (TextMark *self);
void          text_mark_free     (TextMark *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (TextMark, text_mark_free)

G_END_DECLS
