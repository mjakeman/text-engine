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
        let context = &widget.pango_context();
        let font = context.load_font(&context.font_description().unwrap()).unwrap();

        if let Some(doc) = self.document.borrow().as_ref() {
            let display_list = doc.layout(widget.width(), &DefaultLayoutManager {});
            for command in &display_list.commands {
                match command {
                    RenderBox(rect) => {
                        
                    },
                    RenderText(x, y, text) => {
                        let attrs = AttrList::new();
                        let items = pango::itemize(context, text, 0, text.as_bytes().len() as i32, &attrs, None);

                        let mut offset = 0f32;

                        for item in items {
                            let mut glyphs = GlyphString::new();
                            let start = item.offset() as usize;
                            let finish = (item.offset() + item.length()) as usize;
                            let substr = std::str::from_utf8(text.as_bytes().get(start..finish).unwrap()).unwrap();

                            pango::shape(substr, item.analysis(), &mut glyphs);

                            let text_node = gtk::gsk::TextNode::new(&item.analysis().font(), &glyphs, &color, &Point::new((*x as f32) + offset, *y as f32));
                            offset += glyphs.extents(&font).0.width() as f32 / pango::SCALE as f32;

                            if let Some(node) = text_node {
                                snapshot.append_node(node);
                            }
                        }
                    }
                }
            }
            
            /*layout.set_text(&doc.get_all_text());
            println!("{}", doc.get_all_text());*/
        }

        
        /*let layout = pango::Layout::new(&widget.pango_context());
        let font_desc = FontDescription::from_string("Noto Sans");
        layout.set_font_description(Some(&font_desc));
        
        if let Some(doc) = self.document.borrow().as_ref() {
            layout.set_text(&doc.get_all_text());
            println!("{}", doc.get_all_text());
        }
        
        snapshot.append_layout(&layout, &color);*/
    }
}