#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define RICH_TEXT_TYPE_WIDGET (rich_text_widget_get_type())

G_DECLARE_FINAL_TYPE (RichTextWidget, rich_text_widget, RICH_TEXT, WIDGET, GtkWidget)

RichTextWidget *rich_text_widget_new (void);

G_END_DECLS
