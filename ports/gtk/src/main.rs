mod display;

use text_engine;

use gtk::prelude::*;
use gtk::ApplicationWindow;
use adw::Application;
use crate::display::Display;

const APP_ID: &str = "com.mattjakeman.TextEngine";

fn main() {
    // Initialise text engine
    text_engine::init();

    // Create a new application
    let app = Application::builder().application_id(APP_ID).build();

    // Connect to "activate" signal of `app`
    app.connect_activate(build_ui);

    // Run the application
    app.run();
}

fn build_ui(app: &Application) {
    // Create a button with label and margins
    let display = Display::new();
    display.set_margin_bottom(12);
    display.set_margin_top(12);
    display.set_margin_start(12);
    display.set_margin_end(12);
    display.set_document(text_engine::get_sample_document());

    // Create a window
    let window = ApplicationWindow::builder()
        .application(app)
        .title("Text Engine Demo")
        .default_width(400)
        .default_height(400)
        .child(&display)
        .build();

    // Present window
    window.present();
}