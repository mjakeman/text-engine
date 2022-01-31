#include <adwaita.h>

#define DEMO_TYPE_WINDOW demo_window_get_type ()
G_DECLARE_FINAL_TYPE (DemoWindow, demo_window, DEMO, WINDOW, GtkApplicationWindow)

struct _DemoWindow
{
    GtkApplicationWindow parent_instance;
};

G_DEFINE_FINAL_TYPE (DemoWindow, demo_window, GTK_TYPE_APPLICATION_WINDOW)

enum {
    PROP_0,
    N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
demo_window_finalize (GObject *object)
{
    DemoWindow *self = (DemoWindow *)object;

    G_OBJECT_CLASS (demo_window_parent_class)->finalize (object);
}

static void
demo_window_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    DemoWindow *self = DEMO_WINDOW (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
demo_window_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
    DemoWindow *self = DEMO_WINDOW (object);

    switch (prop_id)
    {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
demo_window_class_init (DemoWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = demo_window_finalize;
    object_class->get_property = demo_window_get_property;
    object_class->set_property = demo_window_set_property;
}

static void
demo_window_init (DemoWindow *self)
{
}

static void
demo_activate (GApplication *app)
{
    GtkWindow *window;

    g_assert (G_IS_APPLICATION (app));

    // Add CSS Stylesheet
    GtkCssProvider *css_provider = gtk_css_provider_new ();
    gtk_css_provider_load_from_resource (css_provider, "/com/mattjakeman/TextEngine/Demo/style.css");

    GdkDisplay *display = gdk_display_get_default ();
    gtk_style_context_add_provider_for_display (display, GTK_STYLE_PROVIDER (css_provider),
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    // Get the current window or create one if necessary.
    window = gtk_application_get_active_window (GTK_APPLICATION (app));

    if (window == NULL)
        window = g_object_new (DEMO_TYPE_WINDOW,
                               "application", app,
                               NULL);

    // Ask the window manager/compositor to present the window.
    gtk_window_present (window);
}

int
main (int argc, char **argv)
{
    AdwApplication *app;
    int ret;

    app = adw_application_new ("com.mattjakeman.TextEngine.Demo", G_APPLICATION_FLAGS_NONE);

    g_signal_connect (app, "activate", G_CALLBACK (demo_activate), NULL);

    ret = g_application_run (G_APPLICATION (app), argc, argv);

    g_clear_object (&app);

    return ret;
}
