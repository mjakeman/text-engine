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
    GdkDisplay *display;
    GtkCssProvider *provider;

    // Add a GTK Inspector page for debugging documents
    if (g_io_extension_point_lookup ("gtk-inspector-page"))
        g_io_extension_point_implement ("gtk-inspector-page",
                                        TEXT_TYPE_INSPECTOR,
                                        "text-engine",
                                        10);

    // Add CSS Provider for internal stylesheet
    display = gdk_display_get_default ();
    provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (provider, "/com/mattjakeman/TextEngine/style.css");
    gtk_style_context_add_provider_for_display (display, GTK_STYLE_PROVIDER (provider),
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}
