/* item.h
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
#include "../tree/node.h"

G_BEGIN_DECLS

#define TEXT_TYPE_ITEM (text_item_get_type())

G_DECLARE_DERIVABLE_TYPE (TextItem, text_item, TEXT, ITEM, TextNode)

struct _TextItemClass
{
    TextNodeClass parent_class;
};

TextItem *text_item_new (void);

void
text_item_attach (TextItem *self,
                  TextNode *attachment);
TextNode *
text_item_get_attachment (TextItem *self);

void
text_item_detach (TextItem *self);

G_END_DECLS
