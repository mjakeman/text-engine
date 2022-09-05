/* opaque.h
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

#include "fragment.h"

G_BEGIN_DECLS

#define TEXT_TYPE_OPAQUE (text_opaque_get_type())

G_DECLARE_DERIVABLE_TYPE (TextOpaque, text_opaque, TEXT, OPAQUE, TextFragment)

struct _TextOpaqueClass
{
    TextFragmentClass parent_class;
};

G_END_DECLS
