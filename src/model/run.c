/* run.c
 *
 * Copyright 2022 Matthew Jakeman <mjakeman26@outlook.co.nz>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
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
