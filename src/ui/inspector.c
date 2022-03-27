/* inspector.c
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

#include "inspector.h"

struct _TextInspector
{
    GtkWidget parent_instance;

    GObject *object;
    GtkWidget *label;
};

G_DEFINE_FINAL_TYPE (TextInspector, text_inspector, GTK_TYPE_WIDGET)

enum {
    PROP_0,
    PROP_TITLE,
    PROP_OBJECT,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

#define TITLE "Text Engine"

TextInspector *
text_inspector_new (void)
{
    return g_object_new (TEXT_TYPE_INSPECTOR, NULL);
}

static void
text_inspector_finalize (GObject *object)
{
    TextInspector *self = (TextInspector *)object;

    gtk_widget_unparent (self->label);

    G_OBJECT_CLASS (text_inspector_parent_class)->finalize (object);
}

static void
text_inspector_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    TextInspector *self = TEXT_INSPECTOR (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_value_set_string (value, TITLE);
        break;
    case PROP_OBJECT:
        g_value_set_object (value, self->object);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_inspector_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    TextInspector *self = TEXT_INSPECTOR (object);

    switch (prop_id)
    {
    case PROP_OBJECT:
        self->object = g_value_get_object (value);
        gtk_label_set_text (GTK_LABEL (self->label), g_type_name_from_instance ((GTypeInstance *)self->object));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_inspector_class_init (TextInspectorClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_inspector_finalize;
    object_class->get_property = text_inspector_get_property;
    object_class->set_property = text_inspector_set_property;

    properties [PROP_TITLE]
        = g_param_spec_string ("title",
                               "Title",
                               "Title",
                               TITLE,
                               G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    properties [PROP_OBJECT]
        = g_param_spec_object ("object",
                               "Object",
                               "Object",
                               G_TYPE_OBJECT,
                               G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);


    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
text_inspector_init (TextInspector *self)
{
    self->label = gtk_label_new ("");
    gtk_widget_set_parent (self->label, GTK_WIDGET (self));
    gtk_widget_set_layout_manager (GTK_WIDGET (self), gtk_bin_layout_new ());
}
