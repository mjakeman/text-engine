/* mark.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
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
               int            index,
               TextGravity    gravity)
{
    TextMark *self;

    self = g_slice_new0 (TextMark);
    self->paragraph = paragraph;
    self->index = index;
    self->gravity = gravity;

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

    copy = text_mark_new (self->paragraph, self->index, self->gravity);

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
