/* fragment.h
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

#define TEXT_TYPE_FRAGMENT (text_fragment_get_type())

G_DECLARE_DERIVABLE_TYPE (TextFragment, text_fragment, TEXT, FRAGMENT, TextItem)

struct _TextFragmentClass
{
    TextNodeClass parent_class;
    int (*get_length)(TextFragment *self);
    const char *(*get_text)(TextFragment *self);
};

int         text_fragment_get_length    (TextFragment *self);
const char* text_fragment_get_text      (TextFragment *self);

G_END_DECLS
