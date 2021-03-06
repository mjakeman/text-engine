/* block.h
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

G_BEGIN_DECLS

#define TEXT_TYPE_BLOCK (text_block_get_type())

G_DECLARE_DERIVABLE_TYPE (TextBlock, text_block, TEXT, BLOCK, TextItem)

struct _TextBlockClass
{
    TextNodeClass parent_class;
};

G_END_DECLS
