use gtk::glib;
use gtk::gdk;
use gtk::pango;
use gtk::subclass::prelude::*;
use gtk::traits::SnapshotExt;
use gtk::traits::WidgetExt;

use std::cell::RefCell;
use gtk::pango::FontDescription;

use text_engine::Document;

// Object holding the state
#[derive(Default)]
pub struct Display {
    pub document: RefCell<Option<Document>>
}

// The central trait for subclassing a GObject
#[glib::object_subclass]
impl ObjectSubclass for Display {
    const NAME: &'static str = "TextEngineDisplay";
    type Type = super::Display;
    type ParentType = gtk::Widget;
}

// Trait shared by all GObjects
impl ObjectImpl for Display {}

// Trait shared by all widgets
impl WidgetImpl for Display {
    fn snapshot(&self, snapshot: &gtk::Snapshot) {
        let color = gdk::RGBA::new(0.0, 0.0, 0.0, 1.0);
        let widget = self.obj();
        let layout = pango::Layout::new(&widget.pango_context());
        let font_desc = FontDescription::from_string("Noto Sans");
        layout.set_font_description(Some(&font_desc));
        
        if let Some(doc) = self.document.borrow().as_ref() {
            layout.set_text(&doc.get_all_text());
            println!("{}", doc.get_all_text());
        }
        
        snapshot.append_layout(&layout, &color);
    }
}