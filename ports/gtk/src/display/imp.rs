use gtk::glib;
use gtk::gdk;
use gtk::graphene::Point;
use gtk::pango;
use gtk::pango::AttrList;
use gtk::pango::GlyphString;
use gtk::subclass::prelude::*;
use gtk::traits::{SnapshotExt, StyleContextExt};
use gtk::traits::WidgetExt;
use text_engine::DefaultLayoutManager;
use text_engine::RenderCommand::*;

use std::cell::RefCell;
use adw::gdk::pango::ffi;
use adw::glib::translate::{ToGlibPtr, ToGlibPtrMut};
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

        let widget = self.obj();
        let color = widget.style_context().color();

        if let Some(doc) = self.document.borrow().as_ref() {
            let display_list = doc.layout(widget.width(), &DefaultLayoutManager {});
            for command in &display_list.commands {
                match command {
                    RenderBox(rect) => {
                        
                    },
                    RenderText(x, y, text) => {
                        // TODO: We recreate the layout here and inside the layout engine
                        // Store this as a resource in the render layer and cache it between
                        // redraws. We only want to recompute the delta.
                        let layout = pango::Layout::new(&widget.pango_context());
                        let font_desc = FontDescription::from_string("Noto Sans");
                        layout.set_font_description(Some(&font_desc));
                        layout.set_text(text);

                        snapshot.append_layout(&layout, &color);
                    }
                }
            }
        }
    }
}