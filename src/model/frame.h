/* frame.h
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

G_BEGIN_DECLS

#define TEXT_TYPE_FRAME (text_frame_get_type())

G_DECLARE_DERIVABLE_TYPE (TextFrame, text_frame, TEXT, FRAME, TextBlock)

struct _TextFrameClass
{
    TextBlockClass parent_class;
};

TextFrame *text_frame_new           (void);
void       text_frame_append_block  (TextFrame *self, TextBlock *block);
void       text_frame_prepend_block (TextFrame *self, TextBlock *block);

G_END_DECLS
