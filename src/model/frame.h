/* frame.h
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
