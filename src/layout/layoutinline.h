/* layoutinline.h
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
#include <pango/pango.h>

#include "layoutbox.h"
#include "layoutbox-impl.h"

#include "types.h"

G_BEGIN_DECLS

#define TEXT_TYPE_LAYOUT_INLINE (text_layout_inline_get_type())

G_DECLARE_DERIVABLE_TYPE (TextLayoutInline, text_layout_inline, TEXT, LAYOUT_INLINE, TextLayoutBox)

struct _TextLayoutInlineClass
{
    TextLayoutBoxClass parent_class;
};

TextLayoutInline *text_layout_inline_new (void);

G_END_DECLS
