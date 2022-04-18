/* run.c
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

#include "run.h"

struct _TextRun
{
    TextItem parent_instance;
    gchar *text;
};


G_DEFINE_FINAL_TYPE (TextRun, text_run, TEXT_TYPE_ITEM)

enum {
    PROP_0,
    PROP_TEXT,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

TextRun *
text_run_new (const gchar *text)
{
    return g_object_new (TEXT_TYPE_RUN,
                         "text", text,
                         NULL);
}

static void
text_run_finalize (GObject *object)
{
    TextRun *self = (TextRun *)object;

    G_OBJECT_CLASS (text_run_parent_class)->finalize (object);
}

static void
text_run_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    TextRun *self = TEXT_RUN (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
text_run_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
    TextRun *self = TEXT_RUN (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        if (self->text)
            g_free (self->text);
        self->text = g_value_dup_string (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

int
text_run_get_length (TextRun *self)
{
    return strlen (self->text);
}

static void
text_run_class_init (TextRunClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = text_run_finalize;
    object_class->get_property = text_run_get_property;
    object_class->set_property = text_run_set_property;

    properties [PROP_TEXT]
        = g_param_spec_string ("text",
                               "Text",
                               "Text",
                               NULL,
                               G_PARAM_READWRITE|G_PARAM_CONSTRUCT);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
text_run_init (TextRun *self)
{
}
