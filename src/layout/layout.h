#pragma once

#include <glib-object.h>
#include <pango/pango.h>
#include <graphene.h>

#include "layout-box.h"

#include "../model/frame.h"
#include "../model/paragraph.h"

G_BEGIN_DECLS

#define TEXT_TYPE_LAYOUT (text_layout_get_type())

G_DECLARE_FINAL_TYPE (TextLayout, text_layout, TEXT, LAYOUT, GObject)

TextLayout *text_layout_new (void);

TextLayoutBox *
text_layout_build_layout_tree (TextLayout   *self,
                               PangoContext *context,
                               TextFrame    *frame,
                               int           width);

G_END_DECLS
