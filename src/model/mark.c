/* mark.c
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

#include "mark.h"

G_DEFINE_BOXED_TYPE (TextMark, text_mark, text_mark_copy, text_mark_free)

/**
 * text_mark_new:
 *
 * Creates a new #TextMark.
 *
 * Returns: (transfer full): A newly created #TextMark
 */
TextMark *
text_mark_new (TextParagraph *paragraph,
               int            index)
{
    TextMark *self;

    self = g_slice_new0 (TextMark);
    self->paragraph = paragraph;
    self->index = index;

    return self;
}

/**
 * text_mark_copy:
 * @self: a #TextMark
 *
 * Makes a deep copy of a #TextMark.
 *
 * Returns: (transfer full): A newly created #TextMark with the same
 *   contents as @self
 */
TextMark *
text_mark_copy (TextMark *self)
{
    TextMark *copy;

    g_return_val_if_fail (self, NULL);

    copy = text_mark_new (self->paragraph, self->index);

    return copy;
}

/**
 * text_mark_free:
 * @self: a #TextMark
 *
 * Frees a #TextMark allocated using text_mark_new()
 * or text_mark_copy().
 */
void
text_mark_free (TextMark *self)
{
    g_return_if_fail (self);

    // g_clear_object (&self->parent);

    g_slice_free (TextMark, self);
}
