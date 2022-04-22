/* text-engine.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "text-engine.h"

#include "ui/inspector.h"

#include <gio/gio.h>

void
text_engine_init ()
{
    // Add a GTK Inspector page for debugging documents
    if (g_io_extension_point_lookup ("gtk-inspector-page"))
    g_io_extension_point_implement ("gtk-inspector-page",
                                    TEXT_TYPE_INSPECTOR,
                                    "text-engine",
                                    10);

}
