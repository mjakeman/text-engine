/* run.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * This file is dual-licensed under the terms of the Mozilla Public
 * License 2.0 and the Lesser General Public License 2.1 (or any
 * later version).
 *
 * SPDX-License-Identifier: MPL-2.0 OR LGPL-2.1-or-later
 */

#include "run.h"

struct _TextRun
{
    TextItem parent_instance;
    gchar *text;
    gboolean is_bold;
    gboolean is_italic;
    gboolean is_underline;
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
    g_return_val_if_fail (TEXT_IS_RUN (self), -1);

    return strlen (self->text);
}

gboolean
text_run_get_style_bold (TextRun *self)
{
    return self->is_bold;
}

void
text_run_set_style_bold (TextRun  *self,
                         gboolean  is_bold)
{
    self->is_bold = is_bold;
}

gboolean
text_run_get_style_italic (TextRun *self)
{
    return self->is_italic;
}

void
text_run_set_style_italic (TextRun  *self,
                           gboolean  is_italic)
{
    self->is_italic = is_italic;
}

gboolean
text_run_get_style_underline (TextRun *self)
{
    return self->is_underline;
}

void
text_run_set_style_underline (TextRun  *self,
                              gboolean  is_underline)
{
    self->is_underline = is_underline;
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
