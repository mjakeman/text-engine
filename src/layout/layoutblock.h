/* layoutblock.h
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

#define TEXT_TYPE_LAYOUT_BLOCK (text_layout_block_get_type())

G_DECLARE_DERIVABLE_TYPE (TextLayoutBlock, text_layout_block, TEXT, LAYOUT_BLOCK, TextLayoutBox)

struct _TextLayoutBlockClass
{
    TextLayoutBoxClass parent_class;
};

TextLayoutBlock *text_layout_block_new (void);

PangoLayout *
text_layout_block_get_pango_layout (TextLayoutBlock *self);

G_END_DECLS
