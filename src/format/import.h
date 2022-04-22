/* import.h
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

#include <glib.h>

#include "../model/frame.h"

G_BEGIN_DECLS

TextFrame *format_parse_html (const gchar *html);

G_END_DECLS
